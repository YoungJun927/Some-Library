#include <cox.h>
#include "Dogm128_6.hpp"
#include "Adafruit_GFX.hpp"
#include "SPI.hpp"

#define swap(a, b) { int16_t t = a; a = b; b = t; }

Dogm128_6::Dogm128_6( SPI &spi,
                      int8_t lcdPwr,
                      int8_t lcdReset,
                      int8_t lcdPinCs,
                      int8_t lcdMode )  : Adafruit_GFX(DOGM128_6_LCDWIDTH,DOGM128_6_LCDHEIGHT),
                                          spi(&spi),
                                          lcdPwr(lcdPwr),
                                          lcdReset(lcdReset),
                                          lcdPinCs(lcdPinCs),
                                          lcdMode(lcdMode){
                                            rotation  = 0;
}

void Dogm128_6::begin(){
  const char lcdInitCmd[] ={
      0x40,   /*Display start line 0                    */
      0xa1,   /*ADC reverse, 6 oclock viewing direction */
      0xc0,   /*Normal COM0...COM63                     */
      0xa6,   /*Display normal, not mirrored            */
      0xa2,   /*Set Bias 1/9 (Duty 1/65)                */
      0x2f,   /*Booster, Regulator and Follower On      */
      0xf8,   /*Set internal Booster to 4x              */
      0x00,   /*                                        */
      0x27,   /*Contrast set                            */
      0x81,   /*                                        */
      0x16,   /* <- use value from LCD-MODULE .doc guide*/
      /*    for better contrast (not 0x10)      */
      0xac,   /*No indicator                            */
      0x00,   /*                                        */
      0xaf,   /*Display on                              */
      0xb0,   /*Page 0 einstellen                       */
      0x10,   /*High-Nibble of column address           */
      0x00    /*Low-Nibble of column address            */
  };
  //
  // LCD IO pins Output as GPIO output high
  //
  pinMode(lcdPinCs,OUTPUT);
  pinMode(lcdPwr,OUTPUT);
  pinMode(lcdReset,OUTPUT);
  pinMode(lcdMode,OUTPUT);
  //
  // Set BSP_lcdPwr=1,RSTn=0, A0=1, CSn=1 with high drive strength
  //
  digitalWrite(lcdPwr,HIGH);
  digitalWrite(lcdReset,LOW);
  digitalWrite(lcdMode,HIGH);
  digitalWrite(lcdPinCs,HIGH);
  //
  // Wait ~ 100 ms (@ 16 MHz) and clear reset
  //
  // __delay_cycles(1600000);
  delay(160);
  digitalWrite(lcdReset,HIGH);
  //
  // Send init command sequence
  //
  lcdSendCommand((char*)lcdInitCmd, (uint8_t)sizeof(lcdInitCmd));
}

void Dogm128_6::lcdSendCommand(char *command, uint8_t length){
  //
  // Assert CSn, indicate command (A0 low), send bytes, deassert CSn
  //
  digitalWrite(lcdPinCs,LOW);
  digitalWrite(lcdMode,LOW);

  this->spi->begin(1000000ul, SPI::MSBFIRST, SPI::MODE2);

  while(length--){
    this->spi->transfer((uint8_t)*command);
    command++;
  }
  digitalWrite(lcdPinCs,HIGH);
  this->spi->end();
}

void Dogm128_6::lcdClear(){
  memset(buffer, 0, (DOGM128_6_LCDWIDTH*DOGM128_6_LCDHEIGHT/8));
}

void Dogm128_6::lcdCursor(uint8_t column, uint8_t page){
  uint8_t cmd[] = {0xB0, 0x10, 0x00};
  //
  // Adding Y position, and X position (hi/lo nibble) to command array
  //
  cmd[0] = cmd[0] + page;
  cmd[2] = cmd[2] + (column & 0x0F);
  cmd[1] = cmd[1] + (column >> 4);

  lcdSendCommand((char *)cmd, 3);
}

void Dogm128_6::lcdSendBuffer(char *buf){
  memcpy(this->buffer, buf, sizeof(this->buffer));
  //
  // For each page
  //
  for(uint8_t Page = 0; Page < 8; Page++)
  {
      //
      // Set LCD pointer to start of the correct page and send data
      //
      lcdCursor(0, Page);
      lcdSendData((char *)(this->buffer + (Page * DOGM128_6_LCDWIDTH)),
                  (uint16_t) DOGM128_6_LCDWIDTH);
  }
}

void Dogm128_6::lcdSendData(char *data, uint16_t length){
    //
    // Assert CSn, indicate data (A0 high), send bytes, deassert CSn)
    //
		digitalWrite(lcdPinCs,LOW);
		digitalWrite(lcdMode,HIGH);

		this->spi->begin(1000000ul, SPI::MSBFIRST, SPI::MODE2);

		while(length--){
      this->spi->transfer((uint8_t)*data);
      data++;
    }
    this->spi->end();
		digitalWrite(lcdPinCs,HIGH);
}

void Dogm128_6::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= DOGM128_6_LCDWIDTH) || (y < 0) || (y >= DOGM128_6_LCDHEIGHT))
    return;

  // check rotation, move pixel around if necessary
  switch (getRotation()) {
  case 1:
    swap(x, y);
    x = WIDTH - x - 1;
    break;
  case 2:
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = HEIGHT - y - 1;
    break;
  }
  // x is which column
    switch (color)
    {
      case WHITE:   buffer[x+ (y/8)*DOGM128_6_LCDWIDTH] |=  (1 << (y&7)); break;
      case BLACK:   buffer[x+ (y/8)*DOGM128_6_LCDWIDTH] &= ~(1 << (y&7)); break;
      case INVERSE: buffer[x+ (y/8)*DOGM128_6_LCDWIDTH] ^=  (1 << (y&7)); break;
    }
}

uint8_t Dogm128_6::getRotation(void) const {
    return rotation;
}

void Dogm128_6::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  bool bSwap = false;
  switch(rotation) {
    case 0:
      break;
    case 1:
      // 90 degree rotation, swap x & y for rotation, then invert x and adjust x for h (now to become w)
      bSwap = true;
      swap(x, y);
      x = WIDTH - x - 1;
      x -= (h-1);
      break;
    case 2:
      // 180 degree rotation, invert x and y - then shift y around for height.
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      y -= (h-1);
      break;
    case 3:
      // 270 degree rotation, swap x & y for rotation, then invert y
      bSwap = true;
      swap(x, y);
      y = HEIGHT - y - 1;
      break;
  }

  if(bSwap) {
    drawFastHLineInternal(x, y, h, color);
  } else {
    drawFastVLineInternal(x, y, h, color);
  }
}

void Dogm128_6::drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) {

  // do nothing if we're off the left or right side of the screen
  if(x < 0 || x >= WIDTH) { return; }

  // make sure we don't try to draw below 0
  if(__y < 0) {
    // __y is negative, this will subtract enough from __h to account for __y being 0
    __h += __y;
    __y = 0;

  }

  // make sure we don't go past the height of the display
  if( (__y + __h) > HEIGHT) {
    __h = (HEIGHT - __y);
  }

  // if our height is now negative, punt
  if(__h <= 0) {
    return;
  }

  // this display doesn't need ints for coordinates, use local byte registers for faster juggling
  register uint8_t y = __y;
  register uint8_t h = __h;


  // set up the pointer for fast movement through the buffer
  register char *pBuf = buffer;
  // adjust the buffer pointer for the current row
  pBuf += ((y/8) * DOGM128_6_LCDWIDTH);
  // and offset x columns in
  pBuf += x;

  // do the first partial byte, if necessary - this requires some masking
  register uint8_t mod = (y&7);
  if(mod) {
    // mask off the high n bits we want to set
    mod = 8-mod;

    // note - lookup table results in a nearly 10% performance improvement in fill* functions
    // register uint8_t mask = ~(0xFF >> (mod));
    static uint8_t premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
    register uint8_t mask = premask[mod];

    // adjust the mask if we're not going to reach the end of this byte
    if( h < mod) {
      mask &= (0XFF >> (mod-h));
    }

  switch (color)
    {
    case WHITE:   *pBuf |=  mask;  break;
    case BLACK:   *pBuf &= ~mask;  break;
    case INVERSE: *pBuf ^=  mask;  break;
    }

    // fast exit if we're done here!
    if(h<mod) { return; }

    h -= mod;

    pBuf += DOGM128_6_LCDWIDTH;
  }


  // write solid bytes while we can - effectively doing 8 rows at a time
  if(h >= 8) {
    if (color == INVERSE)  {          // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
      do  {
      *pBuf=~(*pBuf);

        // adjust the buffer forward 8 rows worth of data
        pBuf += DOGM128_6_LCDWIDTH;

        // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
        h -= 8;
      } while(h >= 8);
      }
    else {
      // store a local value to work with
      register uint8_t val = (color == WHITE) ? 255 : 0;

      do  {
        // write our value in
      *pBuf = val;

        // adjust the buffer forward 8 rows worth of data
        pBuf += DOGM128_6_LCDWIDTH;

        // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
        h -= 8;
      } while(h >= 8);
      }
    }

  // now do the final partial byte, if necessary
  if(h) {
    mod = h & 7;
    // this time we want to mask the low bits of the byte, vs the high bits we did above
    // register uint8_t mask = (1 << mod) - 1;
    // note - lookup table results in a nearly 10% performance improvement in fill* functions
    static uint8_t postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
    register uint8_t mask = postmask[mod];
    switch (color)
    {
      case WHITE:   *pBuf |=  mask;  break;
      case BLACK:   *pBuf &= ~mask;  break;
      case INVERSE: *pBuf ^=  mask;  break;
    }
  }
}

void Dogm128_6::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  boolean bSwap = false;
  switch(rotation) {
    case 0:
      // 0 degree rotation, do nothing
      break;
    case 1:
      // 90 degree rotation, swap x & y for rotation, then invert x
      bSwap = true;
      swap(x, y);
      x = WIDTH - x - 1;
      break;
    case 2:
      // 180 degree rotation, invert x and y - then shift y around for height.
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      x -= (w-1);
      break;
    case 3:
      // 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to become h)
      bSwap = true;
      swap(x, y);
      y = HEIGHT - y - 1;
      y -= (w-1);
      break;
  }

  if(bSwap) {
    drawFastVLineInternal(x, y, w, color);
  } else {
    drawFastHLineInternal(x, y, w, color);
  }
}

void Dogm128_6::drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) {
  // Do bounds/limit checks
  if(y < 0 || y >= HEIGHT) { return; }

  // make sure we don't try to draw below 0
  if(x < 0) {
    w += x;
    x = 0;
  }

  // make sure we don't go off the edge of the display
  if( (x + w) > WIDTH) {
    w = (WIDTH - x);
  }

  // if our width is now negative, punt
  if(w <= 0) { return; }

  // set up the pointer for  movement through the buffer
  register char *pBuf = buffer;
  // adjust the buffer pointer for the current row
  pBuf += ((y/8) * DOGM128_6_LCDWIDTH);
  // and offset x columns in
  pBuf += x;

  register uint8_t mask = 1 << (y&7);

  switch (color)
  {
  case WHITE: while(w--) { *pBuf++ |= mask; }; break;
  case BLACK: mask = ~mask;   while(w--) { *pBuf++ &= mask; }; break;
  case INVERSE: while(w--) { *pBuf++ ^= mask; }; break;
  }
}
