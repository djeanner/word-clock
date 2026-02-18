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
    fbgColor(bgColor)
{
  ftft = aScreen->getTFT();
}

Adafruit_ST7735* TFT_Window::getTFT() {return ftft;}

void TFT_Window::draw() {

  int ix = fx;
  int iy = fy;
  int iw = fw;
  int ih = fh;

  // draw frame
  if (fframeWidth > 0) {
    for (int i = 0; i < fframeWidth; i++) {
      ftft->drawRect(fx + i, fy + i,
                    fw - 2 * i, fh - 2 * i,
                    fframeColor);
    }

    ix += fframeWidth;
    iy += fframeWidth;
    iw -= 2 * fframeWidth;
    ih -= 2 * fframeWidth;
  }

  // fill background inside frame
  ftft->fillRect(ix, iy, iw, ih, fbgColor);
}
