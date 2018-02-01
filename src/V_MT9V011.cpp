#include <cox.h>
#include "V_MT9V011.hpp"

V_MT9V011::V_MT9V011(SerialPort &p) {
  this->port = &p;
  this->index = 0;
}

void V_MT9V011::begin() {
  this->port->begin(38400);
  this->port->onReceive(SerialDataReceived, this);
  this->port->listen();
}

void V_MT9V011::SerialDataReceived(void *ctx) {
  V_MT9V011 *v_MT9V011 = (V_MT9V011 *) ctx;
  v_MT9V011->eventDataReceived();
}

void V_MT9V011::eventDataReceived(){
  while(this->port->available()>0){
    uint8_t data = this->port->read();

    if ((this->index == 0 && data != returnSign) ||
        (this->index == 1 && data != serialNumber)) {
          this->index = 0;
      continue;
    } else if((this->index == 2) && (data == 0x36)) {
      this->flag |= stopFrameFlag;
    } else if((this->index == 2) && (data == 0x34)) {
      this->flag |= dataLenFlag;
    } else if((this->index == 2) && (data == 0x32)) {
      this->flag |= imageFlag;
      uint32_t size = 0;
      size |= (((uint32_t) this->len[3]) << 24);
      size |= (((uint32_t) this->len[2])<<16);
      size |= (this->len[1]<<8);
      size |= this->len[0];
      printf("size : %04X\n",size );
      if (this->imageBuf == NULL) {
        // do something
        this->imageSize = 0;
      } else {
        this->imageSize = size;
        this->imageIndex = 0;
      }
    } else if((this->index == 2) && (data == 0x11)) {
      this->flag |= getVerFlag;
    } else if((this->index ==2) && (data == 0x26)) {
      this->flag |= resetFlag;
    } else if((this->flag == stopFrameFlag) && (this->index == 4) &&
            (this->previousData == 0x00) && (data == 0x00)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == dataLenFlag) && (this->index == 4) &&
            (this->previousData != 0x00) && (data != 0x04)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == imageFlag) && (this->index == 4) &&
            (this->previousData != 0x00) && (data != 0x00)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == imageFlag) && (this->index == 6) &&
              (this->previousData != 0xFF) && (data != 0xD8)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == getVerFlag) && (this->index == 15) &&
              (previousData == 0x30) && (data == 0x30)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == dataLenFlag) && (this->index == 6)) {
      this->len[3] = previousData;
      this->len[2] = data;
    } else if((this->flag == dataLenFlag) && (this->index == 8)) {
      this->len[1] = previousData;
      this->len[0] = data;
      this->flag = 0;
      this->index = 0;
      continue;
    } else if (this->flag == imageFlag && (this->index > 6) &&
              (this->index < (this->imageSize+4))) {
      this->imageBuf[this->imageIndex++] = data;
      if ((this->imageSize+3) == this->index && (data == 0xFF)) {
        if (this->gotImageCallback != NULL) {
          this->gotImageCallback(this->imageBuf, this->imageSize);
        }
        this->imageIndex = 0;
        dynamicFree(this->imageBuf);
        this->imageBuf = NULL;
      }
    } else if ( (this->flag == imageFlag) && ((this->imageSize+9)==this->index) &&
                (data == 0x00) && (previousData ==0x00)){
      this->index = 0;
      this->flag = 0;
      this->imageSize = 0;
      this->len[0] = 0;
      this->len[1] = 0;
      this->len[2] = 0;
      this->len[3] = 0;
    }
    this->previousData = data;
    index++;
  }
}

void V_MT9V011::sendData(char *args, uint8_t Len){
  for(int j=0; j<Len; j++,args++){
    this->port->write(*args);
  }
}

void V_MT9V011::getLen(){
  char args[] = {0x56, 0x00, 0x34, 0x01, 0x00};
  sendData((char *)args, sizeof(args));
}

void V_MT9V011::stopFrame(){
  char args[] = {0x56, 0x00, 0x36, 0x01, 0x00};
  sendData((char *)args, sizeof(args));
}

// void V_MT9V011::getimage(void (*func)(const char *buf, uint16_t size)){
void V_MT9V011::getImage(void (*func)(const char *buf, uint32_t size)){
  this->imageBuf = (char *) dynamicMalloc(this->imageSize);
  char args[] = {0x56, 0x00, 0x32, 0x0C, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00,
                 this->len[3], this->len[2], this->len[1], this->len[0], 0x0B, 0xB8};
  sendData((char*)args, sizeof(args));
  this->gotImageCallback = func;
}

void V_MT9V011::getVer(){
  char args[] = {0x56, 0x00, 0x11, 0x00};
  sendData((char*)args, sizeof(args));
}

void V_MT9V011::reset(){
  char args[] = {0x56, 0x00, 0x36, 0x00, 0x00};
  sendData((char*)args, sizeof(args));
}

void V_MT9V011::setRatio(uint8_t ratio){
  char args[] = {0x56, 0x00, 0x31, 0x05, 0x01, 0x01, 0x12, 0x04, ratio};
  sendData((char*)args, sizeof(args));
}

void V_MT9V011::resume(){
  char args[] = {0x56, 0x00, 0x36, 0x01, 0x03};
  sendData((char*)args, sizeof(args));
}
