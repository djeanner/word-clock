#pragma once
#include <Arduino.h> // add in header of classes
//#include <time.h>
//#include <TimeLib.h>
//#include "hardware/timer.h"

class WordClock {
private:
const size_t totOutputPins;
size_t *values;
public:

  enum class WordGP : uint8_t {
    H1_ = 0,
    H2_ = 1,
    H3_ = 2,
    H4_ = 3,
    H5_ = 9,
    H6_ = 5,
    H7_ = 6,
    H8_ = 7,
    H9_ = 8,
    H10 = 4,
    H11 = 10,
    H12 = 11,
    // PM        = 12 
    // AM        = 13
    HTO = 14,
    HPAST = 15,
    HALF = 16,
    M10 = 17,
    M5 = 18,
    MITIS = 12,  // NEW alwas on
    MOCLOCK = 19,
    MQUARTER = 20,
    MTWENTY = 13,     // was 22
    EXTRA = 21,       // new
  };

  WordClock() ;
  ~WordClock() ;
  void testLed() ;
  void ocDriveLowAll_fullON() ;
  void ocDriveLowAll_fullOFF() ;
  void ocDriveWithoutInterrupt(size_t cycles = 10) ;
  void setWordClock(const int curMin, const int curHourtrue, const bool writeItIs = true);
  void debugSetHoursLeds(int unitDigit = 0) ;

};