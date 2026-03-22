#pragma once
#include <Arduino.h> // add in header of classes
#include <functional>
#include <time.h>
#include <TimeLib.h>
#include "hardware/timer.h"
class ClockControl {
private:
	bool qualityAccept;  // 0 take any time and does not running time correction. 1// used for correction
	size_t pointer;
	long int period;
	bool isPositiveCorrection;
	const size_t fsize;
	const bool fdebug;
	const bool fUseSerial;
	std::function<void(String)> fStringCallback;
	time_t *tArray;
	long long *rArray;
public:
  ClockControl(size_t size, bool in3, bool abool);
  ~ClockControl() ;
  String getStringDateHourMinReliable();
  void setStringCallback(std::function<void(String)> aStringCallback = {});
  void thisPrint(String aString = "") ;
  void thisPrintLN(String aString = "") ;
  bool isReliable() ;
  time_t getLastTime() ;
  long long getLastCorrection() ;
  void interruptAdjust();
  void adjustTime(long int curSec) ;
  void adjustTimeMinute(long int curMin);
  void storeDate(const time_t tNow, const long long errorSecondsPerDay) ;
  String stringTime(time_t t = now()) ;
  void storeTime(tmElements_t tm, bool forceReliable = false);
};
