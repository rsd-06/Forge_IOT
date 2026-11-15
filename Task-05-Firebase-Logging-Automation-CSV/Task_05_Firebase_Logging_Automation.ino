// Task 05 — Automatic / manual bulb control + historical logging to Firebase
//
// RECONSTRUCTED around the loop() published in the portfolio write-up. Differences from
// that excerpt, on purpose: the control stream uses its own FirebaseData object, and each
// log record also stores the operating mode. Not compiled from this repo — verify on hardware.
//
// Libraries: "Firebase ESP Client" (Mobizt), "DHT sensor library" (Adafruit) + Adafruit Unified Sensor.

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <DHT.h>

#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"
#define API_KEY        "YOUR_FIREBASE_WEB_API_KEY"
#define DATABASE_URL   "https://YOUR-PROJECT-default-rtdb.firebaseio.com/"
#define USER_EMAIL     "device@example.com"
#define USER_PASSWORD  "YOUR_DEVICE_PASSWORD"

#define DHT_PIN    4
#define DHT_TYPE   DHT11
#define LDR_PIN    34
#define RELAY_PIN  26
#define SENSOR_INTERVAL 2000

DHT dht(DHT_PIN, DHT_TYPE);
FirebaseData fbdo;      // writes
FirebaseData stream;    // reads the control paths over an open stream
FirebaseAuth auth;
FirebaseConfig config;

int  ldrThreshold = 2500;      // overwritten from /settings/ldrThreshold
bool relayState   = false;
String mode       = "manual";  // "manual" | "automatic"
unsigned long lastPush = 0;

// A bare LDR reading is noisy; average a short burst instead.
int readLDR() {
  int sum = 0;
  for (int i = 0; i < 15; i++) { sum += analogRead(LDR_PIN); delay(8); }
  return sum / 15;
}

void applyRelay(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, state ? LOW : HIGH);   // active-LOW module
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  applyRelay(false);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Stream the whole tree; loop() reacts only to the three control paths.
  Firebase.RTDB.beginStream(&stream, "/");
}

void loop() {
  if (!Firebase.ready()) return;

  // Control changes arrive over an open stream — no polling.
  if (Firebase.RTDB.readStream(&stream) && stream.streamAvailable()) {
    if (stream.dataPath() == "/appliances/bulbState" && mode == "manual") {
      applyRelay(stream.to<bool>());
    }
    if (stream.dataPath() == "/settings/mode")         mode = stream.to<String>();
    if (stream.dataPath() == "/settings/ldrThreshold") ldrThreshold = stream.to<int>();
  }

  if (millis() - lastPush >= SENSOR_INTERVAL) {
    lastPush = millis();

    float temperature = dht.readTemperature();
    float humidity    = dht.readHumidity();
    int   light       = readLDR();
    if (isnan(temperature) || isnan(humidity)) { Serial.println("DHT read failed"); return; }

    if (mode == "automatic") {
      bool shouldBeOn = (light > ldrThreshold);       // higher reading = darker room
      if (shouldBeOn != relayState) {
        applyRelay(shouldBeOn);
        // Write back so the dashboard reflects reality.
        Firebase.RTDB.setBool(&fbdo, "/appliances/bulbState", relayState);
      }
    }

    FirebaseJson json;
    json.set("temperature", temperature);
    json.set("humidity", humidity);
    json.set("ldr", light);
    json.set("bulbState", relayState);
    json.set("mode", mode);
    json.set("timestamp/.sv", "timestamp");
    Firebase.RTDB.pushJSON(&fbdo, "/sensorData", &json);   // append-only history
  }
}
