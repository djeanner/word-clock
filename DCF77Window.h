#pragma once
#include "TFT_Window.h"

class DCF77Window : public TFT_Window {
  uint16_t ftextColor;
  uint8_t ftextSize;
  uint8_t fdotSize;
public:
  DCF77Window(TFT_Screen* aScreen,
               int x, int y, int w, int h,
               int frame,
               uint16_t frameColor,
               uint16_t bgColor,
               uint16_t textColor,
               uint8_t textSize = 1,
               uint8_t dotSize = 1
               );
void pixel(const int16_t x, const int16_t y, const uint16_t col);
void miniNum(const int16_t val, const int16_t x, const int16_t y, const uint16_t col);

  void draw() override;
};
