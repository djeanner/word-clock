#include "WifiControl.h"

WifiControl::WifiControl(int aPort)
    : WiFiServer(aPort), fStringCallback({}), fisWifiOK(false)  {}

bool WifiControl::beginControler() {
// const char* password = "INSERT PASSWORD HERE";
#include "password.h"
  const char *ssid = "FibreBox_X6-1A0DE7";
  // const char* password = "GDAEPE69PTRXDTWPRC";
  WiFi.begin(ssid, password);
  bool isWifiOK;
  for (int waitWifi = 0; waitWifi < 100; waitWifi++) {
    const auto status = WiFi.status();
    if (status == WL_CONNECTED) {
      isWifiOK = true;

      thisPrint("http://");
      thisPrint(WiFi.localIP().toString());
      thisPrint("/\n");
      delay(5000);

      WiFiServer::begin();
      break;
    }
    // if (waitWifi == 0) {DBG_PRINT("\nTrying to connect wifi. Status ");}
    //  if (waitWifi == 0) {DBG_PRINTLN(status);}
    thisPrint("Attempt ");
    thisPrint(String((long)waitWifi));
    thisPrint("/100 ()");
    thisPrint(String((long)status));
    thisPrint(")\n");
    delay(500);
    // DBG_PRINT(".");
  }
  if (!isWifiOK) {
    // DBG_PRINTLN("\nCould not reach wifi!");
    thisPrint("No wifi : No server.\n");
    delay(1500);
  }
  fisWifiOK = isWifiOK;
  return isWifiOK;
}

void WifiControl::makeClient() { fClient = WiFiServer::accept(); }

void WifiControl::setStringCallback(
    std::function<void(String)> aStringCallback) {
  fStringCallback = aStringCallback;
}

void WifiControl::thisPrint(String aString) {
  if (fStringCallback) {
    fStringCallback(aString);
  }
}

void WifiControl::thisPrintln(String aString) { thisPrint(aString + "\n"); }

void WifiControl::testIfRequest(bool isReliable, DCF77Decoder& dcf77) {
  if (fisWifiOK) {
    fClient = WiFiServer::accept();

    if (fClient) {
      // DBG_PRINTLN("New fClient connected");

      // Wait for data to arrive
      unsigned long timeout = millis();
      while (!fClient.available()) {
        if (millis() - timeout > 2000) {
          fClient.stop();
          break;
        }
      }

      // Read the HTTP request
      while (fClient.available()) {
        fClient.read();
      }
      // Réponse HTTP
      fClient.print("HTTP/1.1 200 OK\r\n");
      fClient.print("Content-Type: text/html\r\n");
      // fClient.print("Content-Type: application/json\r\n"); // JSON header
      fClient.print("Connection: close\r\n");
      fClient.print("\r\n");

      // main part
      fClient.println("<!DOCTYPE html>");
      fClient.println("<html>");
      fClient.println(" <h1>DCF77 data</h1>");
      fClient.println(" <h2>Running on Pico W Web Server</h2>");

      time_t t = now();
      String lastTimeString =
          (day(t) < 10 ? " " : "") + String(day(t)) + "/" +
          (month(t) < 10 ? " " : "") + String(month(t)) + "/" +
          (year(t) < 10 ? " " : "") + String(year(t)) + " " +
          (hour(t) < 10 ? " " : "") + String(hour(t)) + ":" +
          (minute(t) < 10 ? "0" : "") + String(minute(t)) + ":" +
          (second(t) < 10 ? "0" : "") + String(second(t)) + " " +
          ((isReliable) ? "(+)" : "(-)") + "";
      fClient.print(" <pre>");
      fClient.print(lastTimeString);
      fClient.println("</pre>");
      const int max = dcf77.getNumberLineArchive();
      fClient.println(" <pre>");
      fClient.print(dcf77.getArchive(max + 1).c_str());
      fClient.println("");
      fClient.print(dcf77.getArchive(max).c_str());
      fClient.println("");
      fClient.println("                                                            "
                  "                  M??????????????RAZzaS&lt; min "
                  "&gt;1&lt;hour&gt;2&lt;dayM&gt;&lt;D&gt;&lt;mon&gt;&lt;year  "
                  "&gt;3_                       Error: Pos Dur Num");
      for (int zu = 0; zu < max + 2; zu++) {
        fClient.print(dcf77.getArchive(zu).c_str());
        fClient.println("");
      }
      fClient.println("</pre>");
      fClient.println("</html>");

      fClient.flush(); // force envoi de tout le buffer TCP
      delay(10);
      fClient.stop();
      // DBG_PRINTLN("Client disconnected");
    }
  }
}