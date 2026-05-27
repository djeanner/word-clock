
#include "geigerCounter.h"

GeigerCounter::GeigerCounter(size_t aInput, bool in, pin_size_t aPin,
                             const bool aUseSerial,
                             const bool aSaveArchiveForServer,
                             const size_t aSize)
    : fInputPin(aInput), fDebug(in), fLedPin(aPin), fUseSerial(aUseSerial),
      // fStringCallback({}),
      // fBitDataCallback({}),
      fSaveArchiveForServer(aSaveArchiveForServer), fsize(aSize),
      fNumberHitFromStart(0) {
  ftArray = new unsigned long long[fsize];
  reset();
}

GeigerCounter::~GeigerCounter() { delete[] ftArray; }

void GeigerCounter::reset() {
  fNumberHit = 0;
  fPrevMilis = 0;
  fPointerTArray = 0;
  fSumDurations = 0;
  fSum10Last = 0;
  fSum100Last = 0;
  for (size_t i = 0; i != fsize; i++) {
	ftArray[i] = 0ll;
  }
}

// fast function. May be called by interrupt
void GeigerCounter::pulse(const unsigned long long aMilis) {

  fLastNumberMilis = aMilis - fPrevMilis;
  fSumDurations += fLastNumberMilis;
  fNumberHit++;
  fNumberHitFromStart++;
  fPrevMilis = aMilis;

  // store in ftArray ring
  ftArray[fPointerTArray] = fLastNumberMilis;
  fPointerTArray++;
  if (fPointerTArray == fsize) {
    fPointerTArray = 0;
  }

  fSum10Last += fLastNumberMilis;
  fSum100Last += fLastNumberMilis;
  if (fNumberHitFromStart >= 10) {fSum10Last -= ftArray[pointerBefore(10 + 1)];};
  if (fNumberHitFromStart >= 100) {fSum100Last -= ftArray[pointerBefore(100 + 1)];};
}

String GeigerCounter::getString2() {
  
  String returnedString = "  "; 
  returnedString += "5:" + String((long)averageLastN(5)) + " ";
  returnedString += "10:" + String((long)averageLastN(10)) + " ";
  returnedString += "100:" + String((long)averageLastN(100)) + " ";

  
  returnedString += "\n";
  return returnedString;
}
String GeigerCounter::getString() {
  const String numString = String((long)fLastNumberMilis);
  const String numString2 = String((float)(fSumDurations) / (float)fNumberHit);
  String returnedString = " up " + numString + " " + numString2 + " "; 
    returnedString += String((long)fNumberHitFromStart) + " ";

 // returnedString += "5:" + String((long)averageLastN(5)) + " ";
 // returnedString += "10:" + String((long)averageLastN(10)) + " ";

  
  returnedString += "\n";
  return returnedString;
}
unsigned long long GeigerCounter::averageLastN(const size_t lastN) {
 size_t divider = lastN;
if (lastN > fNumberHitFromStart) {
   divider =  fNumberHitFromStart;
  }
  if (lastN == 10) {
    return fSum10Last / divider;
  }
  if (lastN == 100) {
    return fSum100Last / divider;
  }
  unsigned long long sum = 0;
  if (fPointerTArray >= lastN) {
    for (size_t i = fPointerTArray - lastN; i != fPointerTArray; i++) {
      sum += ftArray[i];
    }
  } else {
    for (size_t i = 0; i != fPointerTArray; i++) {
      sum += ftArray[i];
    }
    for (size_t i = fsize + fPointerTArray - lastN; i != fsize; i++) {
      sum += ftArray[i];
    }
  }
  return sum / divider;
}

size_t GeigerCounter::pointerBefore(const size_t aInt) {
  if (aInt >= fsize) {
    return 0;
  }
  if (fPointerTArray >= aInt) {
    return (fPointerTArray - aInt);
  } else {
    return (fsize + fPointerTArray - aInt);
  }
}