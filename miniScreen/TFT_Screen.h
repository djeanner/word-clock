#ifndef TFT_SCREEN_H
#define TFT_SCREEN_H

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

class TFT_Screen {

private:
  Adafruit_ST7735 tft;
  XPT2046_Touchscreen ts;
  uint8_t width, height;
  uint8_t spi_rx, spi_tx, spi_clk;
  uint8_t demoMode = 0;
  unsigned long lastSwitch = 0;

  // ---------- Demo methods ----------
  void demoText();
  void demoShapes();
  void demoBars();
  void demoAnim();

public:
  TFT_Screen(uint8_t width, uint8_t heigth,
             uint8_t spiRX, uint8_t spiTX, uint8_t spiCLK,
             uint8_t tftCS, uint8_t tftDC, uint8_t tftRST,
             uint8_t touchCS, uint8_t touchIRQ);

  Adafruit_ST7735* getTFT();
  XPT2046_Touchscreen* getTS();
  uint8_t getWidth() {return width;}
  uint8_t getHeight() {return height;}
  void begin(bool showsHello = false, uint8_t rotation = 3);

  // ---------- Touch drawing ----------
  void handleTouch();

  // ---------- Main loop ----------
  void update();
};

#endif // TFT_SCREEN_H
