#include "ClockControl.h"

#if SERIAL_DEBUG
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#else
#define DBG_PRINT(x)
#define DBG_PRINTLN(x)
#endif

  ClockControl::ClockControl(size_t size, bool in3)
    : qualityAccept(false), pointer(0), period(0), isPositiveCorrection(true), fsize(size), 
  debug3(in3) {
    tArray = new time_t[size];
    rArray = new long long[size];
    ;
  }
  ClockControl::~ClockControl() {
    delete[] tArray;
    delete[] rArray;
  }
  bool ClockControl::isReliable() {
    return qualityAccept;
  }
  time_t ClockControl::getLastTime() {
    if (pointer == 0) { return 0; }
    return tArray[pointer];
  }
  long long ClockControl::getLastCorrection() {
    if (pointer == 0) { return 0; }
    return rArray[pointer];
  }

  void ClockControl::interruptAdjust() {
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
  void ClockControl::adjustTime(long int curSec) {
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
  void ClockControl::adjustTimeMinute(long int curMin) {
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

  void ClockControl::storeDate(const time_t tNow, const long long errorSecondsPerDay) {
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

  String ClockControl::stringTime(time_t t) {
    String retString = (day(t) < 10 ? " " : "") + String(day(t)) + " " +
                        (month(t) < 10 ? " " : "") + String(month(t)) + " " +
                        String(year(t)) + " " +
                        (hour(t) < 10 ? " " : "") + String(hour(t)) + ":" +
                        (minute(t) < 10 ? "0" : "") + String(minute(t)) + ":" +
                        (second(t) < 10 ? "0" : "") + String(second(t));
    return retString;
  }

  void ClockControl::storeTime(tmElements_t tm) {

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
