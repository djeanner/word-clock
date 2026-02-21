/*
  // use pico board library by Earle F. Philhower, III 
*/
#include <Arduino.h> // add in header of classes
#include <time.h>
#include <TimeLib.h>
#include "hardware/timer.h"

// set using as compiler option :  --build-property compiler.cpp.extra_flags="-DMILAN_CLOCK=1"
#if defined(DCF77DispClock)
// not using interupts for word clock is not displaying time
#define INTERRUPT_WORD_CLOCK 0
// both methods are implemented but have different requirements for compilation
// use of interrupts does not require a clock_control to be defined for DCF77Decoder  
#define CLOCK_CONTROL_INTERRUPT 0
#define SERIAL_DEBUG 1  // <<< set to 0 to disable ALL serial output NOTE: not functionning well with INTERRUPT_WORD_CLOCK
#define TFT_DISPLAY 1
#define WIFI_DCF77_DECODER 1

#endif

#if defined(TEST_CLOCK)
// not using interupts for word clock is not displaying time
#define INTERRUPT_WORD_CLOCK 0
// both methods are implemented but have different requirements for compilation
// use of interrupts does not require a clock_control to be defined for DCF77Decoder  
#define CLOCK_CONTROL_INTERRUPT 0
#define SERIAL_DEBUG 1  // <<< set to 0 to disable ALL serial output NOTE: not functionning well with INTERRUPT_WORD_CLOCK
#define TFT_DISPLAY 1
#endif

// This is to have MILAN_CLOCK as default
#ifndef CLOCK_CONTROL_INTERRUPT
#define MILAN_CLOCK 1
#endif

#if defined(MILAN_CLOCK)
#include "WordClock.h"
// not using interupts for word clock is not displaying time
#define INTERRUPT_WORD_CLOCK 1
// both methods are implemented but have different requirements for compilation
// use of interrupts does not require a clock_control to be defined for DCF77Decoder  
#define CLOCK_CONTROL_INTERRUPT 1
#define SERIAL_DEBUG 0  // <<< set to 0 to disable ALL serial output NOTE: not functionning well with INTERRUPT_WORD_CLOCK
#endif


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

// delay_ms stops the clock of the CPU. not compatible with serial or other interrupt-based services
#if (INTERRUPT_WORD_CLOCK == 0) && (SERIAL_DEBUG == 0) && (CLOCK_CONTROL_INTERRUPT == 0)
  #define SLEEPORDELAYMS(ms) sleep_ms(ms)
#else
  #define SLEEPORDELAYMS(ms) delay(ms)
#endif

const bool debug8 = false;   // text
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

#if defined(TFT_DISPLAY)
#include <WiFi.h>

const char* ssid = "FibreBox_X6-1A0DE7";
const char* password = "GDAEPE69PTRXDTWPRC";

WiFiServer server(80);
#endif // defined(WIFI_DCF77_DECODER)

#if defined(MILAN_CLOCK)
WordClock theWordClock;
#endif

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

#if defined(WIFI_DCF77_DECODER)
const bool wantsAServer = true;
bool isWifiOK = false;
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

#endif


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
 //screen.getTFT()->drawRect(2, 2, screen.getWidth(), screen.getHeight(), ST77XX_RED);

	win.draw();

	theDCFwin.draw();
  for (int l = 0 ; l < 100; l++) {
	  win.drawShift(l);
    if (l == 50) {win.changeText(" Rolling demo changed text. It needs a ENDL         \n");}
  }
  win.changeText("Fixed demo changed text. It needs a ENDL         \n");
  // note if global : []
  // note if local : [&theDCFwin]
  theClockControl.setStringCallback([](String aString) {win.changeText(aString);});
  //dcf77.setStringCallback([](String aString) {win.changeText(aString);});
  dcf77.setBitDataCallback([](int aInt1, int aInt2, int aInt3, int aInt4, int aInt5) {theDCFwin.updateBit(aInt1, aInt2, aInt3, aInt4, aInt5);});

#endif // defined(TFT_DISPLAY)

#if defined(WIFI_DCF77_DECODER)
#if defined(TFT_DISPLAY)
      win.changeText("Trying to reach wifi...\n");
#endif // defined(TFT_DISPLAY)
WiFi.begin(ssid, password);

  for(int waitWifi = 0; waitWifi < 100; waitWifi++) {
  const auto status = WiFi.status();
    if (status == WL_CONNECTED) {
      isWifiOK = true;

#if defined(TFT_DISPLAY)
      win.changeText("http://");
      win.changeText(WiFi.localIP().toString());
      win.changeText("/\n");
#endif // defined(TFT_DISPLAY)

      DBG_PRINT("Connected to http://");
      DBG_PRINT(WiFi.localIP());
      DBG_PRINTLN("/");
      delay(5000);

      server.begin();
       break;
    }
    if (waitWifi == 0) {DBG_PRINT("\nTrying to connect wifi. Status ");}
    if (waitWifi == 0) {DBG_PRINTLN(status);}
    #if defined(TFT_DISPLAY)
      win.changeText("Attempt ");
      win.changeText(String((long)waitWifi));
      win.changeText("/100 ()");
      win.changeText(String((long)status));
      win.changeText(")\n");
    #endif // defined(TFT_DISPLAY)
    delay(500);
    DBG_PRINT(".");
  }
  if (! isWifiOK) {
    DBG_PRINTLN("\nCould not reach wifi!");
#if defined(TFT_DISPLAY)
    win.changeText("No wifi ...\n");
        delay(1500);
    win.changeText("No server.\n");
#endif // defined(TFT_DISPLAY)
    delay(1500);
  }
#endif // defined(WIFI_DCF77_DECODER)

#if defined(MILAN_CLOCK)
  if (debug2) theWordClock.debugSetHoursLeds(1);
  theWordClock.ocDriveLowAll_fullOFF();
#endif

  // setting up Led
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  delay(100);
  digitalWrite(LEDPIN, LOW);
  delay(200);
  digitalWrite(LEDPIN, HIGH);
  delay(300);
  digitalWrite(LEDPIN, LOW);

  delay(2000);

  DBG_PRINTLN("End setup");
}

void loop() {

#if defined(MILAN_CLOCK)
  if (debug2) theWordClock.debugSetHoursLeds(3);
  // Test each output led
  const bool testEachLedFirst = false;
  if (testEachLedFirst) { 
    theWordClock.testLed();
  } 
#endif // defined(MILAN_CLOCK)
  dcf77.reset();
  dcf77.initListen();
  DBG_PRINTLN("Start listening to dcf77 signal ...");

  for (long superLoop = 0; superLoop < 100000000; superLoop++) {
    // Main listener : returns when have recieved valid time/date. May last minutes.

    
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
      if (lastMinL1 != curMin) { // every minute
        lastMinL1 = curMin;

#if defined(MILAN_CLOCK)
        theWordClock.setWordClock(curMin, hour(t), theClockControl.isReliable());
#endif // defined(MILAN_CLOCK)

#if defined(TFT_DISPLAY)
    time_t t = now();
    String lastTimeString = String(day(t)) + "/" +
                            String(month(t)) + "/" +
                            String(year(t)) + " " +
                            (hour(t) < 10 ? " " : "") + String(hour(t)) + ":" +
                            (minute(t) < 10 ? "0" : "") + String(minute(t)) + " " +
                            // ":" + (second(t) < 10 ? "0" : "") + String(second(t)) +
                            theClockControl.isReliable() ? "+" : "-";
                            "\n";
    win.changeText(lastTimeString);
#endif // defined(TFT_DISPLAY)
      } // every minute
#if defined(WIFI_DCF77_DECODER)
if (isWifiOK) {
  WiFiClient client = server.accept();

  if (client) {
    DBG_PRINTLN("New client connected");

     // Attendre que des données arrivent
    unsigned long timeout = millis();
    while (!client.available()) {
      if (millis() - timeout > 2000) {
        client.stop();
        break;
      }
    }

    // Lire toute la requête HTTP
    while (client.available()) {
      client.read();
    }
    // Réponse HTTP
    client.print("HTTP/1.1 200 OK\r\n");
    client.print("Content-Type: text/html\r\n");
    //client.print("Content-Type: application/json\r\n"); // JSON header
    client.print("Connection: close\r\n");
    client.print("\r\n");

    // main part
    client.println("<!DOCTYPE html>");
    client.println("<html>");
    client.println(" <h1>Pico W Web Server</h1>");

    time_t t = now();
    String lastTimeString = (day(t) < 10 ? " " : "") + String(day(t)) + "/" +
                            (month(t) < 10 ? " " : "") + String(month(t)) + "/" +
                            (year(t) < 10 ? " " : "") + String(year(t)) + " " +
                            (hour(t) < 10 ? " " : "") + String(hour(t)) + ":" +
                            (minute(t) < 10 ? "0" : "") + String(minute(t)) + 
                            ":" + (second(t) < 10 ? "0" : "") + String(second(t)) +
                            " " + (theClockControl.isReliable() ? "(+)" : "(-)") +
                            "\n";
    client.print(" <p>");
    client.print(lastTimeString);
    client.println(" </p>");                    
    for (int zu = 0; zu < 100; zu++) {
      client.print(" <p>");
      // if (zu < 10) {client.print(" ");}
      // client.print(zu);
      client.print("   ");
      client.print(dcf77.getArchive(zu).c_str());
      client.println(" </p>");
    }
    client.println("</html>");


    client.flush();    // force envoi de tout le buffer TCP
    delay(10);
    client.stop();
    DBG_PRINTLN("Client disconnected");
  }
}
#endif // defined(WIFI_DCF77_DECODER)
#if defined(TFT_DISPLAY)
  win.drawShift(fastLoop);
#endif // defined(TFT_DISPLAY)

    } // fastLoop



#if defined(MILAN_CLOCK)
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
    dcf77.reset();
    dcf77.initListen();
    DBG_PRINTLN("Restart listening to dcf77 signal ...");
#endif // defined(MILAN_CLOCK)
  }
}