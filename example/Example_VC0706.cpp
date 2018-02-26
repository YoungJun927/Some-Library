#include <cox.h>
#include "VC0706.hpp"

Timer timerHello;
VC0706 camera(Serial2);
int i=0;

static void readLen(){
  printf("readImage: " );
  printf("\n");
}

static void readImage(const char *buf, uint32_t size) {
  for(uint32_t i=0; i<size;i++){
    System.feedWatchdog();
    printf("%02X ", buf[i]);
  }
}

static void taskHello(void *) {
  if(i==0){
    // camera.stopFrame(readLen);//함수를 실행하면 getLen()까지 실행되고 readImage 콜백

    camera.setMotionCtrl(0x04,0x01,0x01,0x01,0x00);
    //motion_ctrl 사진파일 첨부, setMotionCtrl(dataLen,alarm-output attribute,0100*10ms)

  } else if(i==1) {
    // camera.getImage(readImage);//이미지를 모두 받으면 readImage() 콜백

    camera.setMotionCtrl(0x03,0x00,0x01,0x01);
    //setMotionCtrl(dataLen,motion control and enabling control,UART,start motion monitoring)
  } else if(i==2){
    printf("i == %d\n",i );
    camera.startCapture(readLen, 1000);
  }
  i++;
}
void setup() {
  Serial.begin(115200);
  Serial.println("***** Serial.begin *****" );
  camera.begin();

  timerHello.onFired(taskHello, NULL);
  timerHello.startPeriodic(2500);

}
