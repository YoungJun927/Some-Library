#include <cox.h>
#include "Adafruit_GFX.hpp"
#include "SPI.hpp"

#define DOGM128_6_LCDWIDTH 128
#define DOGM128_6_LCDHEIGHT 64
#define BLACK 0
#define WHITE 1
#define INVERSE 2

class Dogm128_6 : public Adafruit_GFX{
  public:
    Dogm128_6(SPI &spi, int8_t lcdPwr, int8_t lcdReset, int8_t lcdPinCs, int8_t lcdMode);

    char buffer[1024] = {0x00,};
    uint8_t getRotation(void) const;

    void begin();
    void lcdClear();
    void lcdSendCommand(char *command, uint8_t length);
    void lcdSendData(char *data, uint16_t length);
    void lcdCursor(uint8_t column, uint8_t page);
    void lcdSendBuffer(char *buffer);
    void getCoX();
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

    inline void drawFastVLineInternal(int16_t x, int16_t y, int16_t h, uint16_t color) __attribute__((always_inline));
    inline void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) __attribute__((always_inline));

  private:
    SPI *spi = NULL;
    uint8_t accPinCs;
    uint8_t flashPinCs;
    uint8_t lcdPwr;
    uint8_t lcdReset;
    uint8_t lcdPinCs;
    uint8_t lcdMode;
    uint8_t rotation;
};
