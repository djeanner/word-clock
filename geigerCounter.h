#pragma once
#include <Arduino.h> // add in header of classes

/// @brief listen to analogic input of geiger counter hit
class GeigerCounter {
public:
  GeigerCounter(size_t aInput, bool in, pin_size_t aPin,
                const bool aUseSerial = false,
                const bool aSaveArchiveForServer = false,
                const size_t aSize = 200);
  ~GeigerCounter();

  void pulse(const unsigned long long aMilis, unsigned long numberPulses = 1);
  String getString();
  String getString2();

  unsigned long long averageLastN(const size_t lastN);

private:
  const size_t fInputPin;
  const bool fDebug;
  const pin_size_t fLedPin;
  const bool fUseSerial;
  const bool fSaveArchiveForServer;
  const size_t fsize;
  unsigned long long fNumberHitFromStart;
  const size_t fNumberPositonsInPulseArray;
  unsigned long long fai0;
  const unsigned long long
      fFirstDigitFactor; // if miliseconds. Base for 10: 10ms    100: 100 ms...
  const unsigned long long fBaseStorage; // initially 10 will become 2


  size_t fPointerTArray;
  unsigned long long *ftArray;
  unsigned long long fMilis;
  unsigned long long fLastNumberMilis;
  unsigned long long fSumDurations;
  unsigned long long fNumberHit;
  unsigned long long fPrevMilis;

  unsigned long long fSum10Last;
  unsigned long long fSum100Last;
  float *fAverage;
  float *fStdDev;
  float *fTwoHalfAvStd;

  // for numberPulses
  unsigned long *fPulseArray;
  unsigned long *fPulseSum;
  unsigned int *fPulseNbv;
  unsigned int *fDigitsPrevValue;
  unsigned int *fPulseIndex;
  
  // std::function<void(String)> fStringCallback;
  // std::function<void(int, int, int, int, int)> fBitDataCallback;

  void reset();
  size_t pointerBefore(const size_t aInt);
  void pushPulses(unsigned long long i0, const unsigned long numberHits,
                  const bool incrementPosition = false);

  void setArray(const size_t a, const unsigned int b, const unsigned long value,
                const bool incrementPosition);
  bool isWithin(float a, float b, float c);
  bool isDisjoint(float a, float ae, float b, float c);
};