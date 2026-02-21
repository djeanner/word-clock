#pragma once
#include <Adafruit_ST7735.h>
#include "TFT_Screen.h"

class TFT_Window {
protected:
  TFT_Screen* fScreen;
  const int fx, fy, fw, fh;
  int fframeWidth;
  uint16_t fframeColor;
  uint16_t fbgColor;
  uint8_t fWidth;
  uint8_t fHeigth;
  uint8_t fXpos;
  uint8_t fYpos;
  Adafruit_ST7735* ftft;

public:
  TFT_Window(TFT_Screen* aScreen,
             int x, int y, int w, int h,
             int frame,
             uint16_t frameColor,
             uint16_t bgColor);

  Adafruit_ST7735* getTFT();
  uint8_t getWidth() {return fWidth;}
  uint8_t getHeight() {return fHeigth;}
  uint8_t getXpos() {return fXpos;}
  uint8_t getYpos() {return fYpos;}
  virtual void draw();
};
