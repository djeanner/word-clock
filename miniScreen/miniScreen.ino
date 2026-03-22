#include "QRWindow.h"
#include "StringWindow.h"
#include "TFT_Screen.h"

// SPI and pins
#define SPI_RX 16
#define SPI_TX 19
#define SPI_CLK 18
#define TFT_CS 17
#define TFT_DC 20
#define TFT_RST 21
#define TOUCH_CS 26
#define TOUCH_IRQ 255

TFT_Screen screen(160, 128, SPI_RX, SPI_TX, SPI_CLK, TFT_CS, TFT_DC, TFT_RST,
                  TOUCH_CS, TOUCH_IRQ);

void setup() { screen.begin(false, 3); }

void loop() {
  bool demoMode = false;
  if (demoMode) {
    screen.update();
  } else {
    StringWindow win(
        &screen, 2, 2, 156, 15, 1, ST77XX_WHITE, ST77XX_BLACK,
        "          Scan the codes and support the action...          ",
        ST77XX_GREEN, 1);
    win.draw();

    QRWindow winQR(&screen, 25, 17, 110, 110);
    winQR.draw();

    const qrcodegen::QrCode::Ecc errCorLvl =
        qrcodegen::QrCode::Ecc::LOW; // Error correction level

    const char *text1 = "Oui !";
    const qrcodegen::QrCode qr1 =
        qrcodegen::QrCode::encodeText(text1, errCorLvl);
    const char *text2 = "Non !";
    const qrcodegen::QrCode qr2 =
        qrcodegen::QrCode::encodeText(text2, errCorLvl);
#include "hiddenFile.h"

    const qrcodegen::QrCode qrP =
        qrcodegen::QrCode::encodeText(textP, errCorLvl);
    const int maxL = 500;
    long long pt = 0;
    for (int i = 0; i < 100000; i++) {
      winQR.drawQR(qr1);
      for (int g = 0; g < maxL; g++) {
        win.drawShift(pt++);
      }
      winQR.drawQR(qr2);
      for (int g = 0; g < maxL; g++) {
        win.drawShift(pt++);
      }
      winQR.draw(); // erase content
      winQR.drawQR(qrP);
      for (int g = 0; g < maxL; g++) {
        win.drawShift(pt++);
      }
      winQR.draw(); // erase content
    }
  }
}
