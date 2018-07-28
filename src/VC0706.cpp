#include <cox.h>
#include "VC0706.hpp"

Timer captureCycle;

VC0706::VC0706(SerialPort &p) {
  this->port = &p;
  this->index = 0;
}

void VC0706::begin() {
  this->port->begin(38400);
  this->port->onReceive(SerialDataReceived, this);
  this->port->listen();
}

void VC0706::SerialDataReceived(void *ctx) {
  VC0706 *vC0706 = (VC0706 *) ctx;
  vC0706->eventDataReceived();
}

void VC0706::eventDataReceived() {
  while(this->port->available()>0) {
    this->data = this->port->read();
//===================== check returnSign, serialNumber ======================
    if ((this->index == 0 && this->data != returnSign) ||
        (this->index == 1 && this->data != serialNumber)) {
      this->index = 0;
      continue;
//============================ check the flag ================================
    } else if((this->index == 2) && (this->data == 0x36)) {
      this->flag |= stopFrameFlag;
    } else if((this->index == 2) && (this->data == 0x34)) {
      this->flag |= dataLenFlag;
    } else if((this->index == 2) && (data == 0x32)) {
      this->flag |= imageFlag;
      uint32_t size = 0;
      size |= (((uint32_t) this->len[3]) << 24);
      size |= (((uint32_t) this->len[2])<<16);
      size |= (this->len[1]<<8);
      size |= this->len[0];
      if (this->imageBuf == NULL) {
        // do something
        this->imageSize = 0;
      } else {
        this->imageSize = size;
        this->imageIndex = 0;
      }
    } else if((this->index == 2) && (this->data == 0x11)) {
      this->flag |= getVerFlag;
    } else if((this->index == 2) && (this->data == 0x26)) {
      this->flag |= resetFlag;
    } else if((this->index == 2) && (this->data == 0x42)) {
      this->flag |= motionCtrlFlag;
    } else if((this->index == 2) && (this->data == 0x43)) {
      this->flag |= captureFlag;
//============ check return value and reset the flag, index ===================
    } else if((this->flag == stopFrameFlag) && (this->index == 4) &&
            (this->previousData == 0x00) && (this->data == 0x00)) {
      this->index = 0;
      this->flag = 0;
      this->getLen();
      continue;
    } else if((this->flag == dataLenFlag) && (this->index == 4) &&
            (this->previousData != 0x00) && (this->data != 0x04)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == imageFlag) && (this->index == 4) &&
            (this->previousData != 0x00) && (this->data != 0x00)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == motionCtrlFlag) && (this->index == 4) &&
              (this->previousData == 0x00) && (this->data ==0x00)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == imageFlag) && (this->index == 6) &&
              (this->previousData != 0xFF) && (this->data != 0xD8)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == captureFlag) && (this->index == 6) &&
              (this->previousData == 0x01) && (this->data == 0x11)) {
      this->flag = 0;
      this->index = 0;
      //callback
      this->successCapture();
      continue;
    } else if((this->flag == captureFlag) && (this->index == 6) &&
              (this->previousData == 0x01) && (this->data == 0x01)) {
      this->flag = 0;
      this->index = 0;
      continue;
    } else if((this->flag == getVerFlag) && (this->index == 15) &&
              (previousData == 0x30) && (this->data == 0x30)) {
      this->index = 0;
      this->flag = 0;
      continue;
    } else if((this->flag == dataLenFlag) && (this->index == 6)) {
      this->len[3] = this->previousData;
      this->len[2] = this->data;
    } else if((this->flag == dataLenFlag) && (this->index == 8)) {
      this->len[1] = this->previousData;
      this->len[0] = this->data;
      this->flag = 0;
      this->index = 0;
      this->getLenCallback();
      continue;
    } else if ( (this->flag == imageFlag) && ((this->imageSize+9)==this->index) &&
                (this->data == 0x00) && (previousData ==0x00)){
      this->index = 0;
      this->flag = 0;
      this->imageSize = 0;
      this->len[0] = 0;
      this->len[1] = 0;
      this->len[2] = 0;
      this->len[3] = 0;
//============================== save the imageBuf ============================
    } else if (this->flag == imageFlag && (this->index > 6) &&
              (this->index < (this->imageSize+4))) {
      this->imageBuf[this->imageIndex++] = this->data;
      if ((this->imageSize+3) == this->index && (this->data == 0xFF)) {
        if (this->gotImageCallback != NULL) {
          this->gotImageCallback(this->imageBuf, this->imageSize);
        }
        this->imageIndex = 0;
        dynamicFree(this->imageBuf);
        this->imageBuf = NULL;
      }
    }
    this->previousData = this->data;
    index++;
  }
}

void VC0706::sendData(char *args, uint8_t Len) {
  for(int j=0; j<Len; j++,args++){
    this->port->write(*args);
  }
}

void VC0706::getLen() {
  char args[] = {0x56, 0x00, 0x34, 0x01, 0x00};
  sendData((char *)args, sizeof(args));
}

void VC0706::stopFrame(void (*func)()) {
  this->getLenCallback = func;
  char args[] = {0x56, 0x00, 0x36, 0x01, 0x00};
  sendData((char *)args, sizeof(args));
}

void VC0706::getImage(void (*func)(const char *buf, uint32_t size)) {
  this->imageBuf = (char *) dynamicMalloc(this->imageSize);
  char args[] = {0x56, 0x00, 0x32, 0x0C, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00,
                 this->len[3], this->len[2], this->len[1], this->len[0], 0x0B, 0xB8};
  sendData((char*)args, sizeof(args));
  this->gotImageCallback = func;
}

void VC0706::getVer() {
  char args[] = {0x56, 0x00, 0x11, 0x00};
  sendData((char*)args, sizeof(args));
}

void VC0706::reset() {
  char args[] = {0x56, 0x00, 0x36, 0x00, 0x00};
  sendData((char*)args, sizeof(args));
}

void VC0706::setRatio(uint8_t ratio) {
  char args[] = {0x56, 0x00, 0x31, 0x05, 0x01, 0x01, 0x12, 0x04, ratio};
  sendData((char*)args, sizeof(args));
}

void VC0706::resume() {
  char args[] = {0x56, 0x00, 0x36, 0x01, 0x03};
  sendData((char*)args, sizeof(args));
}

void VC0706::setMotionCtrl(uint8_t len, uint8_t motionAttribute, uint8_t ctrlItme, uint8_t firstBit, uint8_t secondBit) {
  if(ctrlItme==1 ) {
    char args[] = {0x56, 0x00, 0x42, len, motionAttribute, ctrlItme, firstBit, secondBit};
    this->motionCycle |= (firstBit<<8);
    this->motionCycle |= secondBit;
    sendData((char*)args, sizeof(args));
  } else{
    char args[] = {0x56, 0x00, 0x42, len, motionAttribute, ctrlItme, firstBit};
    sendData((char*)args, sizeof(args));
  }
}

void VC0706::startCapture(void (*func)(), uint16_t cycle) {
  this->successCapture = func;
  captureCycle.onFired(motionStatus,this);
  captureCycle.startPeriodic(cycle);
}

void VC0706::motionStatus(void *ctx) {
  //check the motionStatus
  VC0706 *vC0706 = (VC0706 *) ctx;
  char args[] = {0x56, 0x00, 0x43, 0x01, 0x00};
  vC0706->sendData((char*)args, sizeof(args));
}

void VC0706::endCapture() {
  //if you want to exit startCapture, run the endCapture()
  this->flag = 0;
  this->index = 0;
  captureCycle.stop();
}
