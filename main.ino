/*
  // use pico board library by Earle F. Philhower, III 
*/

#include <time.h>
#include <TimeLib.h>
#include "hardware/timer.h"

// SERIAL DEBUG MACROS

#define SERIAL_DEBUG 0  // <<< set to 0 to disable ALL serial output

#if SERIAL_DEBUG
#define DBG_BEGIN(x) Serial.begin(x)
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#else
#define DBG_BEGIN(x)
#define DBG_PRINT(x)
#define DBG_PRINTLN(x)
#endif

enum class DCF77Bit : uint8_t;
enum class WordGP : uint8_t;  // pi pico wiring of GPOI and analogic input
class DCF77Decoder;

// Interrups to control led brightness without flickering
repeating_timer_t timer10ms;
alarm_id_t alarmID;
// -------- interrupt function prototypes --------
bool timer10msCallback(repeating_timer_t *rt);  // 100 Hz to avoid flickering
int64_t alarmCallback(alarm_id_t id, void *user_data);
// -------- Interrupt callback --------
// Called every 10 ms
bool timer10msCallback(repeating_timer_t *rt) {
  ocDriveLowAll_fullON();
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
  ocDriveLowAll_fullOFF();
  return 0;  // one-shot alarm
}


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

#define LEDPIN LED_BUILTIN

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
  RADIOINPUT = 28,  // GP28 A2
};

class Ring {
private:
  size_t pointer;
  const size_t maxSize;
  size_t size;
  size_t lastPointer;
  int *array;
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
  int getSize() const {
    return size;
  }
  void reset() {
    size = 0;
    pointer = 0;
    lastPointer = 0;
  }
  bool isFull() {
    return size == maxSize;
  }
  int getValue(size_t item) const {
    if (item >= size) return 0;  // or error
    return array[item];
  }
  int getLast() const {
    return array[lastPointer];
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
};

const bool debug8 = false;  // text
//txt:[+++++++----+-++++-+-+++---+---+----+---+----++--_---£-++++-+],Lms:276, 11:57 Mon Feb 2/2026 All
const bool debug9 = debug8;  // display long and short pulses on the fly
const bool debug5 = false;   // display long pause pulses on the fly
bool debug2 = ! debug8;      // front display debugging steps
const bool debug3 = true;    // dump info about the validation process of times
Ring storedUpTimes(50);

const size_t totOutputPins = 22;
size_t values[totOutputPins];




class DCF77Decoder {
public:
  static constexpr size_t SECONDS = 60;

  DCF77Decoder() {
    reset();
  }

  void reset() {
    point_to_start = 0;
    for (size_t i = 0; i < SECONDS; i++) {
      valueIndexSec[i] = 2;  // unknown
    }
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

private:
  int valueIndexSec[60];
  size_t point_to_start;
};

bool firstSettingTime = true;
// DCF77 management
//int valueIndexSec[60]; // DEL
//size_t point_to_start = 0; // DEL

// int miliStore = 0; // for debuf of moment of second

// Release all pins (Hi-Z, no pull)






void ocReleaseAll() {
  for (size_t lo = 0; lo < totOutputPins; lo++) {
    pinMode(lo, INPUT);  // no pull
  }
}

int inputValue;

// Drive all pins LOW (open-collector active)
void ocDriveLowAll_fullON() {
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

void ocDriveLowAll_fullOFF() {
  for (size_t lo = 0; lo < totOutputPins; lo++) {
    pinMode(lo, INPUT);  // OFF State
  }
}

// chop led current
void ocDriveLowAll(size_t cycles = 10) {
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



void printTime(time_t t = now()) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%02d %02d %04d %02d:%02d:%02d",
           day(t), month(t), year(t),
           hour(t), minute(t), second(t));
  DBG_PRINTLN(buf);  // or Serial.println(buf);
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

void setOutputLed(const int curMin, const int curHourtrue, const bool writeItIs = true) {
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

void displayUnit(int unitDigit = 0) {
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

class ClockControl {
private:
  bool qualityAccept;  // 0 take any time and does not running time correction. 1// used for correction
  size_t pointer;
  long int period;
  bool isPositiveCorrection;
  const size_t fsize;
  time_t *tArray;
  long int *rArray;
public:
  ClockControl(size_t size)
    : qualityAccept(false), pointer(0), period(0), isPositiveCorrection(true), fsize(size) {
    tArray = new time_t[size];
    rArray = new long int[size];
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
  long int getLastCorrection() {
    if (pointer == 0) { return 0; }
    return rArray[pointer];
  }

  // curSec is counter incrementing every second and call for each value (every second)
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

  void storeDate(time_t tNow, long errorSecondsPerDay) {
    tArray[pointer] = tNow;
    rArray[pointer] = errorSecondsPerDay;
    if (errorSecondsPerDay == 0) {
      period = 0;
    } else {
      const long durationOneDay = 24 * 60 * 60;
      period = durationOneDay / errorSecondsPerDay;
    }
    isPositiveCorrection = errorSecondsPerDay > 0;
    pointer++;
    if (pointer == fsize) pointer = 1;  // so pointer == 0 is only when nothing...
  }

  void storeTime(tmElements_t tm) {

    const time_t t = makeTime(tm);
    const time_t tNow = now();

    // analyse correction
    const long long diff = (long long)t - (long long)tNow;  // error in seconds / if positive internal time is too slow
    const long long unsignedDiff = labs(diff);
    const time_t lastTime = getLastTime();
    const long long secondsSinceLastTime = (long long)t - lastTime;
    //const long int secondsSinceLastTime_now = (long)tNow - lastTime;
    if (debug3) DBG_PRINTLN("");
    if (debug3) DBG_PRINT("Time submitted for validation ");
    if (debug3) DBG_PRINT("Earlyer data reliable : ");
    if (debug3) {
      if (qualityAccept) DBG_PRINTLN("Y");
      else DBG_PRINTLN("N");
    }
    if (debug3) DBG_PRINT("tested time      : ");
    if (debug3) printTime(t);
    if (debug3) DBG_PRINT("now :            : ");
    if (debug3) printTime(tNow);
    if (debug3) DBG_PRINT("last stored time : ");
    if (debug3) printTime(lastTime);
    const bool critConsistent = (unsignedDiff < 100);
    if (!qualityAccept) {
      if (debug3) DBG_PRINT(diff);
      if (debug3) DBG_PRINTLN(" < 100");
      setTime(t);
      // see if can consider switch to reliable
      if (critConsistent) {
        if (debug3) DBG_PRINTLN("Crit OK: Switch to reliable");
        qualityAccept = true;
      } else {
        if (debug3) DBG_PRINTLN("Crit failed: Time is not consistent (normal if first call). It is not considered as reliable");
      }
    }
    // dont use else because qualityAccept changes in if
    if (qualityAccept) {
      if (!critConsistent) {
        if (debug3) DBG_PRINTLN(" Crit failed: Time is not consistent: ignored");
        return;
      }
      const long long factor = 600;  // 1 second per minute : 60 // 1 second ten minutes : 600 // 1 second per hour: 3600
      const long long secondSinceLastAbs = labs(secondsSinceLastTime);
      const bool critConsistent = (unsignedDiff < (secondSinceLastAbs / factor));
      if (debug3) DBG_PRINT(factor);
      if (debug3) DBG_PRINT(" * ");
      if (debug3) DBG_PRINT(diff);
      if (debug3) DBG_PRINT(critConsistent ? " < " : " > ");
      if (debug3) DBG_PRINT(secondSinceLastAbs);
      if (pointer == 0) {
        storeDate(tNow, 0L);
        return;
      }
      if (!critConsistent) {
        if (debug3) DBG_PRINTLN(" Crit failed times not consistent rejects time in quality mode");
        return;
      }
      setTime(t);  // correct time

      if (debug3) DBG_PRINTLN(" Crit consistent : save time and consider calculate error for fine tuning in quality mode");

      const long long minNumberSeconds = 60 * 60;  // 60 * 60 : 1 Hour
      if (unsignedDiff < minNumberSeconds) {
        if (debug3) DBG_PRINTLN(" duration not long enough for meaningfull time correction:");
        if (debug3) DBG_PRINT(unsignedDiff);
        if (debug3) DBG_PRINT(" < ");
        if (debug3) DBG_PRINTLN(minNumberSeconds);
        return;
      }
      if (debug3) DBG_PRINTLN(" Calculate time correction:");
      const long long durationOneDay = 24 * 60 * 60;
      long long errorSecondsPerDay = (diff * durationOneDay) / ((long long)tNow - lastTime);
      if (true) { errorSecondsPerDay += getLastCorrection(); }
      if (debug3) DBG_PRINT(" errorSecondsPerDay = ");
      if (debug3) DBG_PRINT(diff);
      if (debug3) DBG_PRINT(" * ");
      if (debug3) DBG_PRINT(durationOneDay);
      if (debug3) DBG_PRINT(" / (");
      if (debug3) DBG_PRINT(tNow);
      if (debug3) DBG_PRINT(" - ");
      if (debug3) DBG_PRINT(lastTime);
      if (debug3) DBG_PRINT(" : ");
      if (debug3) DBG_PRINTLN(errorSecondsPerDay);

      storeDate(tNow, errorSecondsPerDay);
      if (debug3) DBG_PRINT(" period:");
      if (debug3) {
        if (isPositiveCorrection) DBG_PRINT("+");
        else DBG_PRINT("-");
      }
      if (debug3) DBG_PRINTLN(period);
    }
    return;
  }
};

ClockControl theClockControl(10);


DCF77Decoder dcf77;

void setup() {
  // inteerup
  add_repeating_timer_us(
    -10000,  // negative = exact interval, no drift
    timer10msCallback,
    NULL,
    &timer10ms);
  if (debug2) displayUnit(1);
  DBG_BEGIN(115200);
  delay(2000);
  analogReadResolution(12);
  DBG_PRINT("DCF77 pin : ");
  DBG_PRINTLN(RADIOINPUT);
  // Start with everything released
  ocReleaseAll();
  for (size_t lo = 0; lo < totOutputPins; lo++) {
    const size_t pin = lo;
    values[pin] = 0;
  }


  // setting up Led
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  DBG_PRINTLN("End setup");
  if (debug2) displayUnit(2);
}


void loop() {
  if (debug2) displayUnit(3);
  for (int superLoop = 0; superLoop < 100000000; superLoop++) {
    int previVal = 0;
    // const int numberWait = 918;
    // const int factorEndOfLine = 120;
    // int count = 0;
    unsigned long int startUp = 0;
    unsigned long int startDown = 0;
    unsigned long int lastStartUp = 0;
    unsigned long int lastStartDown = 0;
    unsigned long int cMili = 0;
    size_t oldIndexSec = 0;
    const bool cursor_on = true;
    unsigned long int countDownValidTime = 0;             // in seconds
    const unsigned long int initCountDownValidTime = 30;  // debug       60 * 60 * 24 * 30 ; // one month in seconds

    DBG_PRINTLN("starts loop1");
    storedUpTimes.reset();  // assume the phase of pulses is lost. correct after minutes
    for (int loop1 = 0; loop1 < 100000000; loop1++) {

      // meansure DCF77 level
      const int inVal = analogRead((size_t)WordGP::RADIOINPUT);
      cMili = millis();
      const unsigned long int milisOnly = (cMili % 1000UL);
      //const size_t avDur = 150; // average duration pulse
      const size_t supForRounding = 500;  // average duration pulse
      const unsigned long int substract = static_cast<unsigned long int>(supForRounding + storedUpTimes.getAverageCore());
      const long seconds = (cMili - substract) / 1000L;
      const size_t indexSec = static_cast<size_t>(((cMili - substract) / 1000UL) % 60UL);

      if (oldIndexSec != indexSec) {
        theClockControl.adjustTime(seconds);
        oldIndexSec = indexSec;
        if (debug2) displayUnit(10);

        // manage validity of time set by initCountDownValidTime
        if (countDownValidTime > 0) countDownValidTime -= 1;

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
              if (dcf77.raw(i) == 2) { DBG_PRINT(" "); }
              if (dcf77.raw(i) == 3) {
                DBG_PRINT("£");
                {
                  //const size_t store_val2 = dcf77.
                  const size_t store_val = dcf77.getStart();
                  dcf77.setStart((i + 0) % 60);
                  stringForLine += dcf77.getString() + " ";
                  dcf77.setStart(store_val);
                }
              }
              if (dcf77.raw(i) == 0) { DBG_PRINT("-"); }
              if (dcf77.raw(i) == 1) { DBG_PRINT("+"); }
            }
          }
          DBG_PRINT("],");
          if (storedUpTimes.isFull()) { DBG_PRINT("L"); }
          DBG_PRINT("ms:");
          DBG_PRINT(storedUpTimes.getAverageCore());  // miliStore
          DBG_PRINT(", ");
          DBG_PRINTLN(stringForLine);
        }
        // IMPORTNT : Here may want to reload each bit at each cycle be having next line uncommented
        //valueIndexSec[indexSec] = 2;
        bool displayStrongTimeWhileReceptingSignal = false;
        if (displayStrongTimeWhileReceptingSignal) {
          time_t t = now();
          setOutputLed(minute(t), hour(t), theClockControl.isReliable());
          ocDriveLowAll_fullON();
        }
      }
      // DOWN -> UP
      if ((inVal > 1000) && (previVal < 1000)) {
        startUp = cMili;
        const int durCycleMili = cMili - lastStartUp;
        const int margin = 50;  //  margin ms
        // detect large gap for end of DCF77 minute cycle : 2000 ms (no pulse at second 59)
        const int mi = 2000 - margin;
        const int ma = 2000 + margin;

        if (durCycleMili > mi && durCycleMili < ma) {

          const size_t pointerInArrayMinus = (indexSec + 59) % 60;
          dcf77.setRaw(pointerInArrayMinus, 3);

          if (debug2) displayUnit(4);
          dcf77.setStart(pointerInArrayMinus);

          if (dcf77.areAllOK()) {

            firstSettingTime = false;
            tmElements_t tm;
            tm.Year = CalendarYrToTm(2026);
            tm.Month = dcf77.getMonth() - 1;  // jan = 0;
            tm.Day = dcf77.getDayM();
            tm.Hour = dcf77.getHour();
            tm.Minute = dcf77.getMin();
            tm.Second = 0;
            theClockControl.storeTime(tm);
            //if (theClockControl.isReliable())
            countDownValidTime = initCountDownValidTime;
            debug2 = false;  // stop
          }
        }
        // detect small gaps between seconds
        const int mi2 = 1000 - margin;
        const int ma2 = 1000 + margin;
        if (durCycleMili > mi2 && durCycleMili < ma2) {
          if (debug5) DBG_PRINT("^");
          if (debug5) DBG_PRINT(milisOnly);
          if (debug5) DBG_PRINT("<");
          storedUpTimes.push(milisOnly);
          //DBG_PRINT(storedUpTimes.dump());
          if (debug5) DBG_PRINT(storedUpTimes.getAverageCore());
          if (debug5) DBG_PRINT(">");
        }
        previVal = inVal;
      }

      // UP -> DOWN
      if ((inVal < 1000) && (previVal > 1000)) {

        const int durCycleMili = cMili - lastStartDown;
        const int minDurationRealPulse = storedUpTimes.isFull() ? 500 : 10;  // not very logical
        if (durCycleMili > minDurationRealPulse) {
          lastStartDown = cMili;
          startDown = cMili;

          //miliStore = static_cast<int>(milisOnly);
          int durUpMili = cMili - startUp;
          const int delta = absCircularDelta(startUp % 1000, storedUpTimes.getAverageCore());
          //milisOnly // storedUpTimes.getAverageCore()

          // margin of 50 ms for duration and position
          const int critDeltaPos = storedUpTimes.isFull() ? 50 : 1000;  // set tight only when ring is full
          const int critDeltaDur = 50;                                  // if change 50, rewrite code below
          const int mi = 100 - critDeltaDur;
          const int ma = 200 + critDeltaDur;

          if (durUpMili >= mi && durUpMili <= ma) {
            lastStartUp = startUp;

            if (delta < critDeltaPos) {

              // Store value
              int deltaDUR = 0;
              String pulse = "";
              if (durUpMili < 150) {  // short pulse
                if (debug2) displayUnit(8);
                //valueIndexSec[indexSec] = 0; DEL
                dcf77.setRaw(indexSec, 0);

                deltaDUR = durUpMili - 100;
                pulse = "S";
              } else {
                if (debug2) displayUnit(7);
                dcf77.setRaw(indexSec, 1);
                //valueIndexSec[indexSec] = 1; DEL
                deltaDUR = durUpMili - 200;
                pulse = "L";
              }
              if (cursor_on) {
                int signedDeltaStart = circularDelta(startUp % 1000, storedUpTimes.getAverageCore());
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

      digitalWrite(LEDPIN, inVal < 1000 ? LOW : HIGH);
      if (countDownValidTime > 0) {
        //////////// HERE go to loop with no reception
        break;
      }
    }

    // Test each output led
    const bool testEachLedFirst = false;
    if (testEachLedFirst) {
      for (size_t p = 0; p < totOutputPins; p++) {
        for (size_t lo = 0; lo < totOutputPins; lo++) {
          const size_t pin = lo;
          if (lo == p) {
            dcf77.setRaw(pin, 1);//[pin] = 1;
          } else {
            dcf77.setRaw(pin, 0);//[pin] = 0;
          }
        }
        ocDriveLowAll();
      }
    }
    long long last_min = 0;
    // ignore countDownValidTime
    unsigned long int numberMinStaysInLoop = theClockControl.isReliable() ? 60 : 10;
    DBG_PRINT("starts loop2 for ");
    DBG_PRINT(numberMinStaysInLoop);
    DBG_PRINTLN(" min. ");
    time_t t = now();
    setOutputLed(minute(t), hour(t), theClockControl.isReliable());
    int lastMin = minute(t);
    for (unsigned long long loo = 0UL; loo < 1000000000; loo++) {
      t = now();
      int curMin = minute(t);
      if (curMin != lastMin) {
        lastMin = curMin;
        digitalWrite(LEDPIN, ((curMin % 2) == 0) ? LOW : HIGH);
        setOutputLed(minute(t), hour(t), theClockControl.isReliable());
        numberMinStaysInLoop -= 1;
        if (numberMinStaysInLoop <= 0) break;
        sleep_ms(55000);  // wait less than a minute // delay()
      }
    }
    DBG_PRINT("ends loop2 for ");
  }
}
