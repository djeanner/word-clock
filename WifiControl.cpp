#include "WifiControl.h"

WifiControl::WifiControl(int aPort)
    : WiFiServer(aPort), fStringCallback({}), fisWifiOK(false) {}

String WifiControl::beginControler() {
// const char* password = "INSERT PASSWORD HERE";
#include "password.h"
  const char *ssid = "FibreBox_X6-1A0DE7";
  // const char* password = "GDAEPE69PTRXDTWPRC";
  WiFi.begin(ssid, password);
  String isWifiOK = "";
  for (int waitWifi = 0; waitWifi < 100; waitWifi++) {
    const auto status = WiFi.status();
    if (status == WL_CONNECTED) {
      isWifiOK = WiFi.localIP().toString();

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
  if (isWifiOK == "") {
    // DBG_PRINTLN("\nCould not reach wifi!");
    thisPrint("No wifi : No server.\n");
    delay(1500);
  }
  fisWifiOK = (isWifiOK != "");
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

String WifiControl::getFromURL(const String requestLine, const String getString,
                          const String setString) {
  String url = "/"; // default fallback
  String value = "";

  int firstSpace = requestLine.indexOf(' ');
  if (firstSpace != -1) {

    int secondSpace = requestLine.indexOf(' ', firstSpace + 1);

    if (secondSpace != -1) {
      url = requestLine.substring(firstSpace + 1, secondSpace);
    } else {
      // Only one space found → take rest of line
      url = requestLine.substring(firstSpace + 1);
    }
  }
  // Parse value
  const String f1 = "/" + setString + "?";
  const String f2 = "/" + f1;
  if (url.startsWith(f1) || url.startsWith(f2)) {

    int qIndex = url.indexOf('?');
    if (qIndex != -1) {
      String query = url.substring(qIndex + 1);
      int keyIndex = query.indexOf(getString);
      if (keyIndex != -1) {
        int start = keyIndex + 6;
        int end = query.indexOf('&', start);
        if (end == -1)
          end = query.length();
        value = query.substring(start, end);
      }
    }
  }
  return value;
}

String WifiControl::testIfRequest(bool isReliable, DCF77Decoder &dcf77) {
  String value = "";
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
      String requestLine = "";
      // Read the first line of the request
      while (fClient.available()) {
        char c = fClient.read();
        if (c == '\n')
          break; // end of first line
        if (c != '\r')
          requestLine += c;
      }
      while (fClient.available()) {
        fClient.read();
      }

      // Extract a value with "http://192.168.1.65/set?value=toto"
      value = getFromURL(requestLine, "value", "set");

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
      if (value != "") {
        fClient.print(" <h3> value : ");
        fClient.print(value);
        fClient.println(" </h3>");
      }

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
      fClient.println(
          "                                                            "
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
  return value;
}

String WifiControl::httpGET(const char *host, const String &url, int port) {
  WiFiClient client;

  if (!client.connect(host, port)) {
    return "";
  }

  // Send HTTP request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host +
               "\r\n" + "Connection: close\r\n\r\n");

  // Wait for response
  unsigned long timeout = millis();
  while (!client.available()) {
    if (millis() - timeout > 5000) {
      client.stop();
      return "";
    }
  }

  // Skip headers
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r")
      break;
  }

  // Read body
  String payload = "";
  while (client.available()) {
    payload += client.readString();
  }

  client.stop();
  return payload;
}

String WifiControl::getTimeFromInternet() {
  String response = httpGET("worldclockapi.com", "/api/json/cet/now");
  if (response.length() == 0)
    return "";
  // extract currentDateTime field of json
  const String key = "\"currentDateTime\":\"";
  int idx = response.indexOf(key);
  if (idx == -1)
    return "";

  int start = idx + key.length();
  int end = response.indexOf('"', start);

  if (end == -1 || end <= start)
    return "";

  String datetime = response.substring(start, end);
  // Example: 2026-03-22T14:35:12+01:00

  return datetime;
}

void WifiControl::shutdownWiFi() {
  if (fisWifiOK) {
    WiFi.disconnect(true); // disconnect and erase credentials from RAM
    WiFi.end();            // stop WiFi hardware
    fisWifiOK = false;
  }
}
