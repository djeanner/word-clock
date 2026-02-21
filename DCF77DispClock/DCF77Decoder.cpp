#include "DCF77Decoder.h"

  DCF77Decoder::DCF77Decoder(size_t aInput, long int aLongInt,  bool in2, bool in5, bool in8, bool in9, pin_size_t aPin, bool useSerial, bool aSaveArchiveForServer) : 
  storedDCF77upPulsesTimes(50), 
  fRadioInput(aInput), 
  fMinValAntenna(aLongInt), 
  debug2(in2),
  debug5(in5),
  debug8(in8),
  debug9(in9), 
  fLedPin(aPin),
  fUseSerial(useSerial), 
  fStringCallback({}),
  fBitDataCallback({}),
  fSaveArchiveForServer(aSaveArchiveForServer),
  fPreviousIndexForServer(0),
  fMinPointerArchive(0),
  fStringForServer("")
  {
    reset();
  }

  // destructor
  DCF77Decoder::~DCF77Decoder() {}

  void DCF77Decoder::setBitDataCallback(std::function<void(int, int, int, int, int)> aBitDataCallback) {
	    fBitDataCallback = aBitDataCallback;
  }

  void DCF77Decoder::setStringCallback(std::function<void(String)> aStringCallback) {
	    fStringCallback = aStringCallback;
  }

  void DCF77Decoder::thisPrintLN(String aString) {
	    thisPrint(aString + "\n");
  }
 
  void DCF77Decoder::thisPrint(String aString) {
	if (fStringCallback) {
		fStringCallback(aString);
	} 
	if (fUseSerial) {
		Serial.print(aString);
	}
  }

  void DCF77Decoder::initListen() {
    previVal = 0;
    startUp = 0;
    startDown = 0;
    lastStartUp = 0;
    lastStartDown = 0;
    oldIndexSec = 0;
  }
  
  tmElements_t DCF77Decoder::getTM() {return tm;}

  int DCF77Decoder::listen(ClockControl & theClockControl) {
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
      oldIndexSec = indexSec;
#if !CLOCK_CONTROL_INTERRUPT
      theClockControl.adjustTime(seconds);
#endif // !CLOCK_CONTROL_INTERRUPT

    // if (debug2) theWordClock.debugSetHoursLeds(10);
      // dump info
      const size_t numberPerLine = cursor_on ? 6 : 60;
      if ((indexSec % numberPerLine) == 0 && debug8) {
        if (cursor_on) { thisPrintLN(); }
        thisPrint("txt:[");
        // too slow //String stringForLine = "";
        for (size_t i = 0; i < 60; i++) {
          if (i == indexSec && cursor_on) {
            thisPrint("_");
          } else {
            if (raw(i) == 2) { thisPrint(" "); }
            if (raw(i) == 3) {
              thisPrint("*");
              // too slow // const size_t store_val = getStart();
              // too slow //setStart((i + 0) % 60);
              // too slow //stringForLine += getString() + " ";
              // too slow //setStart(store_val);
            }
            if (raw(i) == 0) { thisPrint("-"); }
            if (raw(i) == 1) { thisPrint("+"); }
          }
        }
        thisPrint("],");
        if (isRingFull()) { thisPrint("L"); }
        thisPrint("ms:");
        thisPrint(String((long)getAverageCore()));  // miliStore
        thisPrint(", ");
        // too slow //thisPrintLN(stringForLine);
        thisPrintLN();
      }
      // IMPORTNT : Here may want to reload each bit at each cycle be having next line uncommented
      //valueIndexSec[indexSec] = 2;
    }
    // DOWN -> UP
    if ((inVal > fMinValAntenna) && (previVal < fMinValAntenna)) {
      startUp = cMili;
      const int durCycleMili = cMili - lastStartUp;
      const int margin = 50;  //  margin ms
      // detect large gap for end of DCF77 minute cycle : 2000 ms (no pulse at second 59)
      const int mi = 2000 - margin;
      const int ma = 2000 + margin;

      if (durCycleMili > mi && durCycleMili < ma) {

        const size_t pointerInArrayMinus = (indexSec + 59) % 60;
        setStart(pointerInArrayMinus);
        setRaw(pointerInArrayMinus, 3);
#if DEBUGINWORDCLOCK
        // if (debug2) theWordClock.debugSetHoursLeds(4);
#endif // DEBUGINWORDCLOCK

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
        if (debug5) thisPrint("^");
        if (debug5) thisPrint(String((long)milisOnly));
        if (debug5) thisPrint("<");
        pushDuration(milisOnly);
        //thisPrint(storedDCF77upPulsesTimes.dump());
        if (debug5) thisPrint(String((long)getAverageCore()));
        if (debug5) thisPrint(">");
      }
      previVal = inVal;
    }

    // UP -> DOWN
    if ((inVal < fMinValAntenna) && (previVal > fMinValAntenna)) {

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
              // if (debug2) theWordClock.debugSetHoursLeds(8);
#endif // DEBUGINWORDCLOCK
              //valueIndexSec[indexSec] = 0; DEL
              setRaw(indexSec, 0);

              deltaDUR = durUpMili - 100;
              pulse = "S";
            } else {
#if DEBUGINWORDCLOCK
              // if (debug2) theWordClock.debugSetHoursLeds(7);
#endif // DEBUGINWORDCLOCK
              setRaw(indexSec, 1);
              //valueIndexSec[indexSec] = 1; DEL
              deltaDUR = durUpMili - 200;
              pulse = "L";
            }
            if (cursor_on) {
              int signedDeltaStart = circularDelta(startUp % 1000, getAverageCore());
              if (debug9) thisPrint(" ");
              if (debug9) thisPrint(String((long)signedDeltaStart));
              if (signedDeltaStart > 0) {
                if (debug9) thisPrint("+");
              }
              if (debug9) thisPrint("(");
              if (debug9) thisPrint(pulse);
              if (deltaDUR > 0) {
                if (debug9) thisPrint("+");
              }
              if (debug9) thisPrint(String((long)deltaDUR));
              if (debug9) thisPrint(")");
            }
          } else {
            if (debug9) thisPrint("X");  // wrong start
          }
        } else {
          if (debug9) thisPrint("x");  // wrong duration
        }
      }
      previVal = inVal;
    }
	if (fLedPin < 120) {
    	digitalWrite(fLedPin, inVal < fMinValAntenna ? LOW : HIGH);
	}
    return(0);
  }

  int DCF77Decoder::circularDelta(int a, int b) {  // not absolute value
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

  int DCF77Decoder::absCircularDelta(int a, int b) {
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

  bool DCF77Decoder::isRingFull() {
    return storedDCF77upPulsesTimes.isFull();
  }

  int DCF77Decoder::getAverageCore() {
    return storedDCF77upPulsesTimes.getAverageCore();
  }

  void DCF77Decoder::pushDuration(int input) {
    return storedDCF77upPulsesTimes.push(input);
  }

  void DCF77Decoder::reset() {
    point_to_start = 0;
    for (size_t i = 0; i < 60; i++) {
      valueIndexSec[i] = 2;  // unknown
    }
    storedDCF77upPulsesTimes.reset();
    fStringForServer = "";
    for (int i = 0; i < 100 * 166; i++ ) {fStringForServer += " ";}
  }

  size_t DCF77Decoder::getStart() {
    return point_to_start;
  }

  // ---- bit access ----
  inline bool DCF77Decoder::isBitUnknown(DCF77Bit bit) const {
    size_t b = static_cast<size_t>(bit);
    return valueIndexSec[(b + point_to_start + 1) % 60] == 2;
  }

  inline int DCF77Decoder::getBit(DCF77Bit bit) const {
    size_t b = static_cast<size_t>(bit);
    return (valueIndexSec[(b + point_to_start + 1) % 60] == 0) ? 0 : 1;
  }

  // ---- parity ----
  int DCF77Decoder::getParity(DCF77Bit first, DCF77Bit last) const {
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
  int DCF77Decoder::getHour() const {
    return getBit(DCF77Bit::HOUR_1_) * 1
           + getBit(DCF77Bit::HOUR_2_) * 2
           + getBit(DCF77Bit::HOUR_4_) * 4
           + getBit(DCF77Bit::HOUR_8_) * 8
           + getBit(DCF77Bit::HOUR_10) * 10
           + getBit(DCF77Bit::HOUR_20) * 20;
  }
  int DCF77Decoder::getMin() const {
    return getBit(DCF77Bit::MIN_1_) * 1
           + getBit(DCF77Bit::MIN_2_) * 2
           + getBit(DCF77Bit::MIN_4_) * 4
           + getBit(DCF77Bit::MIN_8_) * 8
           + getBit(DCF77Bit::MIN_10) * 10
           + getBit(DCF77Bit::MIN_20) * 20
           + getBit(DCF77Bit::MIN_40) * 40;
  }

  int DCF77Decoder::getYear() const {
    return getBit(DCF77Bit::YEAR_1_) * 1
           + getBit(DCF77Bit::YEAR_2_) * 2
           + getBit(DCF77Bit::YEAR_4_) * 4
           + getBit(DCF77Bit::YEAR_8_) * 8
           + getBit(DCF77Bit::YEAR_10) * 10
           + getBit(DCF77Bit::YEAR_20) * 20
           + getBit(DCF77Bit::YEAR_40) * 40
           + getBit(DCF77Bit::YEAR_80) * 80;
  }

  String DCF77Decoder::getMonthString() {
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

  String DCF77Decoder::getDayWString() {
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

  int DCF77Decoder::getDayM() const {
    return getBit(DCF77Bit::DAYM_1_) * 1
           + getBit(DCF77Bit::DAYM_2_) * 2
           + getBit(DCF77Bit::DAYM_4_) * 4
           + getBit(DCF77Bit::DAYM_8_) * 8
           + getBit(DCF77Bit::DAYM_10) * 10
           + getBit(DCF77Bit::DAYM_20) * 20;
  }

  int DCF77Decoder::getDayW() const {
    return getBit(DCF77Bit::DAYW_1_) * 1
           + getBit(DCF77Bit::DAYW_2_) * 2
           + getBit(DCF77Bit::DAYW_4_) * 4;
  }

  int DCF77Decoder::getMonth() const {
    return getBit(DCF77Bit::MONTH_1_) * 1
           + getBit(DCF77Bit::MONTH_2_) * 2
           + getBit(DCF77Bit::MONTH_4_) * 4
           + getBit(DCF77Bit::MONTH_8_) * 8
           + getBit(DCF77Bit::MONTH_10) * 10;
  }

  // ---- validity ----
  bool DCF77Decoder::areAllOK() const {
    if (getBit(DCF77Bit::BIT_M_)) return false;
    if (1 - getBit(DCF77Bit::BIT_S_)) return false;

    if (getBit(DCF77Bit::P1) != getParity(DCF77Bit::MIN_1_, DCF77Bit::MIN_40)) return false;
    if (getBit(DCF77Bit::P2) != getParity(DCF77Bit::HOUR_1_, DCF77Bit::HOUR_20)) return false;
    if (getBit(DCF77Bit::P3) != getParity(DCF77Bit::DAYM_1_, DCF77Bit::YEAR_80)) return false;

    if (getHour() > 24) return false;
    if (getMin() > 60) return false;
    if (getDayM() > 31) return false;
    if (getDayW() > 7) return false;
    if (getMonth() > 12) return false;
    return true;
  }

  String DCF77Decoder::getString() {
    String retString = "";
    retString += String((getHour() < 10 ? "0" : "") + String(getHour())) + ":";
    retString += String((getMin() < 10 ? "0" : "") + String(getMin())) + " ";
    retString += String(getDayWString()) + " ";
    retString += getMonthString() + " ";
    retString += String((getDayM() < 10 ? "0" : "") + String(getDayM())) + " ";
    retString += String((long)(2000 + getYear())) + " ";
    if (areAllOK()) {
      retString += "AllT  ";
    } else {
      retString += getBit(DCF77Bit::BIT_M_) == 1 ? "1" : "0";
      retString += getBit(DCF77Bit::BIT_R_) == 1 ? "1" : "0";
      retString += getBit(DCF77Bit::BIT_S_) == 1 ? "1" : "0";
      retString += getParity(DCF77Bit::MIN_1_, DCF77Bit::MIN_40) == 1 ? "1" : "0";
      retString += getParity(DCF77Bit::HOUR_1_, DCF77Bit::HOUR_20) == 1 ? "1" : "0";
      retString += getParity(DCF77Bit::DAYM_1_, DCF77Bit::YEAR_80) == 1 ? "1" : "0";
    }
    return retString; 
  }

  String DCF77Decoder::getArchive(int lineNumber) {
    const int safelineNumber = lineNumber % 100;
    return fStringForServer.substring(166 * safelineNumber, 166 * (safelineNumber + 1));
  }

  void DCF77Decoder::setRaw(size_t index, int input) {
    const size_t checked_index = index % 60;
    valueIndexSec[checked_index] = input;

    // for both cases below...
    const int shiftedIndex = (60 - (point_to_start + 1) + checked_index) % 60;
    const DCF77Bit bit = static_cast<DCF77Bit>(shiftedIndex);
    const int miniString = getDigit(bit);

    if (fSaveArchiveForServer) {
      unsigned int pointerInString = 166 * fMinPointerArchive;
      if (index < fPreviousIndexForServer) { // every Minute
        fMinPointerArchive ++;
        fMinPointerArchive = fMinPointerArchive % 100;
        time_t t = now();
        String lastTimeString = (day(t) < 10 ? " " : "") + String(day(t)) + "/" +
                                (month(t) < 10 ? " " : "") + String(month(t)) + "/" +
                                (year(t) < 10 ? " " : "") + String(year(t)) + " " +
                                (hour(t) < 10 ? " " : "") + String(hour(t)) + ":" +
                                (minute(t) < 10 ? "0" : "") + String(minute(t)) + "  "
                                ;
        for (int u = 0; u < 18 ; u++) {
          fStringForServer.setCharAt(pointerInString + u, lastTimeString.charAt(u));
        }
      }
      if (input == 3) { // every Minute
        const String tmpString = getString() + "                   ";
        for (int u = 0; u < 28 ; u++) {
          fStringForServer.setCharAt(pointerInString + u + 18 + 120, tmpString.charAt(u));
        }
      }
      fPreviousIndexForServer = index; 
      
      pointerInString += 18;
      char c;
      switch (input) {
          case 0: c = ' '; break;
          case 1: c = '1'; break;
          case 2: c = '2'; break;
          case 3: c = '3'; break;
          case 4: c = '4'; break;
          default: c = '?'; break;
      }
      fStringForServer.setCharAt(pointerInString + checked_index  + 0, c);
      switch (miniString) {
          case 11: c = 'T'; break;
          case 12: c = 'F'; break;
          default: c = ' '; break;
      }
      fStringForServer.setCharAt(pointerInString + checked_index + 60, c);
      // thisPrintLN(fStringForServer.substring(pointerInString, pointerInString + 166));
    }
    if (fBitDataCallback) {
        fBitDataCallback(shiftedIndex, input, miniString, checked_index, 0);
    }
  }
  // ---- raw access for pulse logic ----
  int & DCF77Decoder::raw(size_t index) {
    return valueIndexSec[index % 60];
  }
  // read-only access (const object)
  int DCF77Decoder::raw(size_t index) const {
    return valueIndexSec[index];
  }
  void DCF77Decoder::setStart(size_t idx) {
    point_to_start = idx % 60;
  }

// retuns -1 if not valid -2 if not relevant position 0-9 Digit 11 for OK parity and 12 for not OK parity
  int DCF77Decoder::getDigit(DCF77Bit zzz) const {
      const int value = getDigitPrivate(zzz);
      if (value >= 0) {
        return value;
      }
      if (zzz == DCF77Bit::P1) {
        if (getBit(DCF77Bit::P1) == getParity(DCF77Bit::MIN_1_, DCF77Bit::MIN_40)) {return 11;} else {return 12;}
      }
      if (zzz == DCF77Bit::P2) {
        if (getBit(DCF77Bit::P2) == getParity(DCF77Bit::HOUR_1_, DCF77Bit::HOUR_20)) {return 11;} else {return 12;}
      }
      if (zzz == DCF77Bit::P3) {
        if (getBit(DCF77Bit::P3) == getParity(DCF77Bit::DAYM_1_, DCF77Bit::YEAR_80)) {return 11;} else {return 12;}
      }
      if (zzz == DCF77Bit::BIT_S_) {
        if (getBit(DCF77Bit::BIT_S_) == 1) {return 11;} else {return 12;}
      }
      if (zzz == DCF77Bit::BIT_M_) {
        if (getBit(DCF77Bit::BIT_M_) == 0) {return 11;} else {return 12;}
      }
      return 13;// should never occur
  }

  int DCF77Decoder::getDigitPrivate(DCF77Bit zzz) const {
    if (zzz == DCF77Bit::MIN_8_) {return getDigitP(DCF77Bit::MIN_1_, DCF77Bit::MIN_8_);}
    if (zzz == DCF77Bit::MIN_40) {return getDigitP(DCF77Bit::MIN_10, DCF77Bit::MIN_40);}
    if (zzz == DCF77Bit::HOUR_8_) {return getDigitP(DCF77Bit::HOUR_1_, DCF77Bit::HOUR_8_);}
    if (zzz == DCF77Bit::HOUR_20) {return getDigitP(DCF77Bit::HOUR_10, DCF77Bit::HOUR_20);}
    if (zzz == DCF77Bit::DAYM_8_) {return getDigitP(DCF77Bit::DAYM_1_, DCF77Bit::DAYM_8_);}
    if (zzz == DCF77Bit::DAYM_20) {return getDigitP(DCF77Bit::DAYM_10, DCF77Bit::DAYM_20);}
   // if (zzz == DCF77Bit::DAYW_4_) {return getDigitP(DCF77Bit::DAYW_1_, DCF77Bit::DAYW_4_);}
    if (zzz == DCF77Bit::MONTH_8_) {return getDigitP(DCF77Bit::MONTH_1_, DCF77Bit::MONTH_8_);}
    if (zzz == DCF77Bit::MONTH_10) {return getDigitP(DCF77Bit::MONTH_10, DCF77Bit::MONTH_10);}
    if (zzz == DCF77Bit::YEAR_8_) {return getDigitP(DCF77Bit::YEAR_1_, DCF77Bit::YEAR_8_);}
    if (zzz == DCF77Bit::YEAR_80) {return getDigitP(DCF77Bit::YEAR_10, DCF77Bit::YEAR_80);}
    return -2;
  }

  // returns -1 if not valid 
  int DCF77Decoder::getDigitP(DCF77Bit first, DCF77Bit last) const {
      size_t f = static_cast<size_t>(first);
      size_t l = static_cast<size_t>(last);

      int prod = 1;
      int sum = 0;
      for (size_t i = f; i <= l; i++) {
        DCF77Bit b = static_cast<DCF77Bit>(i);
        if (isBitUnknown(b)) return -1;
        if (getBit(b)) sum += prod;
        prod *= 2;
      }
      return sum;
  }


