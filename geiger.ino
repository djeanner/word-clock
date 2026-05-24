#include "StringWindow.h"
#include "TFT_Screen.h"

// SPI and pins
#define SPI_RX 16
#define SPI_TX 19
#define SPI_CLK 18
#define TFT_CS 17
#define TFT_DC 20
#define TFT_RST 21
#define TOUCH_CS 26
#define TOUCH_IRQ 255

TFT_Screen screen(160, 128, SPI_RX, SPI_TX, SPI_CLK, TFT_CS, TFT_DC, TFT_RST,
                  TOUCH_CS, TOUCH_IRQ);

void setup() { screen.begin(false, 3); }

void loop() {
  bool demoMode = false;
  if (demoMode) {
    screen.update();
  } else {
    StringWindow win(
        &screen, 2, 2, 156, 15, 1, ST77XX_WHITE, ST77XX_BLACK,
        "          Scan the codes and support the action...          ",
        ST77XX_GREEN, 1);
    win.draw();



    const int The variable `maxL` in the code snippet provided is being used as a constant to define the maximum value for the inner loop in the `loop()` function. It is set to a value of 500, and the inner loop runs `maxL` times for each iteration of the outer loop. This helps control the number of iterations and the duration of the execution of the code within the nested loops.
    The variable `maxL` in the code snippet is being used as a constant to define the maximum value for the inner loop in the `loop()` function. It is set to 500, and the inner loop runs `maxL` times in each iteration of the outer loop. This helps control the number of iterations and the duration of the loop execution.
    maxL = 500;
    long long pt = 0;
    for (int i = 0; i < 100000; i++) {
      for (int g = 0; g < maxL; g++) {
        win.drawShift(pt++);
      }
    }
  }
}


// this is to facilitate addition of wifi
/*
  // use pico board library by Earle F. Philhower, III 
#include <Arduino.h> // add in header of classes
#include <time.h>
#include <TimeLib.h>
#include "hardware/timer.h"

#include "ClockControl.h"
#include "DCF77Decoder.h"


#define SERIAL_DEBUG 1  // <<< set to 0 to disable ALL serial output NOTE: not functionning well with INTERRUPT_WORD_CLOCK
#define TFT_DISPLAY 1
#define WIFI_DCF77_DECODER 1

#if defined(WIFI_DCF77_DECODER)
#include "WifiControl.h"
#endif // defined(WIFI_DCF77_DECODER)

#if defined(TFT_DISPLAY)

#include <functional>

#include "TFT_Screen.h"
#include "DCF77Window.h"
#include "StringWindow.h"
// SPI and pins
#define SPI_RX   16
#define SPI_TX   19
#define SPI_CLK  18
#define TFT_CS   17
#define TFT_DC   20
#define TFT_RST  21
#define TOUCH_CS 26
#define TOUCH_IRQ 255

TFT_Screen screen(160, 128,
  SPI_RX, SPI_TX, SPI_CLK,
  TFT_CS, TFT_DC, TFT_RST,
  TOUCH_CS, TOUCH_IRQ
);
StringWindow win(&screen,
	                 2, 110, screen.getWidth() - 4, 16,
	                 1,
	                 ST77XX_WHITE,
	                 ST77XX_BLACK,
	                 "Initialize\n",
	                 ST77XX_GREEN,
	                 1);
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


#define SLEEPORDELAYMS(ms) delay(ms)

const bool debug8 = false;   // text
//txt:[+++++++----+-++++-+-+++---+---+----+---+----++--_---£-++++-+],Lms:276, 11:57 Mon Feb 2/2026 All
const bool debug9 = debug8;  // display long and short pulses on the fly
const bool debug5 = false;   // display long pause pulses on the fly
bool debug2 = !debug8;       // front display debugging steps
const bool debug3 = true;    // dump info about the validation process of times

#if defined(WIFI_DCF77_DECODER)
WifiControl server(80);
#endif // defined(TFT_DISPLAY)

#if defined(MILAN_CLOCK)
WordClock theWordClock;
#endif

#if SERIAL_DEBUG
ClockControl theClockControl(10, debug3, true);
#else
ClockControl theClockControl(10, debug3, false);
#endif

#if defined(WIFI_DCF77_DECODER)
const bool wantsAServer = true;
String wifiIP = "";
#else
const bool wantsAServer = false;
#endif // defined(WIFI_DCF77_DECODER)

#if SERIAL_DEBUG 
#if defined(WIFI_DCF77_DECODER)
DCF77Decoder dcf77(RADIOINPUT, MINVAL_ANTENNA, debug2, debug5, debug8, debug9, LEDPIN, true, wantsAServer);
#else
DCF77Decoder dcf77(RADIOINPUT, MINVAL_ANTENNA, debug2, debug5, debug8, debug9, LEDPIN, true);
#endif
#else
DCF77Decoder dcf77(RADIOINPUT, MINVAL_ANTENNA, debug2, debug5, debug8, debug9, LEDPIN, false);
#endif

#if defined(TFT_DISPLAY)       
DCF77Window theDCFwin(&screen,
                     16, 2, 126, 106,
                     1,
                     ST77XX_WHITE,
                     ST77XX_BLACK,
                     ST77XX_RED, 1, 2);
#endif // defined(TFT_DISPLAY)


void setup() {
#if INTERRUPT_WORD_CLOCK
  // interupt every 10000 us
  add_repeating_timer_us(
    -10000,  // negative = exact interval, no drift -10000: 100 Hz to avoid visible flickering
    timer10msCallback,
    NULL,
    &timer10ms);
#endif // INTERRUPT_WORD_CLOCK
  
  DBG_BEGIN(115200);
  delay(2000);

  analogReadResolution(12);

  DBG_PRINT("The DCF77 pin : ");
  DBG_PRINTLN(RADIOINPUT);

#if defined(TFT_DISPLAY)
  screen.begin();
	win.draw();
	theDCFwin.draw();
  
  const bool showControlClockOnDisplay = false;
  if (showControlClockOnDisplay) {
    theClockControl.setStringCallback([](String aString) {win.changeText(aString);});
  }
  dcf77.setBitDataCallback([](int aInt1, int aInt2, int aInt3, int aInt4, int aInt5) {theDCFwin.updateBit(aInt1, aInt2, aInt3, aInt4, aInt5);});

#endif // defined(TFT_DISPLAY)

#if defined(WIFI_DCF77_DECODER)
  DBG_PRINTLN("Trying to reach wifi ...\n");

#if defined(TFT_DISPLAY)
      win.changeText("Trying to reach wifi ...\n");
#endif // defined(TFT_DISPLAY)

#if defined(TFT_DISPLAY)
      server.setStringCallback([](String aString) {win.changeText(aString);});
#else 
      server.setStringCallback();
#endif // defined(TFT_DISPLAY)

      const String serverNameWifi = "picoGeiger";
      wifiIP = server.beginControler(serverNameWifi);
      if (server.isWifiOK()) {
        DBG_PRINT("Server running. IP :");
        DBG_PRINT(wifiIP);
        DBG_PRINT(" http://");
        DBG_PRINT(serverNameWifi);
        DBG_PRINTLN("/");
      } else {
        DBG_PRINTLN("Failed to initilialize server. Could not connect to wifi.");
      }
#endif // defined(WIFI_DCF77_DECODER)

// try to set the time ...
#if defined(WIFI_PULL_TIME_FROM_NET)

      if (server.isWifiOK()) {
        const String value = server.getTimeFromInternet();
        if (value != "") { 
          if (theClockControl.storeTimeString(value, false, "internetPull")) {
            DBG_PRINT(" In setup: Updated time from internet to ");
            DBG_PRINTLN(theClockControl.stringTime());
          } else {
            DBG_PRINT(" Updated time from internet failed string:  ");
            DBG_PRINTLN(value);
            DBG_PRINT(" time is still ");
            DBG_PRINTLN(theClockControl.stringTime());
          }
        } else {
            DBG_PRINT(" getTimeFromInternet () returned EMPTY (server may be down) ...");
            DBG_PRINTLN();
        }
      } else {
        DBG_PRINTLN("Could not get time from internet.");
      }

#endif // defined(WIFI_DCF77_DECODER)

#if defined(MILAN_CLOCK)
  if (debug2) theWordClock.debugSetHoursLeds(1);
  theWordClock.ocDriveLowAll_fullOFF();
#endif
  // setting up Led and flash test
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  delay(100);
  digitalWrite(LEDPIN, LOW);
  delay(200);
  digitalWrite(LEDPIN, HIGH);
  delay(300);
  digitalWrite(LEDPIN, LOW);

  DBG_PRINTLN("End setup");
}

void loop() {
  DBG_PRINTLN("Start loop");



  dcf77.reset();
  dcf77.initListen();
  DBG_PRINTLN("Start listening to dcf77 signal ...");

  for (long superLoop = 0; superLoop < 100000000; superLoop++) {
    int lastMinL1 = minute(now());
    for (long fastLoop = 0; fastLoop < 1000000; fastLoop ++) { // 1000000 about 18 min
      // Main listener : returns when have recieved valid time/date. May last minutes.
      const int isTimeValid = dcf77.listen(theClockControl);
      if (isTimeValid == 1) {
        theClockControl.storeTime(dcf77.getTM(), false, "dcf77");
        debug2 = false;  // stop
      }
      time_t t = now();
      const int curMin = minute(t);
      if (lastMinL1 != curMin) { // every minute
        lastMinL1 = curMin;
#if defined(MILAN_CLOCK)
        theWordClock.setWordClock(curMin, hour(t),
                                  theClockControl.isReliable());
#endif // defined(MILAN_CLOCK)

#if defined(TFT_DISPLAY)
        win.changeText(theClockControl.getStringDateHourMinReliable() + "\n");
#endif // defined(TFT_DISPLAY)
      } // every minute

#if defined(WIFI_DCF77_DECODER)
      if (server.isWifiOK()) {
        const String value =
        server.testIfRequest(theClockControl.isReliable(), dcf77);
        if (value != "") { // 2026-03-20T14:35  // http://192.168.1.65/set?value=2026-03-21T10:35
          if (theClockControl.storeTimeString(value, false, "internetPush")) {
            DBG_PRINT("Considered to update time from wifi to ");
            DBG_PRINTLN(theClockControl.stringTime());
            break;
          } else {
            DBG_PRINT(" Considered from wifi failed string:  ");
            DBG_PRINTLN(value);
            DBG_PRINT(" time is still ");
            DBG_PRINTLN(theClockControl.stringTime());
          }
        }
      }
#endif // defined(WIFI_DCF77_DECODER)

#if defined(TFT_DISPLAY)
      win.drawShift(fastLoop);
#endif // defined(TFT_DISPLAY)
      if (isTimeValid == 1) {
        break;
      }
    } // fastLoop


// try to update time from web
#if defined(WIFI_PULL_TIME_FROM_NET)

      if (server.isWifiOK()) {
        const String value = server.getTimeFromInternet();
        if (theClockControl.storeTimeString(value, false, "internetPull")) {
            DBG_PRINT(" in loop : Considered update time from wifi to ");
            DBG_PRINTLN(theClockControl.stringTime());
          } else {
            if (value == "") {
              DBG_PRINT(" Considered update time from internet failed string is empty");
              DBG_PRINTLN(value);            
            } else {
              DBG_PRINT(" Considered update time from internet failed string:  ");
              DBG_PRINTLN(value);       
            }
            
            DBG_PRINT(" time is still ");
            DBG_PRINTLN(theClockControl.stringTime());
          }
      } else {
            DBG_PRINT(" Considered update time from internet failed.");
            DBG_PRINT(" time is still ");
            DBG_PRINTLN(theClockControl.stringTime());
      }
#endif // defined(WIFI_GET_TIME_FROM_NET)

  } // end superLoop loop
} // loop()

*/
