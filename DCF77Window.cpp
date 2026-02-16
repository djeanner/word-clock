#include "DCF77Window.h"

DCF77Window::DCF77Window(TFT_Screen* aScreen,
						int x, int y, int w, int h,
						int frame,
						uint16_t frameColor,
						uint16_t bgColor,
						uint16_t textColor,
             			uint8_t textSize,
               			uint8_t dotSize
               )
  : TFT_Window(aScreen, x, y, w, h, frame, frameColor, bgColor),
    ftextColor(textColor),
    ftextSize(textSize),
    fdotSize(dotSize)
{}
void DCF77Window::pixel(const int16_t x, const int16_t y, const uint16_t col) {
	if (fdotSize == 2) {
		ftft->drawPixel(fx + fframeWidth + 2*x  ,  fy + fframeWidth +2*y  , col);
		ftft->drawPixel(fx + fframeWidth + 2*x+1,  fy + fframeWidth +2*y  , col);
		ftft->drawPixel(fx + fframeWidth + 2*x  ,  fy + fframeWidth +2*y+1, col);
		ftft->drawPixel(fx + fframeWidth + 2*x+1,  fy + fframeWidth +2*y+1, col);
		return;
	}
	ftft->drawPixel(fx + fframeWidth + x, fy + fframeWidth + y, col);
}
void DCF77Window::miniNum(const int16_t valueDigit, const int16_t x, const int16_t y, const uint16_t col) {
	// 3x5 digit
	switch(valueDigit) {

case 0:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	pixel(x+0,y+1,col);                     pixel(x+2,y+1,col);
	pixel(x+0,y+2,col);                     pixel(x+2,y+2,col);
	pixel(x+0,y+3,col);                     pixel(x+2,y+3,col);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,col);
	break;

case 1:
	pixel(x+2,y+0,col);
	pixel(x+2,y+1,col);
	pixel(x+2,y+2,col);
	pixel(x+2,y+3,col);
	pixel(x+2,y+4,col);
	break;

case 2:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	                     pixel(x+2,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,col);
	pixel(x+0,y+3,col);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,col);
	break;

case 3:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	                     pixel(x+2,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,col);
	                     pixel(x+2,y+3,col);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,col);
	break;

case 4:
	pixel(x+0,y+0,col);                     pixel(x+2,y+0,col);
	pixel(x+0,y+1,col);                     pixel(x+2,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,col);
	                     pixel(x+2,y+3,col);
	                     pixel(x+2,y+4,col);
	break;

case 5:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	pixel(x+0,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,col);
	                     pixel(x+2,y+3,col);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,col);
	break;

case 6:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	pixel(x+0,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,col);
	pixel(x+0,y+3,col);                     pixel(x+2,y+3,col);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,col);
	break;

case 7:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	                     pixel(x+2,y+1,col);
	                     pixel(x+2,y+2,col);
	                     pixel(x+2,y+3,col);
	                     pixel(x+2,y+4,col);
	break;

case 8:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	pixel(x+0,y+1,col);                     pixel(x+2,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,col);
	pixel(x+0,y+3,col);                     pixel(x+2,y+3,col);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,col);
	break;

case 9:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	pixel(x+0,y+1,col);                     pixel(x+2,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,col);
	                     pixel(x+2,y+3,col);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,col);
	break;
}

}

void DCF77Window::draw() {
  TFT_Window::draw();

  int ix = 0;
  int iy = 0;
  int iw = fw - 2 * fframeWidth;
  int ih = fh - 2 * fframeWidth;

  ftft->setTextColor(ftextColor);
  ftft->setTextSize(ftextSize);

   int ty = (8 * ftextSize / 2);

  ftft->setCursor(ix, iy + ty);
  ftft->print("Hello");
  const int shifty = 0;
  for(int i = 0; i < 59; i++) {
	  pixel(ix + i, iy + 3 + shifty, ST77XX_CYAN);
		if ((i % 10) == 5) {
				  pixel(ix + i, iy + 2 + shifty, ST77XX_CYAN);
		}
		if ((i % 10) == 5) {
				  pixel(ix + i, iy + 1 + shifty, ST77XX_CYAN);
				  pixel(ix + i, iy + 2 + shifty, ST77XX_CYAN);
		}
  }
    for(int i = 0; i <= 9; i++) {
						  miniNum(i, ix + i * 6, iy + 5 + shifty, ST77XX_YELLOW);

	}

}
