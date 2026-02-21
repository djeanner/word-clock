#pragma once
#include <Arduino.h> // add in header of classes
#include <time.h>
#include <TimeLib.h>
#include "hardware/timer.h"

#include "ClockControl.h"


/// @brief listen to analogic input of DCF77 antenna and saves time with the listen method
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
  bool debug5;
  bool debug8;
  bool debug9;
  pin_size_t fLedPin;
  const bool fUseSerial;
	std::function<void(String)> fStringCallback;
	std::function<void(int, int, int, int, int)> fBitDataCallback;
  const bool fSaveArchiveForServer;
  unsigned int fPreviousIndexForServer;
  unsigned int fMinPointerArchive;
  String fStringForServer;

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
  DCF77Decoder(size_t aInput, long int aLongInt, bool in2, bool in5, bool in8, bool in9, pin_size_t aPin, bool useSerial = false, bool fSaveArchiveForServer = false);

  // destructor
  ~DCF77Decoder();
  String getArchive(int lineNumber);
  void setBitDataCallback(std::function<void(int, int, int, int, int)> aBitDataCallback = {});
  void setStringCallback(std::function<void(String)> aStringCallback = {});
  void thisPrintLN(String aString = "");
  void thisPrint(String aString = "");
  void initListen();
  tmElements_t getTM();
  int listen(ClockControl & theClockControl);
  int circularDelta(int a, int b);
  int absCircularDelta(int a, int b);
  bool isRingFull();
  int getAverageCore();
  void pushDuration(int input);
  void reset();
  size_t getStart();
  inline bool isBitUnknown(DCF77Bit bit) const ;
  inline int getBit(DCF77Bit bit) const;
  int getParity(DCF77Bit first, DCF77Bit last) const;
  int getHour() const ;
  int getMin() const ;
  int getYear() const ;
  String getMonthString();
  String getDayWString();
  int getDayM() const ;
  int getDayW() const ;
  int getMonth() const ;
  bool areAllOK() const;
  String getString();
  void setRaw(size_t index, int input);
  int &raw(size_t index);
  int raw(size_t index) const;
  void setStart(size_t idx);
  int getDigit(DCF77Bit zzz) const ;
private:
  int getDigitPrivate(DCF77Bit zzz) const ;
  int getDigitP(DCF77Bit first, DCF77Bit last) const ;
};

