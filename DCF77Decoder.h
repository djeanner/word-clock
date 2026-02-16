#pragma once
#include <Arduino.h> // add in header of classes
#include <time.h>
#include <TimeLib.h>
#include "hardware/timer.h"

#if SERIAL_DEBUG
#define DBG_BEGIN(x) Serial.begin(x)
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#else
#define DBG_BEGIN(x)
#define DBG_PRINT(x)
#define DBG_PRINTLN(x)
#endif

class ClockControl {
private:
  bool qualityAccept;  // 0 take any time and does not running time correction. 1// used for correction
  size_t pointer;
  long int period;
  bool isPositiveCorrection;
  const size_t fsize;
  bool debug2;
   bool debug3;
   bool debug5;
   bool debug8;
   bool debug9;
  time_t *tArray;
  long long *rArray;
public:
  ClockControl(size_t size, bool in2, bool in3, bool in5, bool in8, bool in9)
    : qualityAccept(false), pointer(0), period(0), isPositiveCorrection(true), fsize(size), debug2(in2),
  debug3(in3),
  debug5(in5),
  debug8(in8),
  debug9(in9) {
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
    if (period == 0L) { return; }
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
    if (period == 0L) { return; }
    if ((curSec % period) == 0L) {
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
    if ((curMin % (period / 60L)) == 0L) {
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
    isPositiveCorrection = errorSecondsPerDay > 0LL;
    if (errorSecondsPerDay == 0LL) {
      period = 0L;
    } else {
      const long durationOneDay = 24 * 60 * 60;
      period = durationOneDay / labs((long int)errorSecondsPerDay);
    }
#if CLOCK_CONTROL_INTERRUPT
    if (thereIsAtimerClockControlAdjust_running) {
      cancel_repeating_timer(&timerClockControlAdjust);
      thereIsAtimerClockControlAdjust_running = false;
    }
    if (period != 0L) {
      long long interval_us = -((long long)period * 1000000LL);
      add_repeating_timer_us(
        interval_us,
        clockControlAdjustCallback,
        NULL,
        &timerClockControlAdjust
      );
      thereIsAtimerClockControlAdjust_running = true;
    }
    
#endif // CLOCK_CONTROL_INTERRUPT
  }

  String stringTime(time_t t = now()) {
    String retString = (day(t) < 10 ? " " : "") + String(day(t)) + " " +
                        (month(t) < 10 ? " " : "") + String(month(t)) + " " +
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
      

      if (pointer == 0) {
        if (debug3) DBG_PRINTLN("Crit consistent : first time quality, save time but not calculating error...");
        setTime(t);  // correct time
        storeDate(tNow, 0LL);
        return;
      }
      if (debug3) DBG_PRINTLN("Crit consistent : consider calculate error for fine tuning in quality mode");
      const time_t lastTime = getLastTime();
      if (debug3) DBG_PRINT("Last stored time : ");
      if (debug3) DBG_PRINTLN(stringTime(lastTime));
      
      const long long secondsSinceLastTime = (long long)t - (long long)lastTime;
      const long long secondSinceLastAbs = llabs(secondsSinceLastTime);

      const long long minNumberSeconds = 60 * 60;  // 60 * 60 : 1 Hour
      const bool critLongEnough = secondSinceLastAbs > minNumberSeconds;
      if (!critLongEnough) {
        if (debug3) DBG_PRINTLN(" Time since last stored time not long enough for good precision rejects time in quality mode");
        if (debug3) DBG_PRINT(secondSinceLastAbs);
        if (debug3) DBG_PRINT(" < ");
        if (debug3) DBG_PRINT(minNumberSeconds);
        if (debug3) DBG_PRINTLN(" s.");
        return;
      }
      setTime(t);  // correct time
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


class DCF77Decoder {
public:

  enum class DCF77Bit : uint8_t {
    BIT_M_ = 0,   // 0
    BIT_R_ = 15,  // 0 1: abnormal transmitter
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

    P3 = 58  // parity DCF77_getParity(DCF77Bit::DAYM_1_, DCF77_YEAR_80)
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
  long int fMinValAntenna;
   bool debug2;
   bool debug3;
   bool debug5;
   bool debug8;
   bool debug9;
  pin_size_t fLedPin;
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
  DCF77Decoder(size_t aInput, long int aLongInt, bool in2, bool in3, bool in5, bool in8, bool in9, pin_size_t aPin = 127);

  // destructor
  ~DCF77Decoder() ;

  void initListen();
  
  tmElements_t getTM();
  
  int listen(ClockControl & theClockControl) ;

  int circularDelta(int a, int b) ;

  int absCircularDelta(int a, int b) ;

  bool isRingFull() ;

  int getAverageCore() ;

  void pushDuration(int input) ;

  void reset() ;
  size_t getStart() ;

  inline bool isBitUnknown(DCF77Bit bit) const ;

  inline int getBit(DCF77Bit bit) const;
  int getParity(DCF77Bit first, DCF77Bit last) const;
  int getHour() const ;
  int getMin() const ;

  int getYear() const ;

  String getMonthString() ;

  String getDayWString() ;

  int getDayM() const ;

  int getDayW() const ;

  int getMonth() const ;

  bool areAllOK() const;

  String getString();

  String getStringDEL() const ;

  void setRaw(size_t index, int input) ;
  int &raw(size_t index) ;
  int raw(size_t index) const;
  void setStart(size_t idx) ;
  int getDigit(DCF77Bit zzz) const ;
private:
  int getDigitPrivate(DCF77Bit zzz) const ;

  int getDigitP(DCF77Bit first, DCF77Bit last) const ;

};

