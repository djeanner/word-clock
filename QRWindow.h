#pragma once

#include "TFT_Window.h"

#ifdef ARDUINO_ARCH_RP2040
  #include "qrcodegen.hpp"
#else
  #include "lib/qrcodegen/qrcodegen.hpp"
#endif
class QRWindow : public TFT_Window {
private:
  uint16_t fpointColor;
  uint16_t fbackColor;
  uint8_t fpointSize;
  uint8_t fspaceAround;

public:
  QRWindow(TFT_Screen *aScreen, int x, int y, int w = 50, int h = 50,
           int frame = 1, uint16_t frameColor = ST77XX_WHITE,
           uint16_t bgColor = ST77XX_BLACK, uint16_t pointColor = ST77XX_WHITE,
           uint16_t backColor = ST77XX_BLACK, uint8_t pointSize = 3,
           uint8_t spaceAround = 4);

  void drawQR(qrcodegen::QrCode qrCode);
  void draw() override;
  void pixel(const int16_t x, const int16_t y, const uint16_t col, const int thePointSize);
};
