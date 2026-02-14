#include "TFT_Screen.h"
#include "StringWindow.h"

// SPI and pins
#define SPI_RX   16
#define SPI_TX   19
#define SPI_CLK  18
#define TFT_CS   17
#define TFT_DC   20
#define TFT_RST  21
#define TOUCH_CS 26
#define TOUCH_IRQ 255

TFT_Screen screen(
  SPI_RX, SPI_TX, SPI_CLK,
  TFT_CS, TFT_DC, TFT_RST,
  TOUCH_CS, TOUCH_IRQ
);

void setup() {
  screen.begin();
}

void loop() {
	bool demoMode = false;
	if (demoMode) {
	 screen.update();
	} else {
	 StringWindow win(tft,
	                 10, 10, 120, 50,
	                 2,
	                 ST77XX_WHITE,
	                 ST77XX_BLACK,
	                 "Hello worls",
	                 ST77XX_GREEN,
	                 2);
	win.draw();
	while(true);
	}

 
}
