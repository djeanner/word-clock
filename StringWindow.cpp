#include "StringWindow.h"

StringWindow::StringWindow(TFT_Screen* aScreen,
                           int x, int y, int w, int h,
                           int frame,
                           uint16_t frameColor,
                           uint16_t bgColor,
                           String text,
                           uint16_t textColor,
                           uint8_t textSize,
                           int tx,
                           int ty)
  : TFT_Window(aScreen, x, y, w, h, frame, frameColor, bgColor),
    ftext(text),
    ftextColor(textColor),
    ftextSize(textSize),
    ftx(tx),
    fty(ty)
{}

void StringWindow::draw() {

  TFT_Window::draw();
  if (getWidth() > -1) {}
  if (getHeight() > -1) {}
  int ix = fx + fframeWidth;
  int iy = fy + fframeWidth;
  int iw = fw - 2 * fframeWidth;
  int ih = fh - 2 * fframeWidth;

  ftft->setTextColor(ftextColor);
  ftft->setTextSize(ftextSize);

  int ty = fty;
  if (ty < 0)
    ty = iy + ih / 2 - (8 * ftextSize / 2);

  ftft->setCursor(ix + ftx, ty);
  ftft->print(ftext);
}
