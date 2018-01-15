#include <cox.h>
#include <Adafruit_SSD1306.hpp>
#include <Adafruit_GFX.hpp>

#define OLED_RESET D2
Adafruit_SSD1306 display(OLED_RESET, Wire,0x3D);

Timer timerHello;

void testdrawrect(void) {
  for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
    delay(1);
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.display();
    delay(1);
    color++;
  }
}

void testdrawcircle(void) {
  for (int16_t i=0; i<display.height(); i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();
    delay(1);
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
    delay(1);
  }
}

static void taskHello(void*) {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.drawChar(0,0,'A',WHITE,1,1);

  display.display();
  delay(2000);

  display.clearDisplay();
  display.drawPixel(10, 10, WHITE);
  display.display();
  delay(2000);

  display.clearDisplay();
  testdrawrect();
  display.display();
  delay(2000);

  display.clearDisplay();
  testfillrect();
  display.display();
  delay(2000);

  display.clearDisplay();
  testfillroundrect();
  display.display();
  delay(2000);
  display.clearDisplay();
}

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  timerHello.onFired(taskHello,NULL);
  timerHello.startPeriodic(5000);
 }
