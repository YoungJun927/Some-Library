#include "LCD1602.hpp"
#include "cox.h"

LCD1602::LCD1602(TwoWire &wire) : wire(wire){}

void LCD1602::begin(uint8_t lcd_Addr, uint8_t dotsize){
  wire.begin();

  _Addr = lcd_Addr;
  _backlightval = LCD_NOBACKLIGHT;

  //Wait for mort than 40ms after Vcc rises to 2.7V
  delay(50);

  //ready for 4 bit Interface
  write4bits(0x03 << 4);
  delayMicroseconds(4500);
  write4bits(0x03 << 4);
  delayMicroseconds(150);
  write4bits(0x03 << 4);

  //function set
  write4bits(0x02 << 4);
  command(0x28);            // Function set
  command(0x0C);            // Display on/off control
  command(0x04);            // Entry mode set
  backlight();              // backlight on
  command(0x01);            // Clear display
  delayMicroseconds(2000);  //this command takes a long time!
}

inline void LCD1602::command(uint8_t value){
  // send Instruction
  send(value, 0);
}

void LCD1602::send(uint8_t value, uint8_t mode) {
  //for 4bit interface, mode(Rs,Rw,En)
  uint8_t high4bits=value&0xf0;
  uint8_t low4bits=(value<<4)&0xf0;

  write4bits((high4bits|mode));
  write4bits((low4bits|mode));
}

void LCD1602::write4bits(uint8_t data){
  //4-bit data Transfer Timing Sequence
  expanderWrite(data);
  pulseEnable(data);
}

void LCD1602::expanderWrite(uint8_t data){
  wire.beginTransmission(_Addr);
  wire.write((data) | _backlightval);
  wire.endTransmission();
}

void LCD1602::pulseEnable(uint8_t _data){
  expanderWrite((_data |En));  	 // En high
  delayMicroseconds(1);		       // enable pulse must be >450ns
  expanderWrite((_data & ~En));	 // En low
  delayMicroseconds(50);		     // commands need > 37us to settle
}

void LCD1602::setline(){
  // cursor is positioned at the head of the second line
  command(0xC0);
}

void LCD1602::print(const char c[]){
  //print text
  uint8_t a=strlen(c);

  for(int i=0;i<a;i++){
    send(c[i],Rs);
  }
}
void LCD1602::backlight(void){
  //backlight on
  _backlightval=LCD_BACKLIGHT;

  expanderWrite(0);
}

void LCD1602::clear(void){
  //data clear & cursor goes to the home
  command(0x01);
  delayMicroseconds(2000);
}

void LCD1602::home(void){
  //data remains & cursor goes to the home
  command(0x02);
  delayMicroseconds(2000);
}
