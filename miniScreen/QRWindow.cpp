#include "QRWindow.h"

QRWindow::QRWindow(TFT_Screen *aScreen, int x, int y, int w, int h, int frame,
                   uint16_t frameColor, uint16_t bgColor, uint16_t pointColor,
                   uint16_t backColor, uint8_t pointSize, uint8_t spaceAround)
    : TFT_Window(aScreen, x, y, w, h, frame, frameColor, bgColor),
      fpointColor(pointColor), fbackColor(backColor), fpointSize(pointSize),
      fspaceAround(spaceAround) {}

void QRWindow::draw() { TFT_Window::draw(); }

void QRWindow::drawQR(qrcodegen::QrCode qrCode) {
  const int sizeQR =
      qrCode.getSize() + 2 * fspaceAround; // 4 is QR request for frame
  bool isOK = false;
  const int widthScreen = getWidth();
  const int heightScreen = getHeight();
  int thePointSize = 1;
  int shiftX = 0;
  int shiftY = 0;
  for (uint8_t testSize = fpointSize; testSize > 0; testSize -= 1) {

    const int wx = testSize * sizeQR;
    const int wy = testSize * sizeQR;
    const bool fitInScreenX = (wx < fWidth);
    const bool fitInScreenY = (wy < fHeigth);
    if (fitInScreenX && fitInScreenY) {
      thePointSize = testSize;
      shiftX = (fWidth - wx) / (2 * thePointSize);
      shiftY = (fHeigth - wy) / (2 * thePointSize);
      isOK = true;
      break;
    }
  }
  if (isOK) {
    for (int y = 0; y < sizeQR; y++) {
      for (int x = 0; x < sizeQR; x++) {
        const bool pt = qrCode.getModule(x - fspaceAround, y - fspaceAround);
        if (pt) {
          pixel(shiftX + x, shiftY + y, fpointColor, thePointSize);
        } else {
          pixel(shiftX + x, shiftY + y, fbackColor, thePointSize);
        }
      }
    }
  } else { // draw a diag bar
    for (int x = 0; x < (fWidth / fpointSize) && x < (fHeigth / fpointSize);
         x++) {
      pixel(x, x, ST77XX_RED, fpointSize);
    }
  }
}

void QRWindow::pixel(const int16_t x, const int16_t y, const uint16_t col,
                     const int thePointSize) {
  if (thePointSize == 2) {
    ftft->drawPixel(getXpos() + 2 * x, getYpos() + 2 * y, col);
    ftft->drawPixel(getXpos() + 2 * x + 1, getYpos() + 2 * y, col);
    ftft->drawPixel(getXpos() + 2 * x, getYpos() + 2 * y + 1, col);
    ftft->drawPixel(getXpos() + 2 * x + 1, getYpos() + 2 * y + 1, col);
    return;
  }
  if (thePointSize == 2) {
    ftft->drawPixel(getXpos() + x, getYpos() + y, col);
  }
  for (int xx = 0; xx < thePointSize; xx++) {
    for (int yy = 0; yy < thePointSize; yy++) {
      ftft->drawPixel(getXpos() + thePointSize * x + xx,
                      getYpos() + thePointSize * y + yy, col);
    }
  }
}