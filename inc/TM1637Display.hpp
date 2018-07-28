#ifndef __TM1637DISPLAY__
#define __TM1637DISPLAY__

#include <inttypes.h>

class TM1637Display {
public:
  void setBrightness(uint8_t brightness, bool on = true);
  void setSegments(const uint8_t segments[], uint8_t length = 4, uint8_t pos = 0);
  void showNumberDec(int num, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);
  void showNumberDecEx(int num, uint8_t dots = 0, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);

  uint8_t encodeDigit(uint8_t digit);

  TM1637Display(uint8_t pinClk, uint8_t pinDIO);

protected:
  void bitDelay();
  void start();
  void stop();

  bool writeByte(uint8_t b);

private:
  uint8_t m_pinClk;
  uint8_t m_pinDIO;
  uint8_t m_brightness;
};

#endif
