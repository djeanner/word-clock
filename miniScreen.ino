#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <XPT2046_Touchscreen.h>

/*
=== Joy-IT RB-TFT1.8-T → Raspberry Pi Pico wiring ===

SCREEN PIN   | SIGNAL / FUNCTION        | PICO GPIO PIN
-------------|--------------------------|--------------
VCC          | 3.3V power               | 3V3 OUT   36
GND          | Ground                   | GND       38

SCL          | SPI Clock (SCK)          | GP18      24
SDA          | SPI MOSI (display data)  | GP19      25

DC           | TFT Data/Command         | GP20      26
RES          | TFT Reset                | GP21      27
CS           | TFT Chip Select          | GP17      22

IRQ-T        | Touch Interrupt(optional)| GP22      29 (optional)
CS-T         | Touch Chip Select        | GP26      31 
MISO-T       | SPI MISO (touch data)    | GP16      21

Notes:
- SPI bus is shared between TFT + touch controller.
- IRQ-T not required unless using interrupts.
- All signals are 3.3V only (no 5V).
*/

// --- SPI GP ---
#define SPI_RX   16
#define SPI_TX   19
#define SPI_CLK  18

// --- TFT GP ---
#define TFT_CS   17
#define TFT_DC   20
#define TFT_RST  21

// --- Touch GP ---
#define TOUCH_CS 26
#define TOUCH_IRQ 255   // optional


class TFT_Screen {

private:
  Adafruit_ST7735 tft;
  XPT2046_Touchscreen ts;

  uint8_t spi_rx, spi_tx, spi_clk;

  uint8_t demoMode = 0;
  unsigned long lastSwitch = 0;
  
public:
  TFT_Screen(uint8_t spiRX, uint8_t spiTX, uint8_t spiCLK,
             uint8_t tftCS, uint8_t tftDC, uint8_t tftRST,
             uint8_t touchCS, uint8_t touchIRQ)
    : tft(tftCS, tftDC, tftRST),
      ts(touchCS, touchIRQ),
      spi_rx(spiRX),
      spi_tx(spiTX),
      spi_clk(spiCLK)
  {}

  void begin() {
    Serial.begin(115200);

    SPI.setRX(spi_rx);
    SPI.setTX(spi_tx);
    SPI.setSCK(spi_clk);
    SPI.begin();

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10,10);
    tft.setTextColor(ST77XX_WHITE);
    tft.println("Hello Pico");

    ts.begin();
    ts.setRotation(1);
  }

  // ---------- Touch drawing ----------
  void handleTouch() {
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
  void update() {

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

private:
  // ---------- Demo 1: Text ----------
  void demoText() {

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
    tft.println("128x160 pt.");
  }

  // ---------- Demo 2: Shapes ----------
  void demoShapes() {
    tft.fillScreen(ST77XX_BLACK);

    for (int i = 0; i < 15; i += 1) {
      tft.drawRect(10 + 2 * i, 10 + i * 2, 100 - i * 4, 60 - i * 4, ST77XX_MAGENTA);
    }

    tft.fillCircle(100, 100, 20, ST77XX_YELLOW);
    tft.drawLine(0, 0, 160, 128, ST77XX_CYAN);
  }

  // ---------- Demo 3: Color bars ----------
  void demoBars() {
    for (int x = 0; x < tft.width(); x++) {
      uint16_t color = tft.color565(x * 2, 255 - x, x);
      tft.drawFastVLine(x, 0, tft.height(), color);
    }
  }

  // ---------- Demo 4: Moving pixel ----------
  void demoAnim() {
    static int x = 0;
    static int dx = 1;

    tft.fillRect(x, 60, 10, 10, ST77XX_BLACK);

    x += dx;
    if (x < 0 || x > 150) dx = -dx;

    tft.fillRect(x, 60, 10, 10, ST77XX_BLUE);
  }

};

TFT_Screen screen(
  SPI_RX, SPI_TX, SPI_CLK,
  TFT_CS, TFT_DC, TFT_RST,
  TOUCH_CS, TOUCH_IRQ
);

void setup() {
  screen.begin();
}

void loop() {
  screen.update();
}
