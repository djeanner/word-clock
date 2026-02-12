/*
  // use pico board library by Earle F. Philhower, III 
*/
#include <Arduino.h> // add in header of classes
#include <time.h>
#include <TimeLib.h>
#include "hardware/timer.h"



#if defined(ARDUINO_RASPBERRY_PI_PICO_W)

// not using interupts for word clock is not displaying time
#define INTERRUPT_WORD_CLOCK 1
// both methods are implemented but have different requirements for compilation
// use of interrupts does not require a clock_control to be defined for DCF77Decoder  
#define CLOCK_CONTROL_INTERRUPT 1
#define SERIAL_DEBUG 0  // <<< set to 0 to disable ALL serial output NOTE: not functionning well with INTERRUPT_WORD_CLOCK

#else

// not using interupts for word clock is not displaying time
#define INTERRUPT_WORD_CLOCK 0
// both methods are implemented but have different requirements for compilation
// use of interrupts does not require a clock_control to be defined for DCF77Decoder  
#define CLOCK_CONTROL_INTERRUPT 0
#define SERIAL_DEBUG 1  // <<< set to 0 to disable ALL serial output NOTE: not functionning well with INTERRUPT_WORD_CLOCK

#endif




#define DEBUGINWORDCLOCK 1 // this is to disable a debugging feature in DCF77Decoder 

// board's led
#define LEDPIN LED_BUILTIN

// SERIAL DEBUG MACROS
#if SERIAL_DEBUG
#define DBG_BEGIN(x) Serial.begin(x)
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#else
#define DBG_BEGIN(x)
#define DBG_PRINT(x)
#define DBG_PRINTLN(x)
#endif

// GPIO for the analog input
#define RADIOINPUT 28
// minimal value of the level of the RADIOINPUT for goot 500 is good for 1.5 and 3.8 V receptors
#define MINVAL_ANTENNA 500

// delay_ms stops the clock of the CPU. not compatible with serial or other interrupt-based services
#if (INTERRUPT_WORD_CLOCK == 0) && (SERIAL_DEBUG == 0) && (CLOCK_CONTROL_INTERRUPT == 0)
  #define SLEEPORDELAYMS(ms) delay(ms)
#else
  #define SLEEPORDELAYMS(ms) sleep_ms(ms)
#endif


const bool debug8 = true;  // text
//txt:[+++++++----+-++++-+-+++---+---+----+---+----++--_---£-++++-+],Lms:276, 11:57 Mon Feb 2/2026 All
const bool debug9 = debug8;  // display long and short pulses on the fly
const bool debug5 = false;   // display long pause pulses on the fly
bool debug2 = !debug8;       // front display debugging steps
const bool debug3 = true;    // dump info about the validation process of times

#if CLOCK_CONTROL_INTERRUPT

// -------- interrupt function prototypes --------
bool clockControlAdjustCallback(repeating_timer_t *rt);
repeating_timer_t timerClockControlAdjust;

#endif // CLOCK_CONTROL_INTERRUPT


#ifndef CLOCK_CONTROL_INTERRUPT
#define CLOCK_CONTROL_INTERRUPT 0
#endif
class ClockControl {
private:
  bool qualityAccept;  // 0 take any time and does not running time correction. 1// used for correction
  size_t pointer;
  long int period;
  bool isPositiveCorrection;
  const size_t fsize;
  time_t *tArray;
  long long *rArray;
public:
  ClockControl(size_t size)
    : qualityAccept(false), pointer(0), period(0), isPositiveCorrection(true), fsize(size) {
    tArray = new time_t[size];
    rArray = new long long[size];
    ;
  }
  ~ClockControl() {
    delete[] tArray;
    delete[] rArray;
  }
  bool isReliable() {
    return qualityAccept;
  }
  time_t getLastTime() {
    if (pointer == 0) { return 0; }
    return tArray[pointer];
  }
  long long getLastCorrection() {
    if (pointer == 0) { return 0; }
    return rArray[pointer];
  }

  void interruptAdjust() {
    if (!qualityAccept) { return; }
    if (period == 0) { return; }
    if (isPositiveCorrection) {
      setTime(now() + 1);
    } else {
      setTime(now() - 1);
    }
    return;
  }

  // curSec is counter incrementing every second and call for each value (every second)
  // if not using interrupts, call this every second with a second counter it will adjust if necessary. Don't miss seconds
  void adjustTime(long int curSec) {
    if (!qualityAccept) { return; }
    if (period == 0) { return; }
    if ((curSec % period) == 0) {
      if (isPositiveCorrection) {
        setTime(now() + 1);
      } else {
        setTime(now() - 1);
      }
    }
    return;
  }

  // if not using interrupts, call this every minute with a minute counter it will adjust if necessary. Don't miss minutes
  void adjustTimeMinute(long int curMin) {
    if (!qualityAccept) { return; }
    if (period == 0) { return; }
    if ((curMin % (period / 60)) == 0) {
      if (isPositiveCorrection) {
        setTime(now() + 1);
      } else {
        setTime(now() - 1);
      }
    }
    return;
  }

  void storeDate(const time_t tNow, const long long errorSecondsPerDay) {
    pointer++;
    if (pointer == fsize) pointer = 1;  // so pointer == 0 is only when nothing...
    tArray[pointer] = tNow;
    rArray[pointer] = errorSecondsPerDay;
    isPositiveCorrection = errorSecondsPerDay > 0;
    if (errorSecondsPerDay == 0LL) {
      period = 0;
    } else {
      const long durationOneDay = 24 * 60 * 60;
      period = durationOneDay / labs((long int)errorSecondsPerDay);
    }
#if CLOCK_CONTROL_INTERRUPT
    cancel_repeating_timer(&timerClockControlAdjust);
    long long interval_us = -(long long)(period * 1000000LL);
    add_repeating_timer_us(
        interval_us,
        clockControlAdjustCallback,
        NULL,
        &timerClockControlAdjust
    );
#endif // CLOCK_CONTROL_INTERRUPT
  }

  String stringTime(time_t t = now()) {
    String retString = String(day(t)) + " " +
                        String(month(t)) + " " +
                        String(year(t)) + " " +
                        (hour(t) < 10 ? " " : "") + String(hour(t)) + ":" +
                        (minute(t) < 10 ? "0" : "") + String(minute(t)) + ":" +
                        (second(t) < 10 ? "0" : "") + String(second(t));
    return retString;
  }

  void storeTime(tmElements_t tm) {

    const time_t t = makeTime(tm);
    const time_t tNow = now();

    // analyse correction
    const long long diff = (long long)t - (long long)tNow;  // error in seconds / if positive internal time is too slow
    const long long unsignedDiff = llabs(diff);
  
    if (debug3) DBG_PRINTLN("");
    if (debug3) DBG_PRINT("Time submitted for validation ");
    if (debug3) DBG_PRINT("Earlyer data reliable : ");
    if (debug3) {
      if (qualityAccept) DBG_PRINTLN("Y");
      else DBG_PRINTLN("N");
    }
    if (debug3) DBG_PRINT("tested time      : ");
    if (debug3) DBG_PRINTLN(stringTime(t));
    if (debug3) DBG_PRINT("now :            : ");
    if (debug3) DBG_PRINTLN(stringTime(tNow));
  
    const bool critConsistent = (unsignedDiff < 100);
    if (!qualityAccept) {
      if (debug3) DBG_PRINT("Crit : |");
      if (debug3) DBG_PRINT(diff);
      if (debug3) DBG_PRINTLN("| < 100");
      setTime(t);
      // see if can consider switch to reliable
      if (critConsistent) {
        if (debug3) DBG_PRINTLN("Crit OK: Switch to reliable");
        qualityAccept = true;
      } else {
        if (debug3) DBG_PRINTLN("Crit failed: Time is not consistent (normal if first call). It is not considered as reliable");
        return;
      }
    }
    // dont use else because qualityAccept changes in if
    if (qualityAccept) {
      if (!critConsistent) {
        if (debug3) DBG_PRINTLN("Crit failed: Time is not consistent: ignored");
        return;
      }
      

      setTime(t);  // correct time
      if (pointer == 0) {
        if (debug3) DBG_PRINTLN("Crit consistent : first time quality, save time but not calculating error...");
        storeDate(tNow, 0LL);
        return;
      }
      if (debug3) DBG_PRINTLN("Crit consistent : consider calculate error for fine tuning in quality mode");
      const time_t lastTime = getLastTime();
      if (debug3) DBG_PRINT("Last stored time : ");
      if (debug3) DBG_PRINT(stringTime(lastTime));
      
      const long long secondsSinceLastTime = (long long)t - (long long)lastTime;
      const long long secondSinceLastAbs = llabs(secondsSinceLastTime);

      const long long minNumberSeconds = 60 * 60;  // 60 * 60 : 1 Hour
      const bool critLongEnough = secondSinceLastAbs > minNumberSeconds;
      if (!critLongEnough) {
        if (debug3) DBG_PRINTLN(" Time not long enough for good precision rejects time in quality mode");
        if (debug3) DBG_PRINT(secondSinceLastAbs);
        if (debug3) DBG_PRINT(" < ");
        if (debug3) DBG_PRINT(minNumberSeconds);
        if (debug3) DBG_PRINTLN(" x ");
        return;
      }
    
      if (debug3) DBG_PRINTLN("Crit consistent : calculate error for fine tuning in quality mode");

      if (debug3) DBG_PRINTLN("Calculate time correction...");
      const long long durationOneDay = 24 * 60 * 60;
      long long errorSecondsPerDay = (diff * durationOneDay) / ((long long)tNow - (long long)lastTime);
      if (debug3) DBG_PRINT(" errorSecondsPerDay = ");
      if (debug3) DBG_PRINT(diff);
      if (debug3) DBG_PRINT(" * ");
      if (debug3) DBG_PRINT(durationOneDay);
      if (debug3) DBG_PRINT(" / (");
      if (debug3) DBG_PRINT(tNow);
      if (debug3) DBG_PRINT(" - ");
      if (debug3) DBG_PRINT(lastTime);
      if (debug3) DBG_PRINT(") = ");
      if (debug3) DBG_PRINTLN(errorSecondsPerDay);

      if (debug3) DBG_PRINT(" errorSecondsPerDay = ");
      if (debug3) DBG_PRINT(diff);
      if (debug3) DBG_PRINT(" * ");
      if (debug3) DBG_PRINT(durationOneDay);
      if (debug3) DBG_PRINT(" / (");
      if (debug3) DBG_PRINT(tNow - lastTime);
      if (debug3) DBG_PRINT(") = ");
      if (debug3) DBG_PRINTLN(errorSecondsPerDay);

      errorSecondsPerDay += getLastCorrection();

      if (debug3) DBG_PRINT("Including previous correction : ");
      if (debug3) DBG_PRINT(errorSecondsPerDay);
      if (debug3) DBG_PRINTLN("");

      storeDate(tNow, errorSecondsPerDay);
      if (debug3) DBG_PRINT(" Period ");
      if (debug3) {
        if (isPositiveCorrection) DBG_PRINT("add ");
        else DBG_PRINT("subtract ");
      }
      if (debug3) DBG_PRINT("a second : every ");
      if (debug3) DBG_PRINT(period);
      if (debug3) DBG_PRINTLN(" s");
    }
    return;
  }
};


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

  WordClock() : totOutputPins(22) {
    values = new size_t[totOutputPins];
  }

  ~WordClock() {
    delete[] values;
  }

  void testLed() {
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
  void ocDriveLowAll_fullON() {  // called by interrupt
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

  void ocDriveLowAll_fullOFF() {  // called by interrupt
    for (size_t lo = 0; lo < totOutputPins; lo++) {
      pinMode(lo, INPUT);  // OFF State
    }
  }

  // chop led current obsolete since taken cared by interrupts
  void ocDriveWithoutInterrupt(size_t cycles = 10) {
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




  void setWordClock(const int curMin, const int curHourtrue, const bool writeItIs = true) {
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

  void debugSetHoursLeds(int unitDigit = 0) {
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

};

WordClock theWordClock;

ClockControl theClockControl(10);

#if CLOCK_CONTROL_INTERRUPT
bool clockControlAdjustCallback(repeating_timer_t *rt) {
  theClockControl.interruptAdjust();
  return true;  // keep repeating
}
#endif // CLOCK_CONTROL_INTERRUPT

// for DCF77Decoder header ... 
#ifndef CLOCK_CONTROL_INTERRUPT
#define CLOCK_CONTROL_INTERRUPT 1
#endif
/// @brief listen to analogic input of DCF77 antenna and saves time with the listen method
class DCF77Decoder {

public:

  
  enum class DCF77Bit : uint8_t {
    BIT_M_ = 0,   // 0
    BIT_R_ = 15,  // 0
    BIT_A1 = 16,
    BIT_Z1 = 17,
    BIT_Z2 = 18,
    BIT_A2 = 19,
    BIT_S_ = 20,  // 1

    MIN_1_ = 21,  // Minute code
    MIN_2_ = 22,  // Minute code
    MIN_4_ = 23,  // Minute code
    MIN_8_ = 24,  // Minute code
    MIN_10 = 25,  // Minute code
    MIN_20 = 26,  // Minute code
    MIN_40 = 27,  // Minute code
    P1 = 28,      // parity DCF77_getParity(DCF77Bit::MIN_1, DCF77_MIN_40)

    HOUR_1_ = 29,  // Hour code
    HOUR_2_ = 30,  // Hour code
    HOUR_4_ = 31,  // Hour code
    HOUR_8_ = 32,  // Hour code
    HOUR_10 = 33,  // Hour code
    HOUR_20 = 34,  // Hour code
    P2 = 35,       // parity DCF77_getParity(DCF77Bit::HOUR_1, DCF77_HOUR_20)

    DAYM_1_ = 36,  // Day month code
    DAYM_2_ = 37,  // Day month code
    DAYM_4_ = 38,  // Day month code
    DAYM_8_ = 39,  // Day month code
    DAYM_10 = 40,  // Day month code
    DAYM_20 = 41,  // Day month code

    DAYW_1_ = 42,  // Day week code
    DAYW_2_ = 43,  // Day week code
    DAYW_4_ = 44,  // Day week code

    MONTH_1_ = 45,  // Month code
    MONTH_2_ = 46,  // Month code
    MONTH_4_ = 47,  // Month code
    MONTH_8_ = 48,  // Month code
    MONTH_10 = 49,  // Month code

    YEAR_1_ = 50,  // Year code
    YEAR_2_ = 51,  // Year code
    YEAR_4_ = 52,  // Year code
    YEAR_8_ = 53,  // Year code
    YEAR_10 = 54,  // Year code
    YEAR_20 = 55,  // Year code
    YEAR_40 = 56,  // Year code
    YEAR_80 = 57,  // Year code

    P3 = 58  // parity DCF77_getParity(DCF77Bit::YEAR_1, DCF77_YEAR_80)
  };

  /// @brief save integers and take average value ingoring smallest and largest Used for start of pulses
  class Ring {
  public:
    // constructor
    Ring(size_t imaxSize)
      : pointer(0), maxSize(imaxSize), size(0), lastPointer(0) {
      array = new int[imaxSize];
    }

    // destructor
    ~Ring() {
      delete[] array;
    }

    void reset() {
      size = 0;
      pointer = 0;
      lastPointer = 0;
    }

    bool isFull() {
      return size == maxSize;
    }

    void push(int input) {
      array[pointer] = input;
      lastPointer = pointer;
      pointer++;
      if (pointer == maxSize) pointer = 0;
      if (size < maxSize) size++;
    }

    int getAverageCore() const {
      if (size == 0) return 0;

      // 1. Make a copy of the valid array elements
      int *copy = new int[size];
      for (size_t i = 0; i < size; i++) {
        copy[i] = getValue(i);  // oldest -> newest
      }
      // sort
      for (size_t i = 0; i < size - 1; i++) {
        for (size_t j = 0; j < size - i - 1; j++) {
          if (copy[j] > copy[j + 1]) {
            int temp = copy[j];
            copy[j] = copy[j + 1];
            copy[j + 1] = temp;
          }
        }
      }
      int start = size / 3;
      int end = size - size / 3;
      if (start >= end) start = 0;  // handle very small buffers

      long sum = 0;
      int count = 0;
      for (int i = start; i < end; i++) {
        sum += copy[i];
        count++;
      }
      delete[] copy;
      if (count == 0) return -1;
      return sum / count;
    }

    String dump() {
      String retString = "{";
      for (size_t i = 0; i < size; i++) {
        retString += String(array[i]);
        retString += " ";
      }
      retString += "Av.: ";
      retString += String(getAverageCore());  // 2 decimal places
      retString += "}";
      return retString;
    }

  private:
    size_t pointer;
    const size_t maxSize;
    size_t size;
    size_t lastPointer;
    int *array;

    int getValue(size_t item) const {
      if (item >= size) return 0;  // or error
      return array[item];
    }
  };
private:
  Ring storedDCF77upPulsesTimes;
  size_t fRadioInput;
  int valueIndexSec[60];
  size_t point_to_start;
  int previVal;
  unsigned long int startUp;
  unsigned long int startDown;
  unsigned long int lastStartUp;
  unsigned long int lastStartDown;
  size_t oldIndexSec;
  tmElements_t tm;
public:
  // contructor
  DCF77Decoder(size_t aInput) : storedDCF77upPulsesTimes(50), fRadioInput(aInput) {
    reset();
  }

  // destructor
  ~DCF77Decoder() {};

  void initListen() {
    previVal = 0;
    startUp = 0;
    startDown = 0;
    lastStartUp = 0;
    lastStartDown = 0;
    oldIndexSec = 0;
  }
  
  tmElements_t getTM() {return tm;}
  
  int listen() {
    const bool cursor_on = true;

    // meansure DCF77 level
    const int inVal = analogRead(fRadioInput);
    unsigned long long cMili = millis();
    const unsigned long int milisOnly = (cMili % 1000UL);
    //const size_t avDur = 150; // average duration pulse
    const size_t supForRounding = 500;  // average duration pulse
    const unsigned long int substract = static_cast<unsigned long int>(supForRounding + getAverageCore());
    const long seconds = (cMili - substract) / 1000L;
    const size_t indexSec = static_cast<size_t>(((cMili - substract) / 1000UL) % 60UL);

    if (oldIndexSec != indexSec) {
#if !CLOCK_CONTROL_INTERRUPT
      theClockControl.adjustTime(seconds);
#endif // !CLOCK_CONTROL_INTERRUPT
      oldIndexSec = indexSec;
#if DEBUGINWORDCLOCK
      if (debug2) theWordClock.debugSetHoursLeds(10);
#endif // DEBUGINWORDCLOCK
      // dump info
      const size_t numberPerLine = cursor_on ? 6 : 60;
      if ((indexSec % numberPerLine) == 0 && debug8) {
        if (cursor_on) { DBG_PRINTLN(); }
        DBG_PRINT("txt:[");
        String stringForLine = "";
        for (size_t i = 0; i < 60; i++) {
          if (i == indexSec && cursor_on) {
            DBG_PRINT("_");
          } else {
            if (raw(i) == 2) { DBG_PRINT(" "); }
            if (raw(i) == 3) {
              DBG_PRINT("£");
              const size_t store_val = getStart();
              setStart((i + 0) % 60);
              stringForLine += getString() + " ";
              setStart(store_val);
            }
            if (raw(i) == 0) { DBG_PRINT("-"); }
            if (raw(i) == 1) { DBG_PRINT("+"); }
          }
        }
        DBG_PRINT("],");
        if (isRingFull()) { DBG_PRINT("L"); }
        DBG_PRINT("ms:");
        DBG_PRINT(getAverageCore());  // miliStore
        DBG_PRINT(", ");
        DBG_PRINTLN(stringForLine);
      }
      // IMPORTNT : Here may want to reload each bit at each cycle be having next line uncommented
      //valueIndexSec[indexSec] = 2;
    }
    // DOWN -> UP
    if ((inVal > MINVAL_ANTENNA) && (previVal < MINVAL_ANTENNA)) {
      startUp = cMili;
      const int durCycleMili = cMili - lastStartUp;
      const int margin = 50;  //  margin ms
      // detect large gap for end of DCF77 minute cycle : 2000 ms (no pulse at second 59)
      const int mi = 2000 - margin;
      const int ma = 2000 + margin;

      if (durCycleMili > mi && durCycleMili < ma) {

        const size_t pointerInArrayMinus = (indexSec + 59) % 60;
        setRaw(pointerInArrayMinus, 3);
#if DEBUGINWORDCLOCK
        if (debug2) theWordClock.debugSetHoursLeds(4);
#endif // DEBUGINWORDCLOCK
        setStart(pointerInArrayMinus);

        if (areAllOK()) {
          tm.Year = CalendarYrToTm(2026);
          tm.Month = getMonth() - 1;  // jan = 0;
          tm.Day = getDayM();
          tm.Hour = getHour();
          tm.Minute = getMin();
          tm.Second = 0;
          return(1);
        }
      }
      // detect small gaps between seconds
      const int mi2 = 1000 - margin;
      const int ma2 = 1000 + margin;
      if (durCycleMili > mi2 && durCycleMili < ma2) {
        if (debug5) DBG_PRINT("^");
        if (debug5) DBG_PRINT(milisOnly);
        if (debug5) DBG_PRINT("<");
        pushDuration(milisOnly);
        //DBG_PRINT(storedDCF77upPulsesTimes.dump());
        if (debug5) DBG_PRINT(getAverageCore());
        if (debug5) DBG_PRINT(">");
      }
      previVal = inVal;
    }

    // UP -> DOWN
    if ((inVal < MINVAL_ANTENNA) && (previVal > MINVAL_ANTENNA)) {

      const int durCycleMili = cMili - lastStartDown;
      const int minDurationRealPulse = isRingFull() ? 500 : 10;  // not very logical
      if (durCycleMili > minDurationRealPulse) {
        lastStartDown = cMili;
        startDown = cMili;

        //miliStore = static_cast<int>(milisOnly);
        int durUpMili = cMili - startUp;
        const int delta = absCircularDelta(startUp % 1000, getAverageCore());
        //milisOnly // storedDCF77upPulsesTimes.getAverageCore()

        // margin of 50 ms for duration and position
        const int critDeltaPos = isRingFull() ? 50 : 1000;  // set tight only when ring is full
        const int critDeltaDur = 50;                                             // if change 50, rewrite code below
        const int mi = 100 - critDeltaDur;
        const int ma = 200 + critDeltaDur;

        if (durUpMili >= mi && durUpMili <= ma) {
          lastStartUp = startUp;

          if (delta < critDeltaPos) {

            // Store value
            int deltaDUR = 0;
            String pulse = "";
            if (durUpMili < 150) {  // short pulse
#if DEBUGINWORDCLOCK
              if (debug2) theWordClock.debugSetHoursLeds(8);
#endif // DEBUGINWORDCLOCK
              //valueIndexSec[indexSec] = 0; DEL
              setRaw(indexSec, 0);

              deltaDUR = durUpMili - 100;
              pulse = "S";
            } else {
#if DEBUGINWORDCLOCK
              if (debug2) theWordClock.debugSetHoursLeds(7);
#endif // DEBUGINWORDCLOCK
              setRaw(indexSec, 1);
              //valueIndexSec[indexSec] = 1; DEL
              deltaDUR = durUpMili - 200;
              pulse = "L";
            }
            if (cursor_on) {
              int signedDeltaStart = circularDelta(startUp % 1000, getAverageCore());
              if (debug9) DBG_PRINT(" ");
              if (debug9) DBG_PRINT(signedDeltaStart);
              if (signedDeltaStart > 0) {
                if (debug9) DBG_PRINT("+");
              }
              if (debug9) DBG_PRINT("(");
              if (debug9) DBG_PRINT(pulse);
              if (deltaDUR > 0) {
                if (debug9) DBG_PRINT("+");
              }
              if (debug9) DBG_PRINT(deltaDUR);
              if (debug9) DBG_PRINT(")");
            }
          } else {
            if (debug9) DBG_PRINT("X");  // wrong start
          }
        } else {
          if (debug9) DBG_PRINT("x");  // wrong duration
        }
      }
      previVal = inVal;
    }

    digitalWrite(LEDPIN, inVal < MINVAL_ANTENNA ? LOW : HIGH);
    return(0);
  }

  int circularDelta(int a, int b) {  // not absolute value
    // a and b in [0..1000[
    if (a > b) {
      if ((a - b) > 500)
        return -(1000 - a + b);
      else
        return a - b;
    } else {
      if ((b - a) > 500)
        return 1000 - b + a;
      else
        return -(b - a);
    }
  }

  int absCircularDelta(int a, int b) {
    // a and b in [0..1000[
    if (a > b) {
      if ((a - b) > 500)
        return 1000 - a + b;
      else
        return a - b;
    } else {
      if ((b - a) > 500)
        return 1000 - b + a;
      else
        return b - a;
    }
  }

  bool isRingFull() {
    return storedDCF77upPulsesTimes.isFull();
  }

  int getAverageCore() {
    return storedDCF77upPulsesTimes.getAverageCore();
  }

  void pushDuration(int input) {
    return storedDCF77upPulsesTimes.push(input);
  }

  void reset() {
    point_to_start = 0;
    for (size_t i = 0; i < 60; i++) {
      valueIndexSec[i] = 2;  // unknown
    }
    storedDCF77upPulsesTimes.reset();
  }

  size_t getStart() {
    return point_to_start;
  }

  // ---- bit access ----
  inline bool isBitUnknown(DCF77Bit bit) const {
    size_t b = static_cast<size_t>(bit);
    return valueIndexSec[(b + point_to_start + 1) % 60] == 2;
  }

  inline int getBit(DCF77Bit bit) const {
    size_t b = static_cast<size_t>(bit);
    return (valueIndexSec[(b + point_to_start + 1) % 60] == 0) ? 0 : 1;
  }

  // ---- parity ----
  int getParity(DCF77Bit first, DCF77Bit last) const {
    size_t f = static_cast<size_t>(first);
    size_t l = static_cast<size_t>(last);

    int sum = 0;
    for (size_t i = f; i <= l; i++) {
      DCF77Bit b = static_cast<DCF77Bit>(i);
      if (isBitUnknown(b)) return 99;
      if (getBit(b)) sum++;
    }
    return sum % 2;
  }

  // ---- decoded values ----
  int getHour() const {
    return getBit(DCF77Bit::HOUR_1_) * 1
           + getBit(DCF77Bit::HOUR_2_) * 2
           + getBit(DCF77Bit::HOUR_4_) * 4
           + getBit(DCF77Bit::HOUR_8_) * 8
           + getBit(DCF77Bit::HOUR_10) * 10
           + getBit(DCF77Bit::HOUR_20) * 20;
  }
  int getMin() const {
    return getBit(DCF77Bit::MIN_1_) * 1
           + getBit(DCF77Bit::MIN_2_) * 2
           + getBit(DCF77Bit::MIN_4_) * 4
           + getBit(DCF77Bit::MIN_8_) * 8
           + getBit(DCF77Bit::MIN_10) * 10
           + getBit(DCF77Bit::MIN_20) * 20
           + getBit(DCF77Bit::MIN_40) * 40;
  }

  int getYear() const {
    return getBit(DCF77Bit::YEAR_1_) * 1
           + getBit(DCF77Bit::YEAR_2_) * 2
           + getBit(DCF77Bit::YEAR_4_) * 4
           + getBit(DCF77Bit::YEAR_8_) * 8
           + getBit(DCF77Bit::YEAR_10) * 10
           + getBit(DCF77Bit::YEAR_20) * 20
           + getBit(DCF77Bit::YEAR_40) * 40
           + getBit(DCF77Bit::YEAR_80) * 80;
  }

  String getMonthString() {
    const int month = getMonth();
    if (month == 1) return String("Jan");
    if (month == 2) return String("Feb");
    if (month == 3) return String("Mar");
    if (month == 4) return String("Apr");
    if (month == 5) return String("May");
    if (month == 6) return String("Jun");
    if (month == 7) return String("Jul");
    if (month == 8) return String("Aug");
    if (month == 9) return String("Sep");
    if (month == 10) return String("Oct");
    if (month == 11) return String("Nov");
    if (month == 12) return String("Dec");
    return String("XXX");
  }

  String getDayWString() {
    const int day = getDayW();
    if (day == 0) return String("Sun");
    if (day == 1) return String("Mon");
    if (day == 2) return String("Tue");
    if (day == 3) return String("Wed");
    if (day == 4) return String("Thu");
    if (day == 5) return String("Fri");
    if (day == 6) return String("Sat");
    if (day == 7) return String("Sun");
    if (day == 8) return String("DX8");
    return "DXX";
  }

  int getDayM() const {
    return getBit(DCF77Bit::DAYM_1_) * 1
           + getBit(DCF77Bit::DAYM_2_) * 2
           + getBit(DCF77Bit::DAYM_4_) * 4
           + getBit(DCF77Bit::DAYM_8_) * 8
           + getBit(DCF77Bit::DAYM_10) * 10
           + getBit(DCF77Bit::DAYM_20) * 20;
  }

  int getDayW() const {
    return getBit(DCF77Bit::DAYW_1_) * 1
           + getBit(DCF77Bit::DAYW_2_) * 2
           + getBit(DCF77Bit::DAYW_4_) * 4;
  }

  int getMonth() const {
    return getBit(DCF77Bit::MONTH_1_) * 1
           + getBit(DCF77Bit::MONTH_2_) * 2
           + getBit(DCF77Bit::MONTH_4_) * 4
           + getBit(DCF77Bit::MONTH_8_) * 8
           + getBit(DCF77Bit::MONTH_10) * 10;
  }

  // ---- validity ----
  bool areAllOK() const {
    if (getHour() > 24) return false;
    if (getMin() > 60) return false;
    if (getDayM() > 31) return false;
    if (getDayW() > 7) return false;
    if (getMonth() > 12) return false;

    if (getBit(DCF77Bit::BIT_M_)) return false;
    if (getBit(DCF77Bit::BIT_R_)) return false;
    if (!getBit(DCF77Bit::BIT_S_)) return false;

    if (getBit(DCF77Bit::P1) != getParity(DCF77Bit::MIN_1_, DCF77Bit::MIN_40)) return false;
    if (getBit(DCF77Bit::P2) != getParity(DCF77Bit::HOUR_1_, DCF77Bit::HOUR_20)) return false;
    if (getBit(DCF77Bit::P3) != getParity(DCF77Bit::MONTH_1_, DCF77Bit::YEAR_80)) return false;

    return true;
  }

  String getString() {
    String retString = "";
    retString += "" + String(getHour()) + ":";
    retString += "" + String(getMin()) + " ";
    retString += "" + String(getDayWString()) + " ";
    retString += String(getMonthString()) + " ";
    retString += String(getDayM()) + "/";
    retString += "20" + String(getYear()) + " ";
    if (areAllOK()) {
      retString += "AllT";

    } else {
      retString += getBit(DCF77Bit::BIT_M_) ? "T" : "F";
      retString += getBit(DCF77Bit::BIT_R_) ? "T" : "F";
      retString += getBit(DCF77Bit::BIT_S_) ? "T" : "F";
      retString += getParity(DCF77Bit::MIN_1_, DCF77Bit::MIN_40) ? "T" : "F";
      retString += getParity(DCF77Bit::HOUR_1_, DCF77Bit::HOUR_20) ? "T" : "F";
      retString += getParity(DCF77Bit::MONTH_1_, DCF77Bit::YEAR_80) ? "T" : "F";
    }
    return retString;
  }

  String getStringDEL() const {
    String s;
    s.reserve(60);
    for (int i = 0; i < 60; ++i) {
      switch (raw(i)) {
        case 0: s += '0'; break;
        case 1: s += '1'; break;
        case 2: s += ' '; break;  // unknown
        default: s += '?'; break;
      }
    }
    return s;
  }

  void setRaw(size_t index, int input) {
    valueIndexSec[index % 60] = input;
  }
  // ---- raw access for pulse logic ----
  int &raw(size_t index) {
    return valueIndexSec[index % 60];
  }
  // read-only access (const object)
  int raw(size_t index) const {
    return valueIndexSec[index];
  }
  void setStart(size_t idx) {
    point_to_start = idx % 60;
  }


};




#if INTERRUPT_WORD_CLOCK
// Interrups to control led brightness without flickering
repeating_timer_t timer10ms;
alarm_id_t alarmID;

// -------- interrupt function prototypes --------
bool timer10msCallback(repeating_timer_t *rt);  // 100 Hz to avoid flickering
int64_t alarmCallback(alarm_id_t id, void *user_data);

// -------- WordClock interrupt callback --------
// Called every 10 ms
bool timer10msCallback(repeating_timer_t *rt) {
  theWordClock.ocDriveLowAll_fullON();
  // prepare follop up off...
  const uint64_t delayMicroSeconds = 1500;
  alarmID = add_alarm_in_us(
    delayMicroSeconds,
    alarmCallback,
    NULL,
    true  // fire even if IRQs were briefly disabled
  );
  return true;  // keep repeating
}

// Called a few ms after timer10msCallback interrupt
int64_t alarmCallback(alarm_id_t id, void *user_data) {
  theWordClock.ocDriveLowAll_fullOFF();
  return 0;  // one-shot alarm
}
#endif // INTERRUPT_WORD_CLOCK

DCF77Decoder dcf77(RADIOINPUT);

void setup() {
#if INTERRUPT_WORD_CLOCK
  // interupt every 10000 us
  add_repeating_timer_us(
    -10000,  // negative = exact interval, no drift -10000: 100 Hz to avoid visible flickering
    timer10msCallback,
    NULL,
    &timer10ms);
#endif // INTERRUPT_WORD_CLOCK
  
if (debug2) theWordClock.debugSetHoursLeds(1);

  DBG_BEGIN(115200);
  delay(2000);

  analogReadResolution(12);

  DBG_PRINT("DCF77 pin : ");
  DBG_PRINTLN(RADIOINPUT);

  // Start with everything released
  theWordClock.ocDriveLowAll_fullOFF();

  // setting up Led
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  delay(100);
  digitalWrite(LEDPIN, LOW);
  delay(200);
  digitalWrite(LEDPIN, HIGH);
  delay(300);
  digitalWrite(LEDPIN, LOW);

  DBG_PRINTLN("End setup");
  if (debug2) theWordClock.debugSetHoursLeds(2);
}

void loop() {
  if (debug2) theWordClock.debugSetHoursLeds(3);

  // Test each output led
  const bool testEachLedFirst = false;
  if (testEachLedFirst) { 
    theWordClock.testLed();
  }

  for (long superLoop = 0; superLoop < 100000000; superLoop++) {
    // Main listener : returns when have recieved valid time/date. May last minutes.
    DBG_PRINTLN("Start listening to dcf77 signal ...");

    dcf77.reset();
    dcf77.initListen();
    int lastMinL1 = minute(now());
    for (long fastLoop = 0; fastLoop < 1000000000; fastLoop ++) {
      // fast loop 
      const int isTimeValid = dcf77.listen();
      if (isTimeValid == 1) {
        theClockControl.storeTime(dcf77.getTM());
        debug2 = false;  // stop
        break;
      }
      time_t t = now();
      const int curMin = minute(t);
      if (lastMinL1 != curMin) {
        lastMinL1 = curMin;
        theWordClock.setWordClock(curMin, hour(t), theClockControl.isReliable());
      }
    }
  
    // will sleep/delay for about a minute when nothing happens and not listening to dcf77
    long long last_min = 0;
    unsigned long int numberMinStaysInLoop = theClockControl.isReliable() ? 20 : 5; // 180 : 10;
    DBG_PRINT("Stop listening to dcf77 for ");
    DBG_PRINT(numberMinStaysInLoop);
    DBG_PRINTLN(" min.");
    time_t t = now();
    theWordClock.setWordClock(minute(t), hour(t), theClockControl.isReliable());
    int lastMin = minute(t);
    for (unsigned long long loo = 0UL; loo < 1000000000; loo++) {
      t = now();
      int curMin = minute(t);
      // when minute changes
      if (curMin != lastMin) {
        lastMin = curMin;
        // perturb time every minute.. to see how manages
        //
        //
        setTime(now() + 1);
        //

        // refine time if cristal not precise enough
#if !CLOCK_CONTROL_INTERRUPT
        theClockControl.adjustTimeMinute(millis() / 60000LL);
#endif // !CLOCK_CONTROL_INTERRUPT
        digitalWrite(LEDPIN, ((curMin % 2) == 0) ? LOW : HIGH);
        theWordClock.setWordClock(minute(t), hour(t), theClockControl.isReliable());
        numberMinStaysInLoop -= 1;
        if (numberMinStaysInLoop <= 0) break;
        SLEEPORDELAYMS(55000);  // waits for 55 sec
      }
    }
  }
}