#include "TM1637Display.hpp"
#include "cox.h"

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
TM1637Display::TM1637Display(uint8_t pinClk, uint8_t pinDIO){
  m_pinClk = pinClk;
  m_pinDIO = pinDIO;

  pinMode(m_pinClk, INPUT);
  pinMode(m_pinDIO,INPUT);

  digitalWrite(m_pinClk, LOW);
  digitalWrite(m_pinDIO, LOW);
}

void TM1637Display::setBrightness(uint8_t brightness, bool on){
  m_brightness = (brightness & 0x7) | (on? 0x08 : 0x00);
}

void TM1637Display::setSegments(const uint8_t segments[], uint8_t length, uint8_t pos){
  // Write COMM1
  start();
  writeByte(TM1637_I2C_COMM1);
  stop();

  // Write COMM2 + first digit address
  start();
  writeByte(TM1637_I2C_COMM2 + (pos & 0x03));

  // Write the data bytes
  for (uint8_t k=0; k < length; k++)
    writeByte(segments[k]);

  stop();

  // Write COMM3 + brightness
  start();
  writeByte(TM1637_I2C_COMM3 + (m_brightness & 0x0f));
  stop();
}

void TM1637Display::start(){
  pinMode(m_pinDIO, OUTPUT);
  delayMicroseconds(50);
}

void TM1637Display::stop(){
  pinMode(m_pinDIO, OUTPUT);
  delayMicroseconds(50);

  pinMode(m_pinClk, INPUT);
  delayMicroseconds(50);

  pinMode(m_pinDIO, INPUT);
  delayMicroseconds(50);
}

bool TM1637Display::writeByte(uint8_t b){
  uint8_t data = b;

  // 8 Data Bits
  for(uint8_t i = 0; i < 8; i++) {
    // CLK low
    pinMode(m_pinClk, OUTPUT);
    delayMicroseconds(50);

    // Set data bit
    if (data & 0x01)
      pinMode(m_pinDIO, INPUT);
    else
      pinMode(m_pinDIO, OUTPUT);

    delayMicroseconds(50);

    // CLK high
    pinMode(m_pinClk, INPUT);
    delayMicroseconds(50);
    data = data >> 1;
  }

  // Wait for acknowledge
  // CLK to zero
  pinMode(m_pinClk, OUTPUT);
  pinMode(m_pinDIO, INPUT);
  delayMicroseconds(50);

  // CLK to high
  pinMode(m_pinClk, INPUT);
  delayMicroseconds(50);

  uint8_t ack = digitalRead(m_pinDIO);

  if (ack == 0){
    pinMode(m_pinDIO, OUTPUT);
  }

  delayMicroseconds(50);
  pinMode(m_pinClk, OUTPUT);
  delayMicroseconds(50);

  return ack;
}
