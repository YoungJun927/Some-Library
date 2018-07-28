#include <cox.h>

#define returnSign 0x76
#define serialNumber 0x00

class VC0706 {
public:
  VC0706(SerialPort &);

  void begin();
  void stopFrame(void (*func)());
  void sendData(char *args, uint8_t Len);
  void getLen();
  void getImage(void (*func)(const char *buf, uint32_t size));
  void resume();
  void setRatio(uint8_t ratio);
  void reset();
  void setSize(int size);
  void getVer();
  void startCapture(void (*func)(), uint16_t cycle);
  void endCapture();
  void setMotionCtrl (uint8_t len, uint8_t motionAttribute, uint8_t ctrlItme, uint8_t secondBit, uint8_t thirdBit=0);

  static void motionStatus(void *);
  static void readImage(const char *buf, uint32_t size);
  enum flagType {
    stopFrameFlag = 1,
    dataLenFlag = 2,
    imageFlag = 4,
    getVerFlag = 8,
    resetFlag = 16,
    resumeFlag = 32,
    motionCtrlFlag = 64,
    captureFlag = 128
  };
private:
  uint8_t len[4] = {0,};
  uint8_t data = 0;
  uint8_t previousData = 0;
  uint16_t index;
  uint16_t imageIndex = 0;
  uint32_t imageSize = 0;
  uint32_t motionCycle = 0;

  unsigned char flag = 0;
  char *imageBuf = NULL;
  SerialPort *port;

  void eventDataReceived();
  static void SerialDataReceived(void *ctx);
  void (*gotImageCallback)(const char *buf, uint32_t size) = NULL;
  void (*successCapture)() = NULL;
  void (*getLenCallback)() = NULL;
  void (*stopFrameCallback)() = NULL;
};
