#pragma once
#include "TFT_Window.h"

class GeigerWindow : public TFT_Window {
  uint16_t ftextColor;
  uint8_t ftextSize;
  uint8_t fdotSize;
  uint8_t fYmap;
  uint8_t fLastPos;
public:
  GeigerWindow(TFT_Screen* aScreen,
               int x, int y, int w, int h,
               int frame,
               uint16_t frameColor,
               uint16_t bgColor,
               uint16_t textColor,
               uint8_t textSize = 1,
               uint8_t dotSize = 1
               );
// void updateBit(int index, int bitData, int miniString, int indexCrude, int unused);
void pixel(const int16_t x, const int16_t y, const uint16_t col);
void miniNum(const int16_t val, const int16_t x, const int16_t y, const uint16_t col);
void draw() override;
};
