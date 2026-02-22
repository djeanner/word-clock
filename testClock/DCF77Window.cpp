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
    fdotSize(dotSize),
	fYmap(0),
	fLastPos(0) {}

void DCF77Window::pixel(const int16_t x, const int16_t y, const uint16_t col) {
	if (fdotSize == 2) {
		ftft->drawPixel(getXpos() + 2*x  ,  getYpos() +2*y  , col);
		ftft->drawPixel(getXpos() + 2*x+1,  getYpos() +2*y  , col);
		ftft->drawPixel(getXpos() + 2*x  ,  getYpos() +2*y+1, col);
		ftft->drawPixel(getXpos() + 2*x+1,  getYpos() +2*y+1, col);
		return;
	}
	ftft->drawPixel(getXpos() + x, getYpos() + y, col);
}
void DCF77Window::miniNum(const int16_t valueDigit, const int16_t x, const int16_t y, const uint16_t col) {
	// 3x5 digit
	const uint16_t b00 = ST77XX_BLACK;
	const uint16_t cil = col;
	switch(valueDigit) {

case 0:
	pixel(x+0,y+0,cil); pixel(x+1,y+0,col); pixel(x+2,y+0,cil);
	pixel(x+0,y+1,col); pixel(x+1,y+1,b00); pixel(x+2,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,b00); pixel(x+2,y+2,col);
	pixel(x+0,y+3,col); pixel(x+1,y+3,b00); pixel(x+2,y+3,col);
	pixel(x+0,y+4,cil); pixel(x+1,y+4,col); pixel(x+2,y+4,cil);
	break;

case 1:
	pixel(x+0,y+0,b00); pixel(x+1,y+0,col); pixel(x+2,y+0,b00);
	pixel(x+0,y+1,col); pixel(x+1,y+1,col); pixel(x+2,y+1,b00);
	pixel(x+0,y+2,b00); pixel(x+1,y+2,col); pixel(x+2,y+2,b00);
	pixel(x+0,y+3,b00); pixel(x+1,y+3,col); pixel(x+2,y+3,b00);
	pixel(x+0,y+4,b00); pixel(x+1,y+4,col); pixel(x+2,y+4,b00); 
	break;

case 2:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,cil);
	pixel(x+0,y+1,b00); pixel(x+1,y+1,b00); pixel(x+2,y+1,col);
	pixel(x+0,y+2,cil); pixel(x+1,y+2,col); pixel(x+2,y+2,cil);
	pixel(x+0,y+3,col); pixel(x+1,y+3,b00); pixel(x+2,y+3,b00);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,col);
	break;

case 3:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,cil);
	pixel(x+0,y+1,b00); pixel(x+1,y+1,b00); pixel(x+2,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,cil);
	pixel(x+0,y+3,b00); pixel(x+1,y+3,b00); pixel(x+2,y+3,col);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,cil);
	break;

case 4:
	pixel(x+0,y+0,col); pixel(x+1,y+0,b00); pixel(x+2,y+0,col);
	pixel(x+0,y+1,col); pixel(x+1,y+1,b00); pixel(x+2,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,col);
	pixel(x+0,y+3,b00); pixel(x+1,y+3,b00); pixel(x+2,y+3,col);
	pixel(x+0,y+4,b00); pixel(x+1,y+4,b00); pixel(x+2,y+4,col);
	break;

case 5:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	pixel(x+0,y+1,col); pixel(x+1,y+1,b00); pixel(x+2,y+1,b00); 
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,cil);
	pixel(x+0,y+3,b00); pixel(x+1,y+3,b00); pixel(x+2,y+3,col);
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,cil);
	break;

case 6:
	pixel(x+0,y+0,cil); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	pixel(x+0,y+1,col); pixel(x+1,y+1,b00); pixel(x+2,y+1,b00); 
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,cil);
	pixel(x+0,y+3,col); pixel(x+1,y+3,b00); pixel(x+2,y+3,col);
	pixel(x+0,y+4,cil); pixel(x+1,y+4,col); pixel(x+2,y+4,cil);
	break;

case 7:
	pixel(x+0,y+0,col); pixel(x+1,y+0,col); pixel(x+2,y+0,col);
	pixel(x+0,y+1,b00); pixel(x+1,y+1,b00); pixel(x+2,y+1,col);
	pixel(x+0,y+2,b00); pixel(x+1,y+2,col); pixel(x+2,y+2,b00);
	pixel(x+0,y+3,b00); pixel(x+1,y+3,col); pixel(x+2,y+3,b00);
	pixel(x+0,y+4,b00); pixel(x+1,y+4,col); pixel(x+2,y+4,b00);
	break;

case 8:
	pixel(x+0,y+0,cil); pixel(x+1,y+0,col); pixel(x+2,y+0,cil);
	pixel(x+0,y+1,col); pixel(x+1,y+1,b00); pixel(x+2,y+1,col);
	pixel(x+0,y+2,cil); pixel(x+1,y+2,col); pixel(x+2,y+2,cil);
	pixel(x+0,y+3,col); pixel(x+1,y+3,b00); pixel(x+2,y+3,col);
	pixel(x+0,y+4,cil); pixel(x+1,y+4,col); pixel(x+2,y+4,cil);
	break;

case 9:
	pixel(x+0,y+0,cil); pixel(x+1,y+0,col); pixel(x+2,y+0,cil);
	pixel(x+0,y+1,col); pixel(x+1,y+1,b00); pixel(x+2,y+1,col);
	pixel(x+0,y+2,col); pixel(x+1,y+2,col); pixel(x+2,y+2,col);
	pixel(x+0,y+3,b00); pixel(x+1,y+3,b00); pixel(x+2,y+3,col); 
	pixel(x+0,y+4,col); pixel(x+1,y+4,col); pixel(x+2,y+4,cil);
	break;
}

}

void DCF77Window::draw() {
  TFT_Window::draw();

  ftft->setTextColor(ftextColor);// if second color, it is the background
  ftft->setTextSize(ftextSize);

  int ty = (8 * ftextSize / 2);

  const int shifty = 9;
  const auto colorScale = ST77XX_CYAN;
  for(int i = 0; i < 60; i++) {
	const int xPosition = 60 - i;
  	pixel(xPosition, 1 + shifty, colorScale);
	if ((i % 5) == 0) {
		pixel(xPosition, 2 + shifty, colorScale);
	}
	if ((i % 10) == 0) {
		pixel(xPosition, 3 + shifty, colorScale);
	}
  }
}

void DCF77Window::updateBit(const int index, const int bitData, const int miniString, const int indexCrude, const int unused) {
	const int posYminiNum = 1;
	const int posSingleLineBit = 7;
	const int posSingleLineBit_below = posSingleLineBit + 1;
	int posMultiLineBit = 14;

	// calc postion in map (lower part of the window)
	if (indexCrude < fLastPos) {
		int spaceBottom = (((int)getHeight() - posMultiLineBit) / (int)fdotSize) - 7;
		if (spaceBottom < 1) {spaceBottom = 1;}
		fLastPos = indexCrude;
		fYmap++;
		fYmap = fYmap % spaceBottom;
	}
	fLastPos = indexCrude;

	posMultiLineBit += fYmap;
	const uint16_t colorPixel = ST77XX_RED;
	uint16_t coloPixel = ST77XX_BLACK;
	int xPositionMini = 60 - index;
	// avoid overlap of Digits
	if (index == (24)) {xPositionMini +=2;} // Minute Einer
	if (index == (27)) {xPositionMini +=1;} // Minute Zehner
	if (index == (32)) {xPositionMini +=2;} // Stunden Einer
	if (index == (34)) {xPositionMini +=0;} // Stunden Zehner
	if (index == (39)) {xPositionMini +=1;} // Monat Tag Einer
	if (index == (41)) {xPositionMini -=1;} // Monat Tag Zehner
	if (index == (48) ) {xPositionMini +=2;} // Monat Einer
	if (index == (49) ) {xPositionMini -=1;} // Monat Zehner
	if (index == (53) ) {xPositionMini -=1;} // Jahr Einer
	if (index == (57) ) {xPositionMini -=1;} // Jahr Zehner
	if (bitData == 0) {coloPixel = ST77XX_BLUE;}
	if (bitData == 1) {coloPixel = ST77XX_YELLOW;}
	if (bitData == 2) {coloPixel = ST77XX_MAGENTA;}
	if (bitData == 3) {	coloPixel = ST77XX_CYAN;}
	if (miniString == 11) {coloPixel = ST77XX_GREEN;}
	if (miniString == 12) {coloPixel = ST77XX_RED;}
	pixel(60 - index, posSingleLineBit, coloPixel);
	pixel(60 - indexCrude, posMultiLineBit, coloPixel);
	if (miniString == 11 || miniString == 12) {
		pixel(60 - index, posSingleLineBit_below, coloPixel);
	} else {
		pixel(60 - index, posSingleLineBit_below, ST77XX_BLACK);
	}
	// mini digit 0-9
	if (miniString >= 0 && miniString <= 9 ) {
		miniNum(miniString, xPositionMini, posYminiNum, ST77XX_WHITE);
	}
	


}

