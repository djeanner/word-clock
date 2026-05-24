
#include "geigerCounter.h"

GeigerCounter::GeigerCounter(
		size_t aInput, 
		bool in, 
		pin_size_t aPin,
	    const bool aUseSerial, 
		const bool aSaveArchiveForServer) : 
	fInputPin(aInput), 
	fDebug(in), 
	fLedPin(aPin), 
	fUseSerial(aUseSerial),
      // fStringCallback({}),
      // fBitDataCallback({}),
    fSaveArchiveForServer(aSaveArchiveForServer)
{
	reset();
}

void GeigerCounter::reset() {
	
}
