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

  void pulse(const unsigned long long aMilis);
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
  long long fNumberHitFromStart;

  size_t fPointerTArray;
  unsigned long long *ftArray;
  unsigned long long fMilis;
  unsigned long long fLastNumberMilis;
  unsigned long long fSumDurations;
  unsigned long long fNumberHit;
  unsigned long long fPrevMilis;

  unsigned long long fSum10Last;
  unsigned long long fSum100Last;

  // std::function<void(String)> fStringCallback;
  // std::function<void(int, int, int, int, int)> fBitDataCallback;

  void reset();
  size_t pointerBefore(const size_t aInt);

};