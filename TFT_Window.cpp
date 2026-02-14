#include "TFT_Window.h"

TFT_Window::TFT_Window(TFT_Screen* aScreen,
                       int x, int y, int w, int h,
                       int frame,
                       uint16_t frameColor,
                       uint16_t bgColor)
  : fScreen(aScreen),
    fx(x), fy(y), fw(w), fh(h),
    fframe(frame),
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
  if (fframe > 0) {
    for (int i = 0; i < fframe; i++) {
      ftft->drawRect(fx + i, fy + i,
                    fw - 2 * i, fh - 2 * i,
                    fframeColor);
    }

    ix += fframe;
    iy += fframe;
    iw -= 2 * fframe;
    ih -= 2 * fframe;
  }

  // fill background inside frame
  ftft->fillRect(ix, iy, iw, ih, fbgColor);
}
