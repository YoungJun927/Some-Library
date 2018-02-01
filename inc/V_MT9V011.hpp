#include <cox.h>

#define returnSign 0x76
#define serialNumber 0x00

class V_MT9V011 {
public:
  V_MT9V011(SerialPort &);
  void begin();
  void stopFrame();
  void sendData(char *args, uint8_t Len);
  void getLen();
  void getImage(void (*func)(const char *buf, uint32_t size));
  void resume();
  void setRatio(uint8_t ratio);
  void reset();
  void setSize(int size);
  void getVer();
  static void readImage(const char *buf, uint32_t size);

private:
  uint16_t index;
  uint16_t imageIndex = 0;
  uint32_t imageSize = 0;
  SerialPort *port;
  unsigned char flag = 0;
  char *imageBuf = NULL;
  uint8_t len[4] = {0,};
  uint8_t previousData = 0;

  void eventDataReceived();
  static void SerialDataReceived(void *ctx);
  void (*gotImageCallback)(const char *buf, uint32_t size) = NULL;

  enum flagType {
    stopFrameFlag = 1,
    dataLenFlag = 2,
    imageFlag = 4,
    getVerFlag = 8,
    resetFlag = 16,
    resumeFlag = 32
  };
};
