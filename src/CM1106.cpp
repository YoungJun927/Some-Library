#include <cox.h>
#include "CM1106.hpp"

CM1106::CM1106(SerialPort &p) {
  this->port = &p;
  this->index = 0;
}

void CM1106::begin() {
  this->port->begin(9600);
  this->port->onReceive(SerialDataReceived, this);
  this->port->listen();
}

void CM1106::SerialDataReceived(void *ctx) {
  CM1106 *cm1106 = (CM1106 *) ctx;
  cm1106->eventDataReceived();
}

void CM1106::eventDataReceived() {
  while(this->port->available()>0){
    uint8_t data = this->port->read();

    if( ((this->index == 0) && (data != 0x16)) ||
        ((this->index == 1) && (data != 0x05)) ||
        ((this->index == 2) && (data != 0x01))) {
      this->index = 0;
      continue;
    } else if (this->index == 3) {
      this->CO2 = 0;
      this->CO2 |= (uint16_t)(data<<8);
    } else if (this->index == 4) {
      this->CO2 |= data;
    } else if (this->index ==7) {
      this->index = 0;
      this->gotCO2callback();
      continue;
    }
    this->index++;
  }
}

void CM1106::measurement(void (*func)()){
  this->gotCO2callback = func;
  char args[] = {0x11, 0x01, 0x01, 0xED};
  for(int j=0; j<4; j++){
    this->port->write(args[j]);
  }
}

uint16_t CM1106::getCO2() {
  return this->CO2;
}
