// Task 04 — ESP32 -> Firebase Realtime Database: live telemetry + manual bulb control
//
// RECONSTRUCTED from the portfolio write-up (pins, paths and intervals match it).
// It has not been compiled or flashed from this repo — treat it as a starting point.
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
#define USER_EMAIL     "device@example.com"      // a user created in Firebase Authentication
#define USER_PASSWORD  "YOUR_DEVICE_PASSWORD"    // the rules require auth != null

#define DHT_PIN    4
#define DHT_TYPE   DHT11
#define LDR_PIN    34
#define RELAY_PIN  26
#define SENSOR_INTERVAL 2000

DHT dht(DHT_PIN, DHT_TYPE);
FirebaseData fbdo;      // writes
FirebaseData stream;    // control stream (a stream needs its own object)
FirebaseAuth auth;
FirebaseConfig config;

bool relayState = false;
unsigned long lastPush = 0;

int readLDR() {                       // a bare LDR is noisy — average a short burst
  int sum = 0;
  for (int i = 0; i < 15; i++) { sum += analogRead(LDR_PIN); delay(8); }
  return sum / 15;
}

void applyRelay(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, state ? LOW : HIGH);   // active-LOW module
}

void onControl(FirebaseStream data) {
  if (data.dataPath() == "/bulbState" && data.dataType() == "boolean") {
    applyRelay(data.boolData());
  }
}
void onStreamTimeout(bool timeout) { if (timeout) Serial.println("stream timeout, resuming..."); }

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

  Firebase.RTDB.beginStream(&stream, "/appliances");
  Firebase.RTDB.setStreamCallback(&stream, onControl, onStreamTimeout);
}

void loop() {
  if (Firebase.ready() && millis() - lastPush >= SENSOR_INTERVAL) {
    lastPush = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) { Serial.println("DHT read failed"); return; }

    FirebaseJson json;
    json.set("temperature", t);
    json.set("humidity", h);
    json.set("ldr", readLDR());
    json.set("bulbState", relayState);
    json.set("timestamp/.sv", "timestamp");       // server-side time
    Firebase.RTDB.setJSON(&fbdo, "/live", &json); // overwritten each time: the "now" view
  }
}
