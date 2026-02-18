#include "TFT_Screen.h"

TFT_Screen::TFT_Screen(uint8_t widthIn, uint8_t heigthIN, 
                       uint8_t spiRX, uint8_t spiTX, uint8_t spiCLK,
                       uint8_t tftCS, uint8_t tftDC, uint8_t tftRST,
                       uint8_t touchCS, uint8_t touchIRQ)
  : width(widthIn), height(heigthIN),
    tft(tftCS, tftDC, tftRST),
    ts(touchCS, touchIRQ),
    spi_rx(spiRX),
    spi_tx(spiTX),
    spi_clk(spiCLK)
{}

Adafruit_ST7735* TFT_Screen::getTFT() {
  return &tft;
}

XPT2046_Touchscreen* TFT_Screen::getTS() {
  return &ts;
}void TFT_Screen::begin() {
  Serial.begin(115200);

  SPI.setRX(spi_rx);
  SPI.setTX(spi_tx);
  SPI.setSCK(spi_clk);
  SPI.begin();
  // SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10,10);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Hello Pico");
  delay(100);

  ts.begin();
  ts.setRotation(1);
}

// ---------- Touch drawing ----------
void TFT_Screen::handleTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();

  // calibration touch
  // map(p.x, 200, 3800, 0, tft.width());
  // map(p.y, 200, 3800, 0, tft.height());
  // 150–3900 depending on screen
  int x = map(p.x, 200, 3800, 0, tft.width());
  int y = map(p.y, 3800, 200, 0, tft.height());

  tft.fillCircle(x, y, 2, ST77XX_RED);
}

// ---------- Main loop ----------
void TFT_Screen::update() {
  handleTouch();

  // switch demo every 6 seconds
  if (millis() - lastSwitch > 6000) {
    lastSwitch = millis();

    demoMode++;
    demoMode %= 4;
    switch (demoMode) {
      case 0: demoText(); break;
      case 1: demoShapes(); break;
      case 2: demoBars(); break;
      case 3: tft.fillScreen(ST77XX_BLACK); break;
    }
  }

  if (demoMode == 3)
    demoAnim();
}

// ---------- Demo 1: Text ----------
void TFT_Screen::demoText() {

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.println("RP2040 TFT");

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.println("Joy-IT 1.8\"...");
  tft.setTextColor(ST77XX_RED);
  tft.println("Touch enabled !");
  tft.setTextColor(ST77XX_BLUE);
  tft.print(tft.width());
  tft.print("x");
  tft.print(tft.height());
  tft.println(" pt.");
}

// ---------- Demo 2: Shapes ----------
void TFT_Screen::demoShapes() {
  tft.fillScreen(ST77XX_BLACK);

  for (int i = 0; i < 15; i += 1) {
    tft.drawRect(10 + 2 * i, 10 + i * 2, 100 - i * 4, 60 - i * 4, ST77XX_MAGENTA);
  }

  tft.fillCircle(100, 100, 20, ST77XX_YELLOW);
  tft.drawPixel(100, 100, ST77XX_CYAN);
  
  tft.drawLine(0, 0, 160, 128, ST77XX_CYAN);
}

// ---------- Demo 3: Color bars ----------
void TFT_Screen::demoBars() {
  for (int x = 0; x < tft.width(); x++) {
    uint16_t color = tft.color565(x * 2, 255 - x, x);
    tft.drawFastVLine(x, 0, tft.height(), color);
  }
}

// ---------- Demo 4: Moving pixel ----------
void TFT_Screen::demoAnim() {
  static int x = 0;
  static int dx = 1;

  tft.fillRect(x, 60, 10, 10, ST77XX_BLACK);

  x += dx;
  if (x < 0 || x > 150) dx = -dx;

  tft.fillRect(x, 60, 10, 10, ST77XX_BLUE);
}
