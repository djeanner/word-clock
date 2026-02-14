#pragma once
#include <Adafruit_ST7735.h>
#include "TFT_Screen.h"

class TFT_Window {
protected:
  TFT_Screen* fScreen;
  Adafruit_ST7735* ftft;
  uint8_t fWidth;
  uint8_t fHeigth;
  // original geometry (never modified)
  int fx, fy, fw, fh;

  int fframe;
  uint16_t fframeColor;
  uint16_t fbgColor;

public:
  TFT_Window(TFT_Screen* aScreen,
             int x, int y, int w, int h,
             int frame,
             uint16_t frameColor,
             uint16_t bgColor);

  Adafruit_ST7735* getTFT();
  uint8_t getWidth() {return fWidth;}
  uint8_t getHeight() {return fHeigth;}
  virtual void draw();
};
