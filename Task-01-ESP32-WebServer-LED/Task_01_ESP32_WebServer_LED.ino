// Task 01 — ESP32 web server + HTML LED control
// Serves a control page from the board on port 80 and toggles the on-board LED (GPIO 2).
//
// NOTE: reconstructed from the portfolio write-up (handlers are the originals; the
// HTML page and handleRoot were filled in to match the screenshot). Re-flash and test
// on your board before relying on it.

#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "YOUR_WIFI_SSID";       // 2.4 GHz network only
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);
const int ledPin = 2;      // on-board blue LED

// The page lives in flash (PROGMEM). Changing a colour means re-flashing the board.
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 LED Control</title>
  <style>
    body   { font-family: Arial, sans-serif; background: #f0f0f0; display: flex; justify-content: center; padding-top: 40px; }
    .card  { background: #fff; padding: 28px 36px; border-radius: 14px; box-shadow: 0 6px 18px rgba(0,0,0,.12); text-align: center; }
    h1     { margin: 0 0 6px; font-size: 1.5rem; }
    p      { margin: 0 0 20px; font-size: .85rem; color: #444; }
    a      { text-decoration: none; }
    button { border: 0; color: #fff; font-size: 1rem; padding: 12px 34px; margin: 0 6px; border-radius: 6px; cursor: pointer; }
    .on    { background: #2e9e3f; }
    .off   { background: #e0401f; }
  </style>
</head>
<body>
  <div class="card">
    <h1>ESP32 LED Control</h1>
    <p>Control the ESP32 LED using HTTP</p>
    <a href="/ledon"><button class="on">ON</button></a>
    <a href="/ledoff"><button class="off">OFF</button></a>
  </div>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleLEDOn() {
  digitalWrite(ledPin, HIGH);
  server.send(200, "text/html", "<h1>LED is ON</h1>");
  Serial.println("LED ON");
}

void handleLEDOff() {
  digitalWrite(ledPin, LOW);
  server.send(200, "text/html", "<h1>LED is OFF</h1>");
  Serial.println("LED OFF");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Station mode: join an existing router. The board's address is printed below.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("Open http://"); Serial.println(WiFi.localIP());

  // Alternative — soft access point: the board hosts its own network and is
  // reachable at 192.168.4.1 (the address in the portfolio screenshot).
  //   WiFi.softAP("ESP32-LED", "12345678");
  //   Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);          // serves the control page
  server.on("/ledon",  handleLEDOn);
  server.on("/ledoff", handleLEDOff);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
}
