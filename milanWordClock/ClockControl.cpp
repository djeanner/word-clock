#include "ClockControl.h"

  ClockControl::ClockControl(size_t size, bool in3, bool useSerial)
    : fCorrectionIsSet(false), qualityAccept(false), pointer(0), period(0), isPositiveCorrection(true), fsize(size), 
  fdebug(in3), fUseSerial(useSerial), fStringCallback({}) {
    tArray = new time_t[size];
    rArray = new long long[size];
    fSourceTime = "";
  }
  ClockControl::~ClockControl() {
    delete[] tArray;
    delete[] rArray;
  }

  String ClockControl::getStringDateHourMinReliable() {
    time_t t = now();
    return String(day(t)) + "/" +
                            String(month(t)) + "/" +
                            String(year(t)) + " " +
                            (hour(t) < 10 ? " " : "") + String(hour(t)) + ":" +
                            (minute(t) < 10 ? "0" : "") + String(minute(t)) + " " +
                            // ":" + (second(t) < 10 ? "0" : "") + String(second(t)) +
                            (isReliable() ? "+" : "-");
  }

  void ClockControl::setStringCallback(std::function<void(String)> aStringCallback) {
	    fStringCallback = aStringCallback;
  }
  void ClockControl::thisPrintLN(String aString) {
	    thisPrint(aString + "\n");
      fLineHeader = true;
  }
  void ClockControl::thisPrint(String aString) {
  const String stringWithHeader = fLineHeader ? "ClockControl >> " + aString : aString;
  fLineHeader = false;
	if (fStringCallback) {
		fStringCallback(stringWithHeader);
	} 
	if (fUseSerial) {
		if (fdebug) Serial.print(stringWithHeader);
	}
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

  bool ClockControl::storeTimeString(const String value,
                                     const bool forceReliable,
                                     const String source) {
    if (value.length() >= 16) {
      tmElements_t tm;
      tm.Year = CalendarYrToTm(value.substring(0, 4).toInt());
      tm.Month = value.substring(5, 7).toInt();
      tm.Day = value.substring(8, 10).toInt();
      tm.Hour = value.substring(11, 13).toInt();
      tm.Minute = value.substring(14, 16).toInt();
      if (value.length() >= 19) {
        tm.Second = value.substring(17, 19).toInt();
      } else {
        tm.Second = 0;
      }
      storeTime(tm, forceReliable, source);
      return true;
    }
    return false;
  }

  void ClockControl::storeTime(tmElements_t tm, bool forceReliable, String source) {
    const time_t t = makeTime(tm);
    if (forceReliable) { // bypass all checks 
      setTime(t);
      fSourceTime = source;
      thisPrint(" Force reliable to true and time to ");
      thisPrintLN(stringTime(t));
      qualityAccept = true;
      fSourceTime = source;
      return;
    }

    const time_t tNow = now();

    // analyse correction
    const long long diff = (long long)t - (long long)tNow;  // error in seconds / if positive internal time is too slow
    const long long unsignedDiff = llabs(diff);
  
    thisPrintLN("");
    thisPrint("Time submitted for validation source: ");
    thisPrintLN(source);
    thisPrint("Earlyer data reliable : ");
      if (qualityAccept) {thisPrintLN("Y");}
      else {thisPrintLN("N");}
    thisPrint("tested time      : ");
    thisPrintLN(stringTime(t));
    thisPrint("now :            : ");
    thisPrintLN(stringTime(tNow));
    const long long critReliableDelta = 100LL;
    const bool critConsistent = (unsignedDiff < critReliableDelta);
    if (!qualityAccept) {
      thisPrint("Crit : |");
      thisPrint(String((long)diff));
      thisPrintLN("| < 100");
      setTime(t);
      fSourceTime = source;
      // see if can consider switch to reliable
      if (critConsistent) {
        thisPrintLN("Crit OK: Switch to reliable");
        qualityAccept = true;
      } else {
        thisPrint("X: ");
        thisPrint(stringTime(t));
        thisPrint(" is first or not consistent. ");
        thisPrint(String((long)unsignedDiff));
        thisPrint( " s > ");
        thisPrint(String((long)critReliableDelta));
        thisPrintLN("s. It is not considered as reliable");
        return;
      }
    }
    // dont use else because qualityAccept changes in if
    if (qualityAccept) {
      if (!critConsistent) {
        thisPrint("X: ");
        thisPrint(stringTime(t));
        thisPrintLN(" is not consistent: ignored");
        return;
      }
      

      if (pointer == 0) {
        thisPrint("OK : ");
        thisPrint(stringTime(t));
        thisPrintLN(": first time quality, save time but not calculating error...");
        setTime(t);  // correct time
        fSourceTime = source;
        storeDate(tNow, 0LL);
        return;
      }
      thisPrint("OK : ");
      thisPrint(stringTime(t));
      thisPrintLN(" : consider calculate error for fine tuning in quality mode");
      const time_t lastTime = getLastTime();
      thisPrint("Last stored time : ");
      thisPrintLN(stringTime(lastTime));
      
      const long long secondsSinceLastTime = (long long)t - (long long)lastTime;
      const long long secondSinceLastAbs = llabs(secondsSinceLastTime);

      const long long minNumberSeconds = 60 * 60;  // 60 * 60 : 1 Hour
      const bool critLongEnough = secondSinceLastAbs > minNumberSeconds;
      if (!critLongEnough) {
        thisPrintLN(" Time since last stored time not long enough for good precision rejects time in quality mode");
        thisPrint(String((long)secondSinceLastAbs));
        thisPrint(" < ");
        thisPrint(String((long)minNumberSeconds));
        thisPrintLN(" s.");
        return;
      }
      setTime(t);  // correct time
      fSourceTime = source;
      thisPrint("OK : ");
      thisPrint(stringTime(t));
      thisPrintLN(" : calculate error for fine tuning in quality mode");

      thisPrintLN("Calculate time correction...");
      const long long durationOneDay = 24 * 60 * 60;
      long long errorSecondsPerDay = (diff * durationOneDay) / ((long long)tNow - (long long)lastTime);
      thisPrint(" errorSecondsPerDay = ");
      thisPrint(String((long)diff));
      thisPrint(" * ");
      thisPrint(String((long)durationOneDay));
      thisPrint(" / (");
      thisPrint(String((long)tNow));
      thisPrint(" - ");
      thisPrint(String((long)lastTime));
      thisPrint(") = ");
      thisPrintLN(String((int)errorSecondsPerDay));

      thisPrint(" errorSecondsPerDay = ");
      thisPrint(String((long)diff));
      thisPrint(" * ");
      thisPrint(String((long)durationOneDay));
      thisPrint(" / (");
      thisPrint(String((long)(tNow - lastTime)));
      thisPrint(") = ");
      thisPrintLN(String((int)errorSecondsPerDay));

      const auto correctionLast = getLastCorrection();
      thisPrint("Including previous correction : ");
      thisPrint(String((long)errorSecondsPerDay));
      thisPrint(" + ");
      thisPrint(String((long)correctionLast));

      errorSecondsPerDay += correctionLast;

      thisPrint(" = ");
      thisPrint(String((long)errorSecondsPerDay));
      thisPrintLN("");
      thisPrint(stringTime(t));
      thisPrint(" ");
      //
      storeDate(tNow, errorSecondsPerDay);
      // 
      thisPrint(" Period ");
      if (isPositiveCorrection) {thisPrint("add ");}
      else {thisPrint("subtract ");}
      thisPrint("a second every ");
      thisPrint(String((long)period));
      thisPrintLN(" s");
    }
    return;
  }

  bool ClockControl::isCorrectionSet() { return fCorrectionIsSet; }
