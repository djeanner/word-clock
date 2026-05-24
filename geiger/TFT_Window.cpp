#include "TFT_Window.h"

TFT_Window::TFT_Window(TFT_Screen* aScreen,
                       int x, int y, int w, int h,
                       int frame,
                       uint16_t frameColor,
                       uint16_t bgColor)
  : fScreen(aScreen),
    fx(x), fy(y), fw(w), fh(h),
    fframeWidth(frame),
    fframeColor(frameColor),
    fbgColor(bgColor),
    fWidth(0),
    fHeigth(0),
    fXpos(0),
    fYpos(0)
{
  ftft = aScreen->getTFT();
  fXpos = fx + fframeWidth;
  fYpos = fy + fframeWidth;
  fWidth = fw - 2 * fframeWidth;
  fHeigth = fh - 2 * fframeWidth; 
}
#ifdef ARDUINO_ARCH_RP2040
Adafruit_ST7735* TFT_Window::getTFT() {return ftft;}
#else
ST7735* TFT_Window::getTFT() {return ftft;}
#endif

void TFT_Window::draw() {
  // draw frame
  if (fframeWidth > 0) {
    for (int i = 0; i < fframeWidth; i++) {
      ftft->drawRect(fx + i, fy + i,
                    fw - 2 * i, fh - 2 * i,
                    fframeColor);
    }
  }
  
  // fill background inside frame
  ftft->fillRect(fXpos, fYpos, fWidth, fHeigth, fbgColor);
}
