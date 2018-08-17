#include <cox.h>

#define returnSign 0x76
#define serialNumber 0x00

class VC0706 {
public:
  VC0706(SerialPort &);

  void begin();
  void getVer();
  void takePicture(void (*func)(const char *buf, uint32_t size));
  void setRatio(void (*func)(), uint8_t compRatio);
  void stopFrame();
  void getLen();
  void getImage();
  void getImage(void (*func)(const char *buf, uint32_t size));
  void reset();
  void startCapture(void (*func)(), uint16_t cycle);
  void endCapture();
  void setMotionCtrl (uint8_t len, uint8_t motionAttribute, uint8_t ctrlItme, uint8_t secondBit, uint8_t thirdBit=0);
  void sendData(char *args, uint8_t Len);

  static void recoverFrame(void (*func)( ), void *ctx);
  static void motionStatus(void *);

  char *imageBuf = NULL;
  char version[12] = {0,};
  uint8_t ratio = 0x36;
  uint8_t previousRatio = 0;
  uint32_t size = 0;
  uint32_t imageSize = 0;
private:
  SerialPort *port;

  int atOnce = 0;
  unsigned char flag = 0;
  uint8_t versionIndex = 0;
  uint8_t len[4] = {0,};
  uint8_t data = 0;
  uint8_t previousData = 0;
  uint16_t index = 0;
  uint16_t imageIndex = 0;
  uint32_t motionCycle = 0;

  enum flagType {
    stopFrameFlag = 0x01,
    dataLenFlag = 0x02,
    imageFlag = 0x03,
    getVerFlag = 0x04,
    resetFlag = 0x05,
    recoverFlag = 0x06,
    motionCtrlFlag = 0x07,
    captureFlag = 0x08,
    compressionFlag = 0x09,
  };

  void eventDataReceived();
  void (*successCapture)() = NULL;
  void (*recoverFrameCallback)() = NULL;
  void (*setRatioCallback)() = NULL;
  void (*takePictureCallback)(const char *buf, uint32_t size) = NULL;
  static void SerialDataReceived(void *ctx);
};
