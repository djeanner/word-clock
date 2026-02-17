/*
  // use pico board library by Earle F. Philhower, III 
*/
#include <Arduino.h> // add in header of classes
#include <time.h>
#include <TimeLib.h>
#include "hardware/timer.h"
#include "WordClock.h"

// set using as compiler option :  --build-property compiler.cpp.extra_flags="-DMILAN_CLOCK=1"
#if defined(MILAN_CLOCK)

// not using interupts for word clock is not displaying time
#define INTERRUPT_WORD_CLOCK 1
// both methods are implemented but have different requirements for compilation
// use of interrupts does not require a clock_control to be defined for DCF77Decoder  
#define CLOCK_CONTROL_INTERRUPT 1
#define SERIAL_DEBUG 0  // <<< set to 0 to disable ALL serial output NOTE: not functionning well with INTERRUPT_WORD_CLOCK

#else

// not using interupts for word clock is not displaying time
#define INTERRUPT_WORD_CLOCK 0
// both methods are implemented but have different requirements for compilation
// use of interrupts does not require a clock_control to be defined for DCF77Decoder  
#define CLOCK_CONTROL_INTERRUPT 0
#define SERIAL_DEBUG 1  // <<< set to 0 to disable ALL serial output NOTE: not functionning well with INTERRUPT_WORD_CLOCK

#endif


#define DEBUGINWORDCLOCK 1 // this is to disable a debugging feature in DCF77Decoder 

// board's led
#define LEDPIN LED_BUILTIN

// SERIAL DEBUG MACROS
#if SERIAL_DEBUG
#define DBG_BEGIN(x) Serial.begin(x)
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#else
#define DBG_BEGIN(x)
#define DBG_PRINT(x)
#define DBG_PRINTLN(x)
#endif

// GPIO for the analog input
#define RADIOINPUT 28
// minimal value of the level of the RADIOINPUT for goot 500 is good for 1.5 and 3.8 V receptors
#define MINVAL_ANTENNA 500

// delay_ms stops the clock of the CPU. not compatible with serial or other interrupt-based services
#if (INTERRUPT_WORD_CLOCK == 0) && (SERIAL_DEBUG == 0) && (CLOCK_CONTROL_INTERRUPT == 0)
  #define SLEEPORDELAYMS(ms) sleep_ms(ms)
#else
  #define SLEEPORDELAYMS(ms) delay(ms)
#endif

const bool debug8 = true;   // text
//txt:[+++++++----+-++++-+-+++---+---+----+---+----++--_---£-++++-+],Lms:276, 11:57 Mon Feb 2/2026 All
const bool debug9 = debug8;  // display long and short pulses on the fly
const bool debug5 = false;   // display long pause pulses on the fly
bool debug2 = !debug8;       // front display debugging steps
const bool debug3 = true;    // dump info about the validation process of times

// needs some macro  ---- dont move higher up
#include "ClockControl.h"
#include "DCF77Decoder.h"

#if CLOCK_CONTROL_INTERRUPT

// -------- interrupt function prototypes --------
bool clockControlAdjustCallback(repeating_timer_t *rt);
repeating_timer_t timerClockControlAdjust;
bool thereIsAtimerClockControlAdjust_running = false;
#endif // CLOCK_CONTROL_INTERRUPT

#ifndef CLOCK_CONTROL_INTERRUPT
#define CLOCK_CONTROL_INTERRUPT 0
#endif

WordClock theWordClock;
#if SERIAL_DEBUG
ClockControl theClockControl(10, debug3, true);
#else
ClockControl theClockControl(10, debug3, false);
#endif

#if CLOCK_CONTROL_INTERRUPT
bool clockControlAdjustCallback(repeating_timer_t *rt) {
  theClockControl.interruptAdjust();
  return true;  // keep repeating
}
#endif // CLOCK_CONTROL_INTERRUPT

// for DCF77Decoder header ... 
#ifndef CLOCK_CONTROL_INTERRUPT
#define CLOCK_CONTROL_INTERRUPT 1
#endif

#if INTERRUPT_WORD_CLOCK
// Interrups to control led brightness without flickering
repeating_timer_t timer10ms;
alarm_id_t alarmID;

// -------- interrupt function prototypes --------
bool timer10msCallback(repeating_timer_t *rt);  // 100 Hz to avoid flickering
int64_t alarmCallback(alarm_id_t id, void *user_data);

// -------- WordClock interrupt callback --------
// Called every 10 ms
bool timer10msCallback(repeating_timer_t *rt) {
  theWordClock.ocDriveLowAll_fullON();
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
  theWordClock.ocDriveLowAll_fullOFF();
  return 0;  // one-shot alarm
}
#endif // INTERRUPT_WORD_CLOCK

DCF77Decoder dcf77(RADIOINPUT, MINVAL_ANTENNA, debug2, debug5, debug8, debug9, LEDPIN);

void setup() {
#if INTERRUPT_WORD_CLOCK
  // interupt every 10000 us
  add_repeating_timer_us(
    -10000,  // negative = exact interval, no drift -10000: 100 Hz to avoid visible flickering
    timer10msCallback,
    NULL,
    &timer10ms);
#endif // INTERRUPT_WORD_CLOCK
  
if (debug2) theWordClock.debugSetHoursLeds(1);

  DBG_BEGIN(115200);
  delay(2000);

  analogReadResolution(12);

  DBG_PRINT("The DCF77 pin : ");
  DBG_PRINTLN(RADIOINPUT);

  // Start with everything released
  theWordClock.ocDriveLowAll_fullOFF();

  // setting up Led
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  delay(100);
  digitalWrite(LEDPIN, LOW);
  delay(200);
  digitalWrite(LEDPIN, HIGH);
  delay(300);
  digitalWrite(LEDPIN, LOW);

  DBG_PRINTLN("End setup");
  if (debug2) theWordClock.debugSetHoursLeds(2);
}

void loop() {
  if (debug2) theWordClock.debugSetHoursLeds(3);

  // Test each output led
  const bool testEachLedFirst = false;
  if (testEachLedFirst) { 
    theWordClock.testLed();
  }

  for (long superLoop = 0; superLoop < 100000000; superLoop++) {
    // Main listener : returns when have recieved valid time/date. May last minutes.
    DBG_PRINTLN("Start listening to dcf77 signal ...");

    dcf77.reset();
    dcf77.initListen();
    int lastMinL1 = minute(now());
    for (long fastLoop = 0; fastLoop < 1000000000; fastLoop ++) {
      // fast loop 
      const int isTimeValid = dcf77.listen(theClockControl);
      if (isTimeValid == 1) {
        theClockControl.storeTime(dcf77.getTM());
        debug2 = false;  // stop
        break;
      }
      time_t t = now();
      const int curMin = minute(t);
      if (lastMinL1 != curMin) {
        lastMinL1 = curMin;
        theWordClock.setWordClock(curMin, hour(t), theClockControl.isReliable());
      }
    }
  
    // will sleep/delay for about a minute when nothing happens and not listening to dcf77
    unsigned long int numberMinStaysInLoop = theClockControl.isReliable() ? 180 : 10; // 20 : 5; //
    DBG_PRINT("Stop listening to dcf77 for ");
    DBG_PRINT(numberMinStaysInLoop);
    DBG_PRINTLN(" min.");
    time_t t = now();
    theWordClock.setWordClock(minute(t), hour(t), theClockControl.isReliable());
    int lastMin = minute(t);
    for (unsigned long long loo = 0UL; loo < 1000000000; loo++) {
      t = now();
      int curMin = minute(t);
      // when minute changes
      if (curMin != lastMin) {
        lastMin = curMin;
        // perturb time every minute.. to see how manages
        //
        //
        // setTime(now() + 1);
        //

        // refine time if cristal not precise enough
#if !CLOCK_CONTROL_INTERRUPT
        theClockControl.adjustTimeMinute(millis() / 60000LL);
#endif // !CLOCK_CONTROL_INTERRUPT
        digitalWrite(LEDPIN, ((curMin % 2) == 0) ? LOW : HIGH);
        theWordClock.setWordClock(minute(t), hour(t), theClockControl.isReliable());
        numberMinStaysInLoop -= 1;
        if (numberMinStaysInLoop <= 0) break;
        SLEEPORDELAYMS(55000);  // waits for 55 sec
      }
    }
  }
}