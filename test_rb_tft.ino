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

TFT_Screen screen(160, 128,
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
	  StringWindow win(&screen,
	                 10, 10, 120, 16,
	                 1,
	                 ST77XX_WHITE,
	                 ST77XX_BLACK,
	                 "Hello world!",
	                 ST77XX_GREEN,
	                 1);
		StringWindow win2(&screen,
	                 10, 30, 120, 28,
	                 1,
	                 ST77XX_WHITE,
	                 ST77XX_BLACK,
	                 "   Hello world  and all and all jkkkjljlh 123 123 124gggggg     ",
	                 ST77XX_RED,
	                 2);
									 StringWindow win3(&screen,
	                 10, 80, 126, 40,
	                 1,
	                 ST77XX_WHITE,
	                 ST77XX_BLACK,
	                 "   Hello world  and all and all jkkkjljlh 123 123 124gggggg   ",
	                 ST77XX_BLUE,
	                 3);
	win.draw();
	win2.draw();
	for (int i = 0; i < 1000000; i++) {
		//delay(1);
		win.drawShift(i);
		win2.drawShift(i);
		win3.drawShift(i);
	}
	}

 
}
