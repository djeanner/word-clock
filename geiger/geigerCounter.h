#pragma once
#include <Arduino.h> // add in header of classes

/// @brief listen to analogic input of geiger counter hit
class GeigerCounter {
public:
 GeigerCounter(size_t aInput, bool in, pin_size_t aPin, const bool aUseSerial = false,
                const bool aSaveArchiveForServer = false);
private:
  const size_t fInputPin;
  const bool fDebug;
  const pin_size_t fLedPin;
  const bool fUseSerial;
  const bool fSaveArchiveForServer;

  // std::function<void(String)> fStringCallback;
  // std::function<void(int, int, int, int, int)> fBitDataCallback;
 
  void reset();
};