#ifndef WIFICONTROL_H
#define WIFICONTROL_H
#include <WiFi.h>
#include <time.h>
#include <TimeLib.h>
#include "hardware/timer.h"
#include "DCF77Decoder.h"

class WifiControl : public WiFiServer {
private:
  std::function<void(String)> fStringCallback;
  bool fisWifiOK;
  DCF77Decoder* dcf77;

  WiFiServer fServer;
  WiFiClient fClient;

public:
  WifiControl(int aPort = 80);
  bool beginControler();
  void makeClient();
  void setStringCallback(std::function<void(String)> aStringCallback = {});
  void thisPrint(String aString);
  void thisPrintln(String aString);
  void testIfRequest(bool isReliable, DCF77Decoder& dcf77);
};

#endif // TFT_SCREEN_H