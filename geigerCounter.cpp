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
void GeigerCounter::pulse(const unsigned long long aMilis,
                          unsigned long numberPulses) {

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

  for (size_t i = 0; i != deltaBas; i++) {

    const unsigned long curNumberPulses =
        ((i + 1) == deltaBas) ? longavNumberPulses + longreNumberPulses
                              : longavNumberPulses;

    pushPulses(fai0 + i, curNumberPulses);
  }
  if (deltaBas == 0) {
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

    /* fFirstDigitFactor(10)
    -------------- 1 0 24 208 2080(1)
    ---------------------- 10.00[30.00]299%
    0.00[0.00]2147483647% 2.40[0.92]38% 2.08[0.49]23% 2.08[0.00]0%(1)
    fFirstDigitFactor(100)
    ---------------------  3.00[4.58]152% 2.00[1.55]77% 2.25[0.50]22% 2.25[0.00]0%(1)
    ---------------------- 2.00[4.00]200% 2.10[0.70]33% 2.19[0.35]15% 2.25[0.00]0%(1)
    ---------------------- 3.00[4.58]152% 2.10[1.14]54% 2.32[0.46]19% 2.26[0.20]8%(8)
    ---------------------- 2.00[4.00]200% 2.60[1.28]49% 2.06[0.46]22% 2.24[0.18]8%
    2.24[0.00]0%(1)
    ---------------------- 2.00[4.00]200% 2.30[0.78]33% 2.20[0.38]17% 2.26[0.19]8%
    2.24[0.00]0%(1)


    ---------------------- 2.00[4.00]200% 2.30[0.78]33% 2.20[0.38]17% 2.26[0.19]8%
    2.24[0.00]0%(1)

    -------------- 3 24 213 2176 21781 218670(1)
    ---------------------- 3.00[4.58]152% 2.40[1.62]67% 2.13[0.34]15% 2.18[0.09]4%
    2.18[0.03]1% 2.19[0.00]0%(1)

    -------------- 3 19 212 2134 21907 218960(3)
    ---------------------- 3.00[4.58]152% 1.90[0.54]28% 2.12[0.26]12% 2.13[0.13]6%
    2.19[0.05]2% 2.19[0.01]0%(3)

    -------------- 4 24 237 2163 21503 216992 2173760(1)
    ---------------------- 4.00[4.90]122% 2.40[0.66]27% 2.37[0.43]18% 2.16[0.10]4%
    2.15[0.04]1% 2.17[0.01]0% 2.17[0.00]0%(1) Dumps better precision values...


    ---------------------- 3.0[4.6]153%
    0.4[0.5]122% 2.0[0.7]36% 2.090[0.081]3.9%(7)
    -------------- 1 6 203 2090(7)
    ---------------------- 1.0[3.0]300%
    0.6[0.7]111% 2.0[0.7]36% 2.090[0.081]3.9%(7)
    -------------- 1 0 186 2090(7)
    ---------------------- 1.0[3.0]300%
    0.0000000000[0.0000000000]inf% 1.8600000143[0.8452218771]45.4420360536% 2.090[0.081]3.9%(7)
    -------------- 1 1 161 2090(7)
    ---------------------- 1.0[3.0]300%
    0.1[0.3]300% 1.6[1.0]59% 2.090[0.081]3.9%(7)

    -------------- 4 22 224 2274 22266(8)
    ---------------------- 4.0[4.9]122% 2.2[1.6]73% 2.24[0.22]10% 2.274[0.083]3.6%
    2.23[0.15] 7%(8) 13h51m


    -------------- 2 30 245 2276 22378 223780(1)
    ---------------------- 2.0[4.0]200% 3.0[1.2]39% 2.45[0.31]13% 2.276[0.102]4.5%
    2.24[0.14] 6% 2.24(1)    14h14


    -------------- 1 20 219 2237 22474 224250(2)
    ---------------------- 1.0[3.0]300% 2.0[1.3]63% 2.2[0.3]15% 2.237[0.089]4.0% 2.2474[0.0380]1.69%
    2.242500[0.004700]0.2096%(2)  17:56
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
            if (c1 && c2)
              Serial.print("+");
            if (c1 != c2)
              Serial.print(" ");
            if (!(c1 || c2))
              Serial.print("-");
            if (c5)
              Serial.print("*");
          
else              Serial.print("-");

            Serial.print(" ");
          }
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

  if (b >= 10)
    return;
  if (a >= fNumberPositonsInPulseArray)
    return;
  const size_t ind = a * 10 + b;
  if (ind >= 10 * fNumberPositonsInPulseArray)
    return;

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
    float average1 = 0.0;
    float average2 = 0.0;
    float stdErr1 = 0.0;
    float stdErr2 = 0.0;
    for (unsigned int i = 0; i != 9; i++) { // only 8 values
      const unsigned int iMod = a * 10 + ((i + b) % 10);
      if (i < 4) {
        average1 += (float)fPulseArray[iMod];
      }
      if (i > 4) {
        average2 += (float)fPulseArray[iMod];
      }
    }
    average1 /= 4.f;
    average2 /= 4.f;
    for (unsigned int i = 0; i != 9; i++) { // only 8 values
      const unsigned int iMod = a * 10 + ((i + b) % 10);
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