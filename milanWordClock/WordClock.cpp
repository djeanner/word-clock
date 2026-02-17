#include "WordClock.h"

  WordClock::WordClock() : totOutputPins(22) {
    values = new size_t[totOutputPins];
  }

  WordClock::~WordClock() {
    delete[] values;
  }

  void WordClock::testLed() {
    for (size_t p = 0; p < totOutputPins; p++) {
      for (size_t lo = 0; lo < totOutputPins; lo++) {
        const size_t pin = lo;
        if (lo == p) {
          values[pin] = 1;  //[pin] = 1;
        } else {
          values[pin] = 0;  //[pin] = 0;
        }
      }
      delay(1000);
    }
  }

  // Drive all pins LOW (open-collector active) output used as mosfet. Use INPUT mode as low states output
  void WordClock::ocDriveLowAll_fullON() {  // called by interrupt
    for (size_t lo = 0; lo < totOutputPins; lo++) {
      const size_t pin = lo;
      if (values[pin] > 0) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
      } else {
        pinMode(pin, INPUT);
        //digitalWrite(pin, LOW); // dummy
      }
    }
  }

  void WordClock::ocDriveLowAll_fullOFF() {  // called by interrupt
    for (size_t lo = 0; lo < totOutputPins; lo++) {
      pinMode(lo, INPUT);  // OFF State
    }
  }

  // chop led current obsolete since taken cared by interrupts
  void WordClock::ocDriveWithoutInterrupt(size_t cycles) {
    for (size_t t = 0; t < cycles; t++) {
      for (size_t lo = 0; lo < totOutputPins; lo++) {
        pinMode(lo, INPUT);  // OFF State
      }
      delay(10);
      for (size_t lo = 0; lo < totOutputPins; lo++) {
        if (values[lo] > 0) {
          pinMode(lo, OUTPUT);
          digitalWrite(lo, LOW);
        } else {
          pinMode(lo, INPUT);
          //digitalWrite(pin, LOW); // dummy
        }
      }
      delay(1);
    }
    for (size_t lo = 0; lo < totOutputPins; lo++) {
      pinMode(lo, INPUT);  // OFF State
    }
  }

  void WordClock::setWordClock(const int curMin, const int curHourtrue, const bool writeItIs) {
    values[(size_t)WordGP::MITIS] = writeItIs ? 1 : 0;
    int curHour = curHourtrue;  // displayed
    if (curMin > 34) curHour++;

    if (curMin < 5) values[(size_t)WordGP::MOCLOCK] = 1;
    else values[(size_t)WordGP::MOCLOCK] = 0;
    values[(size_t)WordGP::M5] = 0;
    values[(size_t)WordGP::M10] = 0;
    values[(size_t)WordGP::MQUARTER] = 0;
    values[(size_t)WordGP::MTWENTY] = 0;
    // past / to
    if ((curMin >= 5) && (curMin < 35)) values[(size_t)WordGP::HPAST] = 1;
    else values[(size_t)WordGP::HPAST] = 0;
    if ((curMin >= 35) && (curMin < 60)) values[(size_t)WordGP::HTO] = 1;
    else values[(size_t)WordGP::HTO] = 0;
    // min
    if ((curMin >= 0) && (curMin < 5)) values[(size_t)WordGP::MOCLOCK] = 1;
    else values[(size_t)WordGP::MOCLOCK] = 0;
    if ((curMin >= 5) && (curMin < 10)) values[(size_t)WordGP::M5] = 1;
    if ((curMin >= 10) && (curMin < 15)) values[(size_t)WordGP::M10] = 1;
    if ((curMin >= 15) && (curMin < 20)) values[(size_t)WordGP::MQUARTER] = 1;
    if ((curMin >= 20) && (curMin < 25)) values[(size_t)WordGP::MTWENTY] = 1;
    if ((curMin >= 25) && (curMin < 30)) {
      values[(size_t)WordGP::MTWENTY] = 1;
      values[(size_t)WordGP::M5] = 1;
    }
    if ((curMin >= 30) && (curMin < 35)) values[(size_t)WordGP::HALF] = 1;
    else values[(size_t)WordGP::HALF] = 0;
    if ((curMin >= 35) && (curMin < 40)) {
      values[(size_t)WordGP::MTWENTY] = 1;
      values[(size_t)WordGP::M5] = 1;
    }
    if ((curMin >= 40) && (curMin < 45)) values[(size_t)WordGP::MTWENTY] = 1;
    if ((curMin >= 45) && (curMin < 50)) values[(size_t)WordGP::MQUARTER] = 1;
    if ((curMin >= 50) && (curMin < 55)) values[(size_t)WordGP::M10] = 1;
    if ((curMin >= 55) && (curMin < 60)) values[(size_t)WordGP::M5] = 1;
    // hours
    if ((curHour == 0) || (curHour == 12) || (curHour == 24)) values[(size_t)WordGP::H12] = 1;
    else values[(size_t)WordGP::H12] = 0;
    if ((curHour == 1) || (curHour == 13)) values[(size_t)WordGP::H1_] = 1;
    else values[(size_t)WordGP::H1_] = 0;
    if ((curHour == 2) || (curHour == 14)) values[(size_t)WordGP::H2_] = 1;
    else values[(size_t)WordGP::H2_] = 0;
    if ((curHour == 3) || (curHour == 15)) values[(size_t)WordGP::H3_] = 1;
    else values[(size_t)WordGP::H3_] = 0;
    if ((curHour == 4) || (curHour == 16)) values[(size_t)WordGP::H4_] = 1;
    else values[(size_t)WordGP::H4_] = 0;
    if ((curHour == 5) || (curHour == 17)) values[(size_t)WordGP::H5_] = 1;
    else values[(size_t)WordGP::H5_] = 0;
    if ((curHour == 6) || (curHour == 18)) values[(size_t)WordGP::H6_] = 1;
    else values[(size_t)WordGP::H6_] = 0;
    if ((curHour == 7) || (curHour == 19)) values[(size_t)WordGP::H7_] = 1;
    else values[(size_t)WordGP::H7_] = 0;
    if ((curHour == 8) || (curHour == 20)) values[(size_t)WordGP::H8_] = 1;
    else values[(size_t)WordGP::H8_] = 0;
    if ((curHour == 9) || (curHour == 21)) values[(size_t)WordGP::H9_] = 1;
    else values[(size_t)WordGP::H9_] = 0;
    if ((curHour == 10) || (curHour == 22)) values[(size_t)WordGP::H10] = 1;
    else values[(size_t)WordGP::H10] = 0;
    if ((curHour == 11) || (curHour == 23)) values[(size_t)WordGP::H11] = 1;
    else values[(size_t)WordGP::H11] = 0;
    // AM
    // if ((curHourtrue < 12) || (curHourtrue == 24)) {values[AM] = 1; values[PM] = 0;} else {values[AM] = 0;values[PM] = 1;}
  }

  void WordClock::debugSetHoursLeds(int unitDigit) {
    for (size_t i = 0; i < totOutputPins; i++) {
      values[i] = 0;
    }
    if (unitDigit == 1) { values[(size_t)WordGP::H1_] = 1; }
    if (unitDigit == 2) { values[(size_t)WordGP::H2_] = 1; }
    if (unitDigit == 3) { values[(size_t)WordGP::H3_] = 1; }
    if (unitDigit == 4) { values[(size_t)WordGP::H4_] = 1; }
    if (unitDigit == 5) { values[(size_t)WordGP::H5_] = 1; }
    if (unitDigit == 6) { values[(size_t)WordGP::H6_] = 1; }
    if (unitDigit == 7) { values[(size_t)WordGP::H7_] = 1; }
    if (unitDigit == 8) { values[(size_t)WordGP::H8_] = 1; }
    if (unitDigit == 9) { values[(size_t)WordGP::H9_] = 1; }
    if (unitDigit == 10) { values[(size_t)WordGP::H10] = 1; }
    if (unitDigit == 11) { values[(size_t)WordGP::H11] = 1; }
    if (unitDigit == 12) { values[(size_t)WordGP::H12] = 1; }
    return;
  }
