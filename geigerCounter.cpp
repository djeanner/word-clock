#line 1 "/Users/djeanner/git/word-clock/geiger/geigerCounter.cpp"

#include "geigerCounter.h"

GeigerCounter::GeigerCounter(size_t aInput, bool in, pin_size_t aPin,
                             const bool aUseSerial,
                             const bool aSaveArchiveForServer,
                             const size_t aSize)
    : fInputPin(aInput), fDebug(in), fLedPin(aPin), fUseSerial(aUseSerial),
      // fStringCallback({}),
      // fBitDataCallback({}),
      fSaveArchiveForServer(aSaveArchiveForServer), fsize(aSize),
      fNumberHitFromStart(0), fNumberPositonsInPulseArray(8), fai0(0),
      fFirstDigitFactor(100) {
  ftArray = new unsigned long long[fsize];

  // for numberPulses
  fPulseArray = new unsigned long[fNumberPositonsInPulseArray * 10];
  fPulseSum = new unsigned long[fNumberPositonsInPulseArray];
  fPulseNbv = new unsigned int[fNumberPositonsInPulseArray];
  fAverage = new float[fNumberPositonsInPulseArray];
  fStdDev = new float[fNumberPositonsInPulseArray];
  fTwoHalfAvStd = new float[fNumberPositonsInPulseArray * 6];

  reset();
}

GeigerCounter::~GeigerCounter() {
  delete[] ftArray;
  delete[] fPulseArray;
  delete[] fPulseSum;
  delete[] fPulseNbv;
  delete[] fAverage;
  delete[] fStdDev;
  delete[] fTwoHalfAvStd;
}

void GeigerCounter::reset() {
  fNumberHit = 0;
  fPrevMilis = 0;
  fPointerTArray = 0;
  fSumDurations = 0;
  fSum10Last = 0;
  fSum100Last = 0;
  for (size_t i = 0; i != fsize; i++) {
    ftArray[i] = 0ull;
  }

  // for numberPulses
  for (size_t i = 0; i != fNumberPositonsInPulseArray * 10; i++) {
    fPulseArray[i] = 0ull;
  }
  for (size_t i = 0; i != fNumberPositonsInPulseArray * 6; i++) {
    fTwoHalfAvStd[i] = 0.0f;
  }

  for (size_t i = 0; i != fNumberPositonsInPulseArray; i++) {
    fPulseSum[i] = 0ull;
    fPulseNbv[i] = 0;
    fAverage[i] = 0.0f;
    fStdDev[i] = 0.0f;
  }
}

// fast function. May be called by interrupt
void GeigerCounter::pulse(const unsigned long long aMilis, unsigned long numberPulses) {

  fLastNumberMilis = aMilis - fPrevMilis;
  fSumDurations += fLastNumberMilis;
  fNumberHit += numberPulses;
  fNumberHitFromStart += numberPulses;
  fPrevMilis = aMilis;

  // store in ftArray ring and make average time over 10 and 100 past points
  const long long partLastNumberMilis = fLastNumberMilis / numberPulses;
  const long long partLastNumberMilisRest =
      fLastNumberMilis - partLastNumberMilis * numberPulses;
  for (unsigned long i = 0; i != numberPulses; i++) {
    const long long curPartLastNumberMilis =
        (i + 1 == numberPulses) ? partLastNumberMilis + partLastNumberMilisRest
                                : partLastNumberMilis;
    ftArray[fPointerTArray] = curPartLastNumberMilis;
    fPointerTArray++;
    if (fPointerTArray == fsize) {
      fPointerTArray = 0;
    }

    fSum10Last += curPartLastNumberMilis;
    fSum100Last += curPartLastNumberMilis;
    if (fNumberHitFromStart >= 10) {
      fSum10Last -=
          ftArray[pointerBefore(10 + 1)]; // +1 because already incremented...
    };
    if (fNumberHitFromStart >= 100) {
      fSum100Last -=
          ftArray[pointerBefore(100 + 1)]; // +1 because already incremented...
    };
  }

  // storage in table of number hits per time unit
  const unsigned long long i0 = aMilis / fFirstDigitFactor;
  const unsigned long long deltaBas = i0 - fai0;
  const unsigned long longavNumberPulses = numberPulses / deltaBas;
  const unsigned long longreNumberPulses =
      numberPulses - longavNumberPulses * deltaBas;
  const bool showDetails = false;
  for (size_t i = 0; i != deltaBas; i++) {

    const unsigned long curNumberPulses =
        ((i + 1) == deltaBas) ? longavNumberPulses + longreNumberPulses
                              : longavNumberPulses;

    pushPulses(fai0 + i, curNumberPulses);
    if (fUseSerial && showDetails) {
      Serial.print("--");
      Serial.print(" ");
      Serial.print((long)(fai0 + i));
      Serial.print(" ");
      Serial.print((long)(numberPulses));
      Serial.print("\n");
    }
  }
  if (deltaBas == 0) {
    if (fUseSerial && showDetails) {
      Serial.print("->");
      Serial.print(" ");
      Serial.print((long)(i0));
      Serial.print(" ");
      Serial.print((long)(numberPulses));
      Serial.print("\n");
    }
    pushPulses(i0, numberPulses, true);
  }
  fai0 = i0;
  if (fUseSerial) {

    Serial.print("--------------");
    Serial.print(" ");
    for (size_t i = 0; i < fNumberPositonsInPulseArray; i++) {
      if (fPulseNbv[i] > 0) {
        if (fPulseNbv[i] == 10) {
          Serial.print((long)(fPulseSum[i]));
        } else {
          Serial.print((long)(10 * fPulseSum[i] / fPulseNbv[i]));
          Serial.print("(");
          Serial.print((long)(fPulseNbv[i]));
          Serial.print(")");
        }
        Serial.print(" ");
      }
    }
    Serial.print("\n");
    Serial.print("----------------------");
    Serial.print((long)(i0));
    /*
2913 add u
        const size_t iMod = a * 10 + ((10 - i + b) % 10);


13410 on granite

-------------- 1 4 93 1381 12710(1) 
----------------------13803 0:1.0[3.0]300%   1:0.4[0.5]122%   2:0.9[0.6]61%   3:1.38[0.09] 6%   4:1.27(1)   
2---------------------13803 0:2.5[4.3]173% - 1:0.2[0.4]173%-- 2:0.3[0.2]59%+* 3:1.40[0.10] 7%-- Z2:13403 
2---------------------13803 0:0.0[0.0]inf% - 1:0.5[0.5]100%-- 2:1.4[0.4]28%+* 3:1.40[0.06] 5%-- Z2:13403 
 0.00 0.00 9.00 150.00 1271.00 0.00 0.00 0.00 
 0.00 0.00 17.00 134.00 0.00 0.00 0.00 0.00 
 1.00 0.00 11.00 150.00 0.00 0.00 0.00 0.00 
 0.00 1.00 11.00 133.00 0.00 0.00 0.00 0.00 
 0.00 1.00 6.00 150.00 0.00 0.00 0.00 0.00 
 0.00 1.00 2.00 139.00 0.00 0.00 0.00 0.00 
 0.00 1.00 1.00 139.00 0.00 0.00 0.00 0.00 
 0.00 0.00 4.00 132.00 0.00 0.00 0.00 0.00 
 0.00 0.00 14.00 127.00 0.00 0.00 0.00 0.00 
 0.00 0.00 18.00 127.00 0.00 0.00 0.00 0.00 
Means 133XX - 134XX ( in 2:) not best
-------------- 1 3 82 1381 12710(1) 
----------------------13900 0:1.0[3.0]300%   1:0.3[0.5]153%   2:0.8[0.6]70%   3:1.38[0.09] 6%   4:1.27(1)   
2---------------------13900 0:2.5[4.3]173% - 1:0.5[0.5]100% - 2:0.3[0.1]45%+* 3:1.40[0.10] 7%-- Z2:13500 
2---------------------13900 0:0.0[0.0]inf% - 1:0.0[0.0]inf% - 2:1.2[0.3]25%+* 3:1.40[0.06] 5%-- Z2:13500 
 0.00 1.00 9.00 150.00 1271.00 0.00 0.00 0.00 
 0.00 0.00 17.00 134.00 0.00 0.00 0.00 0.00 
 0.00 0.00 11.00 150.00 0.00 0.00 0.00 0.00 
 0.00 0.00 11.00 133.00 0.00 0.00 0.00 0.00 
 0.00 0.00 6.00 150.00 0.00 0.00 0.00 0.00 
 0.00 0.00 2.00 139.00 0.00 0.00 0.00 0.00 
 0.00 0.00 1.00 139.00 0.00 0.00 0.00 0.00 
 0.00 1.00 4.00 132.00 0.00 0.00 0.00 0.00 
 0.00 0.00 3.00 127.00 0.00 0.00 0.00 0.00 
 1.00 1.00 18.00 127.00 0.00 0.00 0.00 0.00 
Means 134XX - 135XX ( in 2:) better

------------- 1 3 49 1008 12710(1) 
----------------------17015 0:1.0[3.0]300%   1:0.3[0.6]213%   2:0.5[0.2]42%   3:1.0[0.5]45%   4:1.3(1)   
2---------------------17015 0:2.5[4.3]173% - 1:0.0[0.0]inf% - 2:0.6[0.2]38% - 3:0.5[0.1]28%+* Z3:13015 
2---------------------17015 0:0.0[0.0]inf% - 1:0.5[0.9]173% - 2:0.3[0.1]32% - 3:1.3[0.1] 7%+* Z3:13015 
 0.00 0.00 5.00 150.00 1271.00 0.00 0.00 0.00 
 0.00 0.00 5.00 134.00 0.00 0.00 0.00 0.00 
 0.00 0.00 4.00 150.00 0.00 0.00 0.00 0.00 
 0.00 0.00 2.00 68.00 0.00 0.00 0.00 0.00 
 1.00 2.00 3.00 36.00 0.00 0.00 0.00 0.00 
 0.00 0.00 5.00 35.00 0.00 0.00 0.00 0.00 
 0.00 1.00 9.00 49.00 0.00 0.00 0.00 0.00 
 0.00 0.00 5.00 132.00 0.00 0.00 0.00 0.00 
 0.00 0.00 8.00 127.00 0.00 0.00 0.00 0.00 
 0.00 0.00 3.00 127.00 0.00 0.00 0.00 0.00 
Means 12XXX  - 13XXX ( in 3: ) NOT BEST

-------------- 1 4 42 913 12710(1) 
----------------------18556 0:1.0[3.0]300%   1:0.4[0.5]122%   2:0.4[0.1]23%   3:0.9[0.5]52%   4:1.3(1)   
2---------------------18556 0:2.5[4.3]173% - 1:0.2[0.4]173%-- 2:0.4[0.1]35% - 3:0.4[0.1]14%+* Z3:14556 - 13556
2---------------------18556 0:0.0[0.0]inf% - 1:0.5[0.5]100%-- 2:0.4[0.0] 0% - 3:1.4[0.1] 7%+* Z3:14556 
 0.00 1.00 5.00 150.00 1271.00 0.00 0.00 0.00 
 0.00 0.00 4.00 134.00 0.00 0.00 0.00 0.00 
 0.00 1.00 2.00 150.00 0.00 0.00 0.00 0.00 
 0.00 0.00 6.00 68.00 0.00 0.00 0.00 0.00 
 0.00 0.00 4.00 36.00 0.00 0.00 0.00 0.00 
 1.00 0.00 5.00 35.00 0.00 0.00 0.00 0.00 
 0.00 0.00 4.00 49.00 0.00 0.00 0.00 0.00 
 0.00 1.00 4.00 37.00 0.00 0.00 0.00 0.00 
 0.00 0.00 4.00 127.00 0.00 0.00 0.00 0.00 
 0.00 1.00 4.00 127.00 0.00 0.00 0.00 0.00 
 Means 13XXX - 14XXX ( in 3: ) CORRECT 

-------------- 2 10 38 822 12710(1) 
----------------------19229 0:2.0[4.0]200%   1:1.0[0.4]45%   2:0.4[0.3]67%   3:0.8[0.5]59%   4:1.3(1)   
2---------------------19229 0:5.0[5.0]100% - 1:1.0[0.0] 0% - 2:0.4[0.3]83%-- 3:0.4[0.1]14%+* Z3:15229 
2---------------------19229 0:0.0[0.0]inf% - 1:1.0[0.7]71% - 2:0.3[0.2]52%-- 3:1.3[0.3]27%+* Z3:15229 
 0.00 1.00 2.00 150.00 1271.00 0.00 0.00 0.00 
 0.00 1.00 9.00 134.00 0.00 0.00 0.00 0.00 
 0.00 1.00 2.00 150.00 0.00 0.00 0.00 0.00 
 0.00 2.00 6.00 68.00 0.00 0.00 0.00 0.00 
 0.00 0.00 4.00 36.00 0.00 0.00 0.00 0.00 
 0.00 1.00 3.00 35.00 0.00 0.00 0.00 0.00 
 0.00 1.00 1.00 49.00 0.00 0.00 0.00 0.00 
 1.00 1.00 7.00 37.00 0.00 0.00 0.00 0.00 
 1.00 1.00 3.00 36.00 0.00 0.00 0.00 0.00 
 0.00 1.00 1.00 127.00 0.00 0.00 0.00 0.00 
 Means 14XXX - 15XXX( in 3: ) wrong
1h19



-------------- 1 4 34 320 3472 48280(1) 
----------------------183740 0:1.0[3.0]300%   1:0.4[0.5]122%   2:0.3[0.2]69%   3:0.3[0.0]14%   4:0.35[0.03] 8%   5:0.48(1)   
2---------------------183740 0:2.5[4.3]173% - 1:0.7[0.4]58%-- 2:0.3[0.3]94%-- 3:0.3[0.0]12%-- 4:0.32[0.03]10% - 
2---------------------183740 0:0.0[0.0]inf% - 1:0.2[0.4]173%-- 2:0.4[0.1]31%-- 3:0.3[0.0]15%-- 4:0.36[0.01] 2% - 
 0.00 1.00 4.00 28.00 364.00 4828.00 0.00 0.00 
 0.00 0.00 5.00 38.00 353.00 0.00 0.00 0.00 
 0.00 1.00 2.00 35.00 374.00 0.00 0.00 0.00 
 0.00 1.00 3.00 27.00 370.00 0.00 0.00 0.00 
 0.00 0.00 1.00 27.00 375.00 0.00 0.00 0.00 
 0.00 0.00 9.00 33.00 284.00 0.00 0.00 0.00 
 0.00 1.00 1.00 36.00 318.00 0.00 0.00 0.00 
 0.00 0.00 2.00 25.00 320.00 0.00 0.00 0.00 
 0.00 0.00 2.00 33.00 353.00 0.00 0.00 0.00 
 1.00 0.00 5.00 38.00 361.00 0.00 0.00 0.00 

--remove granit 183740 ---- 150007 -170000 ? 203215


in two regions : 
-------------- 1 2 33 350 3443 48280(1) 
----------------------199995 0:1.0[3.0]300%   1:0.2[0.4]200%   2:0.3[0.2]47%   3:0.3[0.1]17%   4:0.34[0.03] 8%   5:0.48(1)   
2---------------------199995 0:2.5[4.3]173% - 1:0.2[0.4]173%-- 2:0.3[0.1]54%-- 3:0.4[0.0]10%-- 4:0.31[0.02] 5%+* Z4:159995 
2---------------------199995 0:0.0[0.0]inf% - 1:0.2[0.4]173%-- 2:0.3[0.1]41%-- 3:0.3[0.1]16%-- 4:0.37[0.01] 2%+* Z4:159995 
 0.00 0.00 4.00 39.00 364.00 4828.00 0.00 0.00 
 0.00 0.00 1.00 29.00 353.00 0.00 0.00 0.00 
 0.00 1.00 4.00 25.00 374.00 0.00 0.00 0.00 
 0.00 0.00 3.00 33.00 370.00 0.00 0.00 0.00 
 1.00 0.00 4.00 29.00 375.00 0.00 0.00 0.00 
 0.00 1.00 3.00 32.00 284.00 0.00 0.00 0.00 
 0.00 0.00 5.00 40.00 318.00 0.00 0.00 0.00 
 0.00 0.00 2.00 41.00 320.00 0.00 0.00 0.00 
 0.00 0.00 1.00 42.00 324.00 0.00 0.00 0.00 
 0.00 0.00 6.00 40.00 361.00 0.00 0.00 0.00 
-------------- 1 3 30 340 3422 41250(2) 
----------------------200016 0:1.0[3.0]300%   1:0.3[0.5]153%   2:0.3[0.1]42%   3:0.3[0.1]17%   4:0.34[0.03] 8%   5:0.4[0.1]17%(2)   
2---------------------200016 0:2.5[4.3]173% - 1:0.2[0.4]173%-- 2:0.3[0.1]54%-- 3:0.4[0.0]13% * 4:0.33[0.01] 3%+* Z4:160016 
2---------------------200016 0:0.0[0.0]inf% - 1:0.5[0.5]100%-- 2:0.3[0.1]41%-- 3:0.3[0.0]10% * 4:0.37[0.01] 2%+* Z4:160016 
 0.00 0.00 4.00 39.00 364.00 4828.00 0.00 0.00 
 0.00 0.00 1.00 29.00 353.00 3422.00 0.00 0.00 
 0.00 1.00 4.00 25.00 374.00 0.00 0.00 0.00 
 0.00 0.00 3.00 33.00 370.00 0.00 0.00 0.00 
 0.00 0.00 4.00 29.00 375.00 0.00 0.00 0.00 
 1.00 1.00 3.00 32.00 284.00 0.00 0.00 0.00 
 0.00 0.00 5.00 40.00 318.00 0.00 0.00 0.00 
 0.00 0.00 2.00 41.00 320.00 0.00 0.00 0.00 
 0.00 0.00 1.00 42.00 324.00 0.00 0.00 0.00 
 0.00 1.00 3.00 30.00 340.00 0.00 0.00 0.00 

 another one : 
 
 -------------- 2 6 19 322 3322 41250(2) 
----------------------243215 0:2.0[4.0]200%   1:0.6[0.8]133%   2:0.2[0.2]80%   3:0.32[0.04]13%   4:0.33[0.02] 7%   5:0.4[0.1]17%(2)   
2---------------------243215 0:2.5[4.3]173% - 1:1.2[0.8]66% * 2:0.2[0.2]137%-- 3:0.29[0.04]14%-- 4:0.34[0.01] 2%+* Z4:203215 
2---------------------243215 0:0.0[0.0]inf% - 1:0.0[0.0]inf% * 2:0.2[0.1]37%-- 3:0.32[0.02] 6%-- 4:0.31[0.02] 5%+* Z4:203215 
 0.00 1.00 0.00 35.00 341.00 4828.00 0.00 0.00 
 0.00 1.00 5.00 30.00 332.00 3422.00 0.00 0.00 
 0.00 0.00 3.00 23.00 340.00 0.00 0.00 0.00 
 0.00 0.00 3.00 38.00 348.00 0.00 0.00 0.00 
 1.00 0.00 1.00 30.00 375.00 0.00 0.00 0.00 
 1.00 0.00 3.00 33.00 284.00 0.00 0.00 0.00 
 0.00 0.00 2.00 35.00 318.00 0.00 0.00 0.00 
 0.00 2.00 1.00 31.00 320.00 0.00 0.00 0.00 
 0.00 2.00 1.00 37.00 324.00 0.00 0.00 0.00 
 0.00 0.00 0.00 30.00 340.00 0.00 0.00 0.00 


next day 17:00
-------------- 1 7 32 312 3020 33691(7) 
----------------------741831 0:1.0[3.0]300%   1:0.7[0.6]91%   2:0.3[0.2]48%   3:0.31[0.02] 8%   4:0.302[0.010]3.3%   5:0.3[0.1]18%(7)   
2---------------------741831 0:2.5[4.3]173% - 1:0.7[0.8]111%-- 2:0.4[0.1]18%+* 3:0.32[0.02] 7%-- 4:0.305[0.010]3.4%-- Z2:741431 
2---------------------741831 0:0.0[0.0]inf% - 1:0.7[0.4]58%-- 2:0.2[0.1]37%+* 3:0.31[0.03] 9%-- 4:0.301[0.005]1.5%-- Z2:741431 
 1.00 1.00 3.00 32.00 311.00 4828.00 0.00 0.00 
 0.00 2.00 2.00 30.00 287.00 3422.00 0.00 0.00 
 0.00 0.00 1.00 29.00 311.00 3322.00 0.00 0.00 
 0.00 1.00 6.00 27.00 311.00 3085.00 0.00 0.00 
 0.00 1.00 4.00 34.00 283.00 3039.00 0.00 0.00 
 0.00 1.00 3.00 33.00 299.00 2935.00 0.00 0.00 
 0.00 1.00 5.00 32.00 297.00 2953.00 0.00 0.00 
 0.00 0.00 4.00 29.00 300.00 0.00 0.00 0.00 
 0.00 0.00 1.00 31.00 309.00 0.00 0.00 0.00 
 0.00 0.00 3.00 35.00 312.00 0.00 0.00 0.00 


at 17h24  : ADD uranium
 0.00 0.00 1.00 32.00 309.00 0.00 0.00 0.00 
 0.00 0.00 3.00 25.00 312.00 0.00 0.00 0.00 
-------------- 2 2 26 283 3044 33691(7) 
----------------------762950 0:2.0[4.0]200%   1:0.2[0.6]300%   2:0.3[0.1]43%   3:0.3[0.1]19%   4:0.304[0.008]2.5%   5:0.3[0.1]18%(7)   
2---------------------762950 0:2.5[4.3]173% - 1:0.5[0.9]173% - 2:0.2[0.1]47% - 3:0.2[0.0]18%-- 4:0.307[0.004]1.4%-- 
2---------------------762950 0:0.0[0.0]inf% - 1:0.0[0.0]inf% - 2:0.3[0.1]33% - 3:0.3[0.0]16%-- 4:0.308[0.005]1.5%-- 
 0.00 0.00 2.00 20.00 311.00 4828.00 0.00 0.00 
 0.00 0.00 4.00 22.00 287.00 3422.00 0.00 0.00 
 0.00 0.00 4.00 35.00 311.00 3322.00 0.00 0.00 
 0.00 0.00 2.00 33.00 311.00 3085.00 0.00 0.00 
 0.00 2.00 4.00 30.00 305.00 3039.00 0.00 0.00 
 1.00 0.00 1.00 21.00 301.00 2935.00 0.00 0.00 
 0.00 0.00 3.00 31.00 297.00 2953.00 0.00 0.00 
 0.00 0.00 2.00 34.00 300.00 0.00 0.00 0.00 
 0.00 0.00 1.00 32.00 309.00 0.00 0.00 0.00 
 1.00 0.00 3.00 25.00 312.00 0.00 0.00 0.00 

762950 precision ? prob 7630??
Z2:763567 -   500 = 7630XX  
Z3:767135 -  5000 = 762XXX  should be best not sure
Z3:768624 -  5000 = 763XXX  
Z4:813072 - 50000 = 76XXXX


Z2:763567 -   400 = 7631XX  

Z3:766276 -  4000 = 762XXX  (not best)
Z3:767135 -  4000 = 763XXX  should be best not sure
Z3:768624 -  4000 = 764XXX  

Z4:813072 - 40000 = 77XXXX

Z3 not best 
2---------------------766276 0:2.5[4.3]173% - 1:0.7[0.8]111% - 2:2.0[0.2]11% - 3:1.7[0.8]49%+* 4:0.307[0.004]1.4%-- Z3:762276 
2---------------------766276 0:0.0[0.0]inf% - 1:2.5[1.7] 66% - 2:1.8[0.2]10% - 3:0.3[0.1]20%+* 4:0.308[0.005]1.5%-- Z3:762276 
 
Z3  best or next 
2---------------------767135 0:2.5[4.3]173% - 1:2.7[1.1]40%-- 2:2.20[0.24]11%-- 3:2.11[0.09] 4%+* 4:0.307[0.004]1.4%-- Z3:763135 
2---------------------767135 0:0.0[0.0]inf% - 1:2.7[0.4]16%-- 2:2.08[0.31]15%-- 3:0.25[0.05]18%+* 4:0.308[0.005]1.5%-- Z3:763135 
 
2---------------------768624 0:2.5[4.3]173%-- 1:3.0[1.2]41% - 2:2.53[0.11] 4% * 3:2.09[0.06] 3%+* 4:0.307[0.004]1.4%-- Z3:764624 
2---------------------768624 0:2.5[4.3]173%-- 1:1.7[0.4]25% - 2:2.25[0.11] 5% * 3:0.23[0.03]11%+* 4:0.308[0.005]1.5%-- Z3:764624 


Z2: 
 
-------------- 3 24 123 275 3044 33691(7) 
----------------------763567 0:3.0[4.6]153%   1:2.4[1.6]65%   2:1.2[1.0]84%   3:0.3[0.0]18%   4:0.304[0.008]2.5%   5:0.3[0.1]18%(7)   
2---------------------763567 0:5.0[5.0]100% - 1:2.7[1.5]54%-- 2:2.3[0.3]11%+* 3:0.2[0.0]11% - 4:0.307[0.004]1.4%-- Z2:763167 
2---------------------763567 0:0.0[0.0]inf% - 1:1.5[1.5]100%-- 2:0.3[0.1]45%+* 3:0.3[0.0]17% - 4:0.308[0.005]1.5%-- Z2:763167 
 0.00 0.00 18.00 20.00 311.00 4828.00 0.00 0.00 
 0.00 3.00 25.00 22.00 287.00 3422.00 0.00 0.00 
 0.00 5.00 25.00 27.00 311.00 3322.00 0.00 0.00 
 1.00 3.00 19.00 33.00 311.00 3085.00 0.00 0.00 
 0.00 2.00 25.00 30.00 305.00 3039.00 0.00 0.00 
 0.00 1.00 1.00 21.00 301.00 2935.00 0.00 0.00 
 1.00 4.00 3.00 31.00 297.00 2953.00 0.00 0.00 
 1.00 1.00 2.00 34.00 300.00 0.00 0.00 0.00 
 0.00 4.00 1.00 32.00 309.00 0.00 0.00 0.00 
 0.00 1.00 4.00 25.00 312.00 0.00 0.00 0.00 

Z3 first not best 
00 34.00 300.00 0.00 0.00 0.00 
 0.00 3.00 19.00 32.00 309.00 0.00 0.00 0.00 
 0.00 1.00 24.00 25.00 312.00 0.00 0.00 0.00 
-------------- 1 18 203 826 3044 33691(7) 
----------------------766276 0:1.0[3.0]300%   1:1.8[1.5]82%   2:2.0[0.3]16%   3:0.8[0.8]103%   4:0.304[0.008]2.5%   5:0.3[0.1]18%(7)   
2---------------------766276 0:2.5[4.3]173% - 1:0.7[0.8]111% - 2:2.0[0.2]11% - 3:1.7[0.8]49%+* 4:0.307[0.004]1.4%-- Z3:762276 
2---------------------766276 0:0.0[0.0]inf% - 1:2.5[1.7]66% - 2:1.8[0.2]10% - 3:0.3[0.1]20%+* 4:0.308[0.005]1.5%-- Z3:762276 
 0.00 5.00 19.00 20.00 311.00 4828.00 0.00 0.00 
 0.00 1.00 19.00 22.00 287.00 3422.00 0.00 0.00 
 0.00 3.00 27.00 27.00 311.00 3322.00 0.00 0.00 
 0.00 0.00 20.00 225.00 311.00 3085.00 0.00 0.00 
 0.00 0.00 15.00 200.00 305.00 3039.00 0.00 0.00 
 1.00 1.00 18.00 210.00 301.00 2935.00 0.00 0.00 
 0.00 2.00 19.00 31.00 297.00 2953.00 0.00 0.00 

Z3 correct or next...
----------------------767135 0:1.0[3.0]300%   1:2.6[0.8]31%   2:2.13[0.27]13%   3:1.00[0.90]90%   4:0.304[0.008]2.5%   5:0.3[0.1]18%(7)   
2---------------------767135 0:2.5[4.3]173% - 1:2.7[1.1]40%-- 2:2.20[0.24]11%-- 3:2.11[0.09] 4%+* 4:0.307[0.004]1.4%-- Z3:763135 
2---------------------767135 0:0.0[0.0]inf% - 1:2.7[0.4]16%-- 2:2.08[0.31]15%-- 3:0.25[0.05]18%+* 4:0.308[0.005]1.5%-- Z3:763135 
 0.00 4.00 24.00 20.00 311.00 4828.00 0.00 0.00 
 0.00 3.00 19.00 22.00 287.00 3422.00 0.00 0.00 
 0.00 3.00 16.00 27.00 311.00 3322.00 0.00 0.00 
 0.00 2.00 20.00 225.00 311.00 3085.00 0.00 0.00 
 1.00 3.00 23.00 200.00 305.00 3039.00 0.00 0.00 
 0.00 3.00 24.00 210.00 301.00 2935.00 0.00 0.00 
 0.00 2.00 23.00 208.00 297.00 2953.00 0.00 0.00 
 0.00 3.00 22.00 34.00 300.00 0.00 0.00 0.00 


-------------- 2 25 234 1187 3044 33691(7) 
----------------------768624 0:2.0[4.0]200%   1:2.5[1.4]54%   2:2.34[0.24]10%   3:1.19[0.94]79%   4:0.304[0.008]2.5%   5:0.3[0.1]18%(7)   
2---------------------768624 0:2.5[4.3]173%-- 1:3.0[1.2]41% - 2:2.53[0.11] 4% * 3:2.09[0.06] 3%+* 4:0.307[0.004]1.4%-- Z3:764624 
2---------------------768624 0:2.5[4.3]173%-- 1:1.7[0.4]25% - 2:2.25[0.11] 5% * 3:0.23[0.03]11%+* 4:0.308[0.005]1.5%-- Z3:764624 
 0.00 2.00 22.00 20.00 311.00 4828.00 0.00 0.00 
 0.00 2.00 25.00 22.00 287.00 3422.00 0.00 0.00 
 0.00 1.00 25.00 27.00 311.00 3322.00 0.00 0.00 
 1.00 2.00 24.00 225.00 311.00 3085.00 0.00 0.00 
 0.00 2.00 25.00 200.00 305.00 3039.00 0.00 0.00 
 0.00 1.00 27.00 210.00 301.00 2935.00 0.00 0.00 
 0.00 2.00 18.00 208.00 297.00 2953.00 0.00 0.00 
 1.00 5.00 21.00 218.00 300.00 0.00 0.00 0.00 
 0.00 5.00 23.00 32.00 309.00 0.00 0.00 0.00 

Z4 before best:

-------------- 4 21 250 2218 9999 41978(8) 
----------------------801190 0:4.0[4.9]122%   1:2.1[1.2]58%   2:2.5[0.5]19%   3:2.22[0.12] 5%   4:1.00[0.87]87%   5:0.42[0.23]54%(8)   
2---------------------801190 0:5.0[5.0]100%-- 1:2.2[1.1]48%-- 2:2.7[0.5]18%-- 3:2.27[0.09] 4%-- 4:2.04[0.28]14%+* Z4:761190 
2---------------------801190 0:5.0[5.0]100%-- 1:2.5[1.1]45%-- 2:2.5[0.4]16%-- 3:2.22[0.06] 3%-- 4:0.30[0.01] 3%+* Z4:761190 
 0.00 2.00 29.00 236.00 311.00 4828.00 0.00 0.00 
 0.00 3.00 19.00 225.00 287.00 3422.00 0.00 0.00 
 1.00 1.00 23.00 228.00 311.00 3322.00 0.00 0.00 
 0.00 4.00 22.00 213.00 311.00 3085.00 0.00 0.00 
 1.00 0.00 24.00 220.00 305.00 3039.00 0.00 0.00 
 0.00 1.00 32.00 229.00 301.00 2935.00 0.00 0.00 
 0.00 2.00 21.00 194.00_1559.00 2953.00 0.00 0.00 
 1.00 2.00 25.00 237.00 2229.00 9999.00 0.00 0.00 
 0.00 4.00 34.00 219.00 2188.00 0.00 0.00 0.00 
 1.00 2.00 21.00 217.00 2197.00 0.00 0.00 0.00 

Correct one Excellent contrast....
-------------- 2 30 212 2200 11901 41978(8) 
----------------------818925 0:2.0[4.0]200%   1:3.0[1.5]49%   2:2.1[0.5]23%   3:2.20[0.13] 6%   4:1.19[0.91]76%   5:0.42[0.23]54%(8)   
2---------------------818925 0:2.5[4.3]173% - 1:3.2[1.3]40%-- 2:1.9[0.5]24%-- 3:2.23[0.12] 5%-- 4:2.21[0.02] 1%+* Z4:778925 
2---------------------818925 0:0.0[0.0]inf% - 1:2.5[1.8]72%-- 2:2.3[0.5]21%-- 3:2.22[0.12] 5%-- 4:0.31[0.00] 1%+* Z4:778925 
 0.00 4.00 19.00 230.00 2213.00 4828.00 0.00 0.00 
 0.00 4.00 29.00 202.00 287.00 3422.00 0.00 0.00 
 0.00 4.00 17.00 227.00 311.00 3322.00 0.00 0.00 
 0.00 5.00 25.00 216.00 311.00 3085.00 0.00 0.00 
 1.00 2.00 19.00 235.00 305.00 3039.00 0.00 0.00 
 1.00 0.00 19.00 211.00 301.00 2935.00 0.00 0.00 
 0.00 3.00 21.00 213.00 1559.00 2953.00 0.00 0.00 
 0.00 3.00 12.00 235.00 2229.00 9999.00 0.00 0.00 
 0.00 1.00 25.00 200.00 2188.00 0.00 0.00 0.00 
 0.00 4.00 26.00 231.00 2197.00 0.00 0.00 0.00 

    */
    Serial.print(" ");
    int n = 1;
    int np = 0;
    for (size_t i = 0; i < fNumberPositonsInPulseArray; i++) {
      if (fPulseNbv[i] > 0) {

        if (fPulseNbv[i] > 1 && fStdDev[i] > 0.001) {
          if (log(fAverage[i] / fStdDev[i]) > 1.0) {
            n = (int)(log(fAverage[i] / fStdDev[i])) + 0;
            np = (int)(log(fAverage[i] / fStdDev[i])) - 2;
            if (np < 0) {
              np = 0;
            }
          }
        }
        Serial.print((long)(i));
        Serial.print(":");
        Serial.print(String(fAverage[i], n));
        if (fPulseNbv[i] > 1) {
          Serial.print("[");
          Serial.print(String(fStdDev[i], n));
          Serial.print("]");
          Serial.print(String(100.0 * fStdDev[i] / fAverage[i], np));
          Serial.print("%");
        }
        if (fPulseNbv[i] < 10) {
          Serial.print("(");
          Serial.print((long)(fPulseNbv[i]));
          Serial.print(")");
        }
        Serial.print("   ");
      }
    }
    Serial.print("\n");

    // trwo half statistics

    if (true) {
      int n = 1;
      int np = 0;
      for (size_t zIc = 0; zIc != 2; zIc++) {
        String endLineString = "";

        Serial.print("2---------------------");
        Serial.print((long)(i0));

        Serial.print(" ");
        int n = 1;
        int np = 0;

        for (size_t i = 0; i < fNumberPositonsInPulseArray; i++) {
          const bool c1 = !isWithin(fAverage[i], fTwoHalfAvStd[6 * i + 0],
                                    fTwoHalfAvStd[6 * i + 2]);
          const bool c2 = !isWithin(fAverage[i], fTwoHalfAvStd[6 * i + 1],
                                    fTwoHalfAvStd[6 * i + 3]);
          const bool c3 =
              !isWithin(fTwoHalfAvStd[6 * i + 0], fTwoHalfAvStd[6 * i + 1],
                        fTwoHalfAvStd[6 * i + 3]);
          const bool c4 =
              !isWithin(fTwoHalfAvStd[6 * i + 1], fTwoHalfAvStd[6 * i + 0],
                        fTwoHalfAvStd[6 * i + 2]);
          const bool c5 =
              isDisjoint(fTwoHalfAvStd[6 * i + 1], 
              fTwoHalfAvStd[6 * i + 3], fTwoHalfAvStd[6 * i + 0],
                        fTwoHalfAvStd[6 * i + 2]);
          const bool c6 =
              isDisjoint(fTwoHalfAvStd[6 * i + 1], 
              fTwoHalfAvStd[6 * i + 3], fTwoHalfAvStd[6 * i + 0],
                        fTwoHalfAvStd[6 * i + 2]);
          if (fPulseNbv[i] == 10 && fStdDev[i] > 0.001) {
            if (log(fAverage[i] / fStdDev[i]) > 1.0) {
              n = (int)(log(fAverage[i] / fStdDev[i])) + 0;
              np = (int)(log(fAverage[i] / fStdDev[i])) - 2;
              if (np < 0) {
                np = 0;
              }
            }

            Serial.print((long)(i));
            Serial.print(":");
            Serial.print(String(fTwoHalfAvStd[6 * i + 0 + zIc], n));
            if (fPulseNbv[i] > 1) {
              Serial.print("[");
              Serial.print(String(fTwoHalfAvStd[6 * i + 2 + zIc], n));
              Serial.print("]");
              Serial.print(String(100.0 * fTwoHalfAvStd[6 * i + 2 + zIc] /
                                      fTwoHalfAvStd[6 * i + 0 + zIc],
                                  np));
              Serial.print("%");
            }
            if (c1 && c2) {
              Serial.print("+");
              }
            if (c1 != c2) {
              Serial.print(" ");
              }
            if (!(c1 || c2)) {
              Serial.print("-");
              }
            if (c5){
              Serial.print("*");
            } else {       
              Serial.print("-");
            }
            if (c5 && (c1 && c2)) {
              unsigned long long longSubstract = 1ull;
              if (i == 1) {longSubstract = 10ull;}
              if (i == 2) {longSubstract = 100ull;}
              if (i == 3) {longSubstract = 1000ull;}
              if (i == 4) {longSubstract = 10000ull;}
              if (i == 5) {longSubstract = 100000ull;}
              if (i == 6) {longSubstract = 1000000ull;}
              if (i == 7) {longSubstract = 10000000ull;}       
              if (i == 8) {longSubstract = 100000000ull;}       
              if (i == 9) {longSubstract = 1000000000ull;}       
              if(i == 10) {longSubstract = 10000000000ull;}       

              endLineString += "X" + String((long)(i)) + ":" +  String((long)(i0 - 5 * longSubstract)) + " ";
            }
            Serial.print(" ");
          }
        }
        Serial.print(endLineString);
        Serial.print("\n");
      }



for (size_t zIc = 0; zIc != 10; zIc++) {
        Serial.print(" ");
        for (size_t i = 0; i < fNumberPositonsInPulseArray; i++) {
            Serial.print((float)fPulseArray[zIc  + i * 10]);
            Serial.print(" ");
          }
        Serial.print("\n");
    }
  }
  }
}


bool GeigerCounter::isWithin(float a, float b, float c) {
  return ((a > (b - c)) && (a < (b + c)));
}

bool GeigerCounter::isDisjoint(float a, float ae, float b, float c) {
  if (a > b) {
    return ((a - ae) > (b + c));
  } else {
    return ((a + ae) < (b - c));
  }
}

void GeigerCounter::pushPulses(unsigned long long i0,
                               const unsigned long numberHits,
                               const bool incrementPosition) {
  if (fUseSerial && false) {
    Serial.print("--");

    Serial.print(" i0 :");
    Serial.print((long)i0);
    if (incrementPosition)
      Serial.print(" +");

    Serial.print("\n");
  }
  for (size_t mainIndex = 0; mainIndex != fNumberPositonsInPulseArray;
       mainIndex++) {

    const bool incdePosOnlyFirst = (mainIndex == 0) ? incrementPosition : false;
    const unsigned int curIndex = i0 % 10;

    const unsigned long value =
        (mainIndex == 0) ? numberHits : fPulseSum[mainIndex - 1];

    setArray(mainIndex, curIndex, value, incdePosOnlyFirst);

    if (curIndex != 9) {
      break;
    }

    i0 -= curIndex;
    i0 /= 10;
  }
}

void GeigerCounter::setArray(const size_t a, const unsigned int b,
                             const unsigned long value,
                             const bool incrementPosition) {

  if (b >= 10) {
    return;
    }
  if (a >= fNumberPositonsInPulseArray) {
    return;
    }
  const size_t ind = a * 10 + b;
  if (ind >= 10 * fNumberPositonsInPulseArray) {
    return;
    }

  if (incrementPosition) {
    fPulseArray[ind] += value;
  } else {
    fPulseSum[a] -= fPulseArray[ind];
    fPulseArray[ind] = value;
    if (fPulseNbv[a] < 10) {
      fPulseNbv[a]++;
    }
  }
  fPulseSum[a] += value;

  // calc av and stderr

  float average = float(fPulseSum[a]) / 10.0f;
  if (fPulseNbv[a] < 10 && fPulseNbv[a] > 0) {
    average = float(fPulseSum[a]) / ((float)fPulseNbv[a]);
  }

  float stdErr = 0.0;
  if (fPulseNbv[a] > 0) {
    for (size_t i = 0; i < fPulseNbv[a]; i++) {
      stdErr += ((float)fPulseArray[a * 10 + i] - average) *
                ((float)fPulseArray[a * 10 + i] - average);
    }
    stdErr = sqrt(stdErr / ((float)fPulseNbv[a]));
  }

  const float factor = (float)fFirstDigitFactor * pow(10.0f, ((float)a - 3.0f));
  fStdDev[a] = stdErr / factor;
  fAverage[a] = average / factor;

  // calculation of av and stderr over two half blocks
  if (fPulseNbv[a] == 10) {
    float average1 = 0.0f;
    float average2 = 0.0f;
    float stdErr1 = 0.0f;
    float stdErr2 = 0.0f;
    for (size_t i = 0; i != 9; i++) { // only 8 values skip 4
      const size_t iMod = a * 10 + ((10 - i + b) % 10);
      if (i < 4) {
        average1 += (float)fPulseArray[iMod];
      }
      if (i > 4) {
        average2 += (float)fPulseArray[iMod];
      }
    }
    average1 /= 4.f;
    average2 /= 4.f;
    for (size_t i = 0; i != 9; i++) { // only 8 values skip 4
      const size_t iMod = a * 10 + ((10 - i + b) % 10);
      if (i < 4) {
        stdErr1 += ((float)fPulseArray[iMod] - average1) *
                   ((float)fPulseArray[iMod] - average1);
      }
      if (i > 4) {
        stdErr2 += ((float)fPulseArray[iMod] - average2) *
                   ((float)fPulseArray[iMod] - average2);
      }
    }
    stdErr1 = sqrt(stdErr1 / 4.f);
    stdErr2 = sqrt(stdErr2 / 4.f);
    fTwoHalfAvStd[a * 6 + 0] = average1 / factor;
    fTwoHalfAvStd[a * 6 + 1] = average2 / factor;
    fTwoHalfAvStd[a * 6 + 2] = stdErr1 / factor;
    fTwoHalfAvStd[a * 6 + 3] = stdErr2 / factor;
    // fTwoHalfAvStd[a * 6 + 4] = fPulseArray[a * 10 + b];
  }
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
    divider = fNumberHitFromStart;
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