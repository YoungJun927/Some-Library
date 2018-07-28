#include <cox.h>

class CM1106 {
public:
  CM1106(SerialPort &);
  void begin();
  void sendData(char *args, uint8_t Len);
  void measurement(void (*func)());
  uint16_t CO2=0;
  uint16_t getCO2();
private:
  SerialPort *port;
  uint8_t index;
  void eventDataReceived();
  static void SerialDataReceived(void *ctx);
  void (*gotCO2callback)() = NULL;
};
