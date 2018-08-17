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
//===================== check returnSign & serialNumber ======================
    if ((this->index == 0 && this->data != returnSign) ||
        (this->index == 1 && this->data != serialNumber)) {
        this->index = 0;
        continue;
//============================ check the flag ================================
    } else if((this->index == 2) && (data == 0x32)) {
        if (this->imageBuf == NULL) {
          // do something
          this->imageSize = 0;
        } else {
          this->imageIndex = 0;
        }
    }
    //============ check return value and reset the flag, index ===================
      else if ((this->flag == resetFlag) && (this->index == 4) &&
            (this->previousData == 0x00) && (this->data == 0x00)) {
        //Finish the reset !
        this->index = 0;
        this->flag = 0;
        continue;
    } else if((this->flag == stopFrameFlag) && (this->index == 4) &&
            (this->previousData == 0x00) && (this->data == 0x00)) {
        //Finish the stopFrame !
        this->index = 0;
        this->flag = 0;
        if(this->atOnce == 1) { getLen(); }
        continue;
    } else if((this->flag == recoverFlag) && (this->index == 4) &&
            (this->previousData == 0x00) && (this->data == 0x00)) {
        //Finish recoverFrame !
        this->index = 0;
        this->flag = 0;
        delay(100);
        if(this->setRatioCallback != NULL){
          this->setRatioCallback();
          setRatioCallback = NULL;
        }
        if(this->recoverFrameCallback != NULL){
          this->recoverFrameCallback();
        }
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
        recoverFrame(NULL,this);
        continue;
    } else if((this->flag == motionCtrlFlag) && (this->index == 4) &&
              (this->previousData == 0x00) && (this->data == 0x00)) {
        //Finish the setMotionCtrl !
        this->index = 0;
        this->flag = 0;
        continue;
    } else if((this->flag == compressionFlag) && (this->index == 4) &&
              (this->previousData == 0x00) && (this->data == 0x00)){
        //Finish the setRatio & Start recoverFrame !
        this->index = 0;
        this->flag = 0;
        recoverFrame(NULL,this);
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
        this->successCapture();
        continue;
    } else if((this->flag == captureFlag) && (this->index == 6) &&
              (this->previousData == 0x01) && (this->data == 0x01)) {
        this->flag = 0;
        this->index = 0;
        continue;
    } else if((this->flag == getVerFlag) && (this->index == 15) &&
              (previousData == 0x30) && (this->data == 0x30)) {
        //Finish the get version !
        this->index = 0;
        this->flag = 0;
        continue;
    } else if((this->flag == dataLenFlag) && (this->index == 6)) {
        //Take a High 16-bits of image length !
        this->len[3] = this->previousData;
        this->len[2] = this->data;
        size |= (((uint32_t) this->len[3]) << 24);
        size |= (((uint32_t) this->len[2])<<16);
    } else if((this->flag == dataLenFlag) && (this->index == 8)) {
      //Take a Low 16-bits of image length & Finish the getLen !
        this->len[1] = this->previousData;
        this->len[0] = this->data;
        size |= (this->len[1]<<8);
        size |= this->len[0];
        this->imageSize = size;
        this->flag = 0;
        this->index = 0;
        if(this->atOnce == 1) { getImage(NULL); }
        continue;
    }
//============================== save the imageBuf or get the version ============================
      else if (this->flag == imageFlag && (this->index > 4) &&
              (this->index <= (this->imageSize+4))) {

        this->imageBuf[this->imageIndex++] = this->data;

    } else if (this->flag == getVerFlag && (this->index > 4) &&
            (this->index <= 14)) {

        this->version[this->versionIndex++] = this->data;

        if(this->versionIndex == 10) {
          //Print the version !
          for (int i=0; i<11; i++){
              printf("%c",this->version[i]);
              if(i==10) {printf("\n" );}
          }
          this->flag = 0;
          this->index = 0;
          this->versionIndex = 0;
        }

    } else if (this->flag == imageFlag && ((this->index)==(this->imageSize+6)) &&
              (this->data == 0x00) && (previousData ==0x76)) {
        if (this->takePictureCallback != NULL) {
          this->takePictureCallback(this->imageBuf, this->imageSize);
        }

        dynamicFree(this->imageBuf);
        this->imageBuf = NULL;
    } else if ( (this->flag == imageFlag) && ((this->imageSize+9)==this->index) &&
                (this->data == 0x00) && (previousData ==0x00)){
        //Finish the getImage & Start the recoverFrame !
        this->index = 0;
        this->flag = 0;
        this->imageIndex = 0;
        this->imageSize = 0;
        this->len[0] = 0;
        this->len[1] = 0;
        this->len[2] = 0;
        this->len[3] = 0;
        size=0;
        atOnce=0;
        recoverFrame(NULL,this);
        continue;
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

void VC0706::takePicture(void (*func)(const char *buf, uint32_t size)) {
  if (this->flag == 0) {
    this->takePictureCallback = func;
    this->flag |= stopFrameFlag;
    this->atOnce = 1;
    char args[] = {0x56, 0x00, 0x36, 0x01, 0x00};
    sendData(args, sizeof(args));
  } else {
    //Busy now !
    reset();
  }
}

void VC0706::stopFrame() {
  //Captures the current frame
  this->flag |= stopFrameFlag;
  char args[] = {0x56, 0x00, 0x36, 0x01, 0x00};
  sendData(args, sizeof(args));
}

void VC0706::getLen() {
  //Take a length
  this->flag |= dataLenFlag;
  char args[] = {0x56, 0x00, 0x34, 0x01, 0x00};
  sendData(args, sizeof(args));
}


void VC0706::getImage(void (*func)(const char *buf, uint32_t size)) {
  //Take a image data
  this->imageBuf = (char *) dynamicMalloc(this->imageSize);

  if( (this->imageBuf == NULL)){
    //Not enough memory
    this->index = 0;
    this->flag = 0;
    this->imageIndex = 0;
    this->imageSize = 0;
    this->len[0] = 0;
    this->len[1] = 0;
    this->len[2] = 0;
    this->len[3] = 0;
    size=0;

    if (this->ratio > 0xF6) {
      //Max value of the ratio
      this->ratio = 0xFF;
    }
    else {
      //Increase compression ratio
      this->ratio +=10;
    }

    if (this->previousRatio == this->ratio) {
      //I need more memory
      getVer();
    }

    this->previousRatio = this->ratio;
    setRatio(NULL,this->ratio);
  }
  else {
    //Take a image data
    this->flag |= imageFlag;
    char args[] = {0x56, 0x00, 0x32, 0x0C, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00,
                   this->len[3], this->len[2], this->len[1], this->len[0], 0x00, 0x0A};
    sendData(args, sizeof(args));
  }
}

void VC0706::getVer() {
  //Take a Version
  this->flag |= getVerFlag;
  char args[] = {0x56, 0x00, 0x11, 0x00};
  sendData(args, sizeof(args));
}

void VC0706::reset() {
  //System reset

  while(this->port->available()>0){
    this->data = this->port->read();
  }

  this->flag |= resetFlag;
  this->data = 0;
  this->index = 0;
  char args[] = {0x56, 0x00, 0x26, 0x00};
  sendData(args, sizeof(args));
}

void VC0706::setRatio(void (*func)(), uint8_t compRatio) {
  this->flag |= compressionFlag;
  this->setRatioCallback = func;
  //Max value of the compression ratio
  if(compRatio>0xFF) { compRatio=0xFF; }
  //Min value of the compression ratio
  if(compRatio<0x00) { compRatio=0x00; }
  this->ratio = compRatio;
  char args[] = {0x56, 0x00, 0x31, 0x05, 0x01, 0x01, 0x12, 0x04, this->ratio};
  this->sendData(args, sizeof(args));
}

void VC0706::recoverFrame(void (*func)(), void *ctx ) {
  //Recover Frame for next picture
  VC0706 *vC0706 = (VC0706 *)ctx;
  vC0706->recoverFrameCallback = func;
  vC0706->flag |= recoverFlag;
  char args[] = {0x56, 0x00, 0x36, 0x01, 0x03};
  vC0706->sendData(args, sizeof(args));
}

void VC0706::setMotionCtrl(uint8_t len, uint8_t motionAttribute, uint8_t ctrlItme, uint8_t firstBit, uint8_t secondBit) {
  //Set the motion control
  this->flag |= motionCtrlFlag;
  if(ctrlItme==1 ) {
    char args[] = {0x56, 0x00, 0x42, len, motionAttribute, ctrlItme, firstBit, secondBit};
    this->motionCycle |= (firstBit<<8);
    this->motionCycle |= secondBit;
    sendData(args, sizeof(args));
  } else{
    char args[] = {0x56, 0x00, 0x42, len, motionAttribute, ctrlItme, firstBit};
    sendData(args, sizeof(args));
  }
}

void VC0706::startCapture(void (*func)(), uint16_t cycle) {
  //Start a capture
  this->flag |= captureFlag;
  this->successCapture = func;
  captureCycle.onFired(motionStatus,this);
  captureCycle.startPeriodic(cycle);
}

void VC0706::motionStatus(void *ctx) {
  //check the motionStatus
  VC0706 *vC0706 = (VC0706 *) ctx;
  char args[] = {0x56, 0x00, 0x43, 0x01, 0x00};
  vC0706->sendData(args, sizeof(args));
}

void VC0706::endCapture() {
  //if you want to exit startCapture, run the endCapture()
  this->flag = 0;
  this->index = 0;
  captureCycle.stop();
}
