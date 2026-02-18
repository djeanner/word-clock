#pragma once
#include "TFT_Window.h"

class StringWindow : public TFT_Window {
private:
  String ftext;
  uint16_t ftextColor;
  uint8_t ftextSize;
  int ftx, fty;
  String fBufferedInput;

public:
  StringWindow(TFT_Screen* aScreen,
               int x, int y, int w, int h,
               int frame,
               uint16_t frameColor,
               uint16_t bgColor,
               String text,
               uint16_t textColor,
               uint8_t textSize = 1,
               int tx = 0,
               int ty = -1);

  void draw() override;
  void drawShift(int shift);
  void changeText(String aString);

};
