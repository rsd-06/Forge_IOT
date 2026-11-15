// Task 02 — Adafruit IO (MQTT) relay control
// Subscribes to the "esp" feed; "ON" / "OFF" published from anywhere drives the relay.
//
// Libraries: "Adafruit IO Arduino" (+ its dependencies: Adafruit MQTT, ArduinoHttpClient).
// Keep real credentials out of git — copy the file locally and fill them in there.

#include <AdafruitIO_WiFi.h>

#define WIFI_SSID   "YOUR_WIFI_SSID"
#define WIFI_PASS   "YOUR_WIFI_PASSWORD"
#define IO_USERNAME "YOUR_ADAFRUIT_IO_USERNAME"
#define IO_KEY      "YOUR_ADAFRUIT_IO_KEY"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *relayFeed = io.feed("esp");

#define RELAY_PIN 26

// Called whenever anything publishes to the feed, from anywhere.
void handleMessage(AdafruitIO_Data *data) {
  String command = data->toString();
  command.trim();
  command.toUpperCase();

  if (command == "ON")       digitalWrite(RELAY_PIN, HIGH);
  else if (command == "OFF") digitalWrite(RELAY_PIN, LOW);

  Serial.println("BULB -> " + command);
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  relayFeed->onMessage(handleMessage);
  io.connect();
  while (io.status() < AIO_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(io.statusText());

  relayFeed->get();   // pull the current state on reconnect
}

void loop() {
  io.run();           // keeps the MQTT connection alive
}
