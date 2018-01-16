#include <cox.h>
#include <Dogm128_6.hpp>
#include <Adafruit_GFX.hpp>

Dogm128_6 display = Dogm128_6(IoSpi0,
                              LCD_PWR,
                              LCD_RESET_N,
                              LCD_CS_N,
                              LCD_MODE);

Timer timerHello;

void testdrawrect(void) {
  for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.lcdSendBuffer(display.buffer);
    delay(1);
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.lcdSendBuffer(display.buffer);
    delay(1);
    color++;
  }
}

void testdrawcircle(void) {
  for (int16_t i=0; i<display.height(); i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.lcdSendBuffer(display.buffer);
    delay(1);
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.lcdSendBuffer(display.buffer);
    delay(1);
  }
}

static void taskHello(void*) {
  display.drawChar(0,0,'A',WHITE,1,1);
  display.lcdSendBuffer(display.buffer);
  delay(2000);

  display.lcdClear();
  display.drawPixel(10, 10, WHITE);
  display.lcdSendBuffer(display.buffer);
  delay(2000);

  display.lcdClear();
  testdrawrect();
  display.lcdSendBuffer(display.buffer);
  delay(2000);

  display.lcdClear();
  testfillrect();
  display.lcdSendBuffer(display.buffer);
  delay(2000);

  display.lcdClear();
  testfillroundrect();
  display.lcdSendBuffer(display.buffer);
  delay(2000);
  display.lcdClear();
}

void setup() {
  Serial.begin(9600);
  printf("******** Test TRxeB ********\n" );

  display.begin();
  display.lcdClear();
  display.lcdSendBuffer(display.buffer);

  timerHello.onFired(taskHello,NULL);
  timerHello.startPeriodic(2000);
}
