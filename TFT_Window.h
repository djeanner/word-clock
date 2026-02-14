#pragma once
#include <Adafruit_ST7735.h>

class TFT_Window {
protected:
  Adafruit_ST7735* ftft;

  // original geometry (never modified)
  int fx, fy, fw, fh;

  int fframe;
  uint16_t fframeColor;
  uint16_t fbgColor;

public:
  TFT_Window(Adafruit_ST7735* tft,
             int x, int y, int w, int h,
             int frame,
             uint16_t frameColor,
             uint16_t bgColor);

  Adafruit_ST7735* getTFT();
  virtual void draw();
};
