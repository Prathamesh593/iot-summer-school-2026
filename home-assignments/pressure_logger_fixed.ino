#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---- Wi-Fi & MQTT ----
const char* ssid     = "''''''''''''";
const char* password = "''''''''''''";
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic  = "iitjammu/prathmesh/pressure"; // replace with your name

WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastMqttPublish = 0;
const unsigned long MQTT_INTERVAL = 60000; // 60 sec

// ---- OLED SPI pins (your kit) ----
#define OLED_MOSI 23
#define OLED_CLK  18
#define OLED_DC   16
#define OLED_CS   5
#define OLED_RESET 17

Adafruit_SSD1306 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// ---- BMP280 (I2C) ----
Adafruit_BMP280 bmp; // uses default SDA=21, SCL=22 on ESP32

// ---- Pins ----
#define POT_PIN   34
#define LED_GREEN 25
#define LED_RED   26

// ---- Circular buffer for pressure trend ----
#define LOG_SIZE 24
float pressureLog[LOG_SIZE];
int logIndex = 0;
bool bufferFilled = false;

unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 30000; // 30 sec

unsigned long lastPageSwitch = 0;
bool showPage1 = true;

// ---- Calibration ----
// seaLevelPressure is now calibrated ONCE at startup (or on demand),
// NOT recomputed every reading. Recomputing it every loop from the
// potentiometer was the bug: it made bmp.readAltitude() mathematically
// cancel the pressure term out, so altitude just echoed the pot position
// instead of reflecting real sensor data.
float seaLevelPressure = 1013.25; // hPa, default; overwritten by calibrateAltitude()
bool needsCalibration = true;     // set true again if you want to re-calibrate on the fly

// Potentiometer now only sets the "known altitude" reference used *when calibrating*,
// not something fed into every single reading.
float localAltitude = 0; // meters, from potentiometer at calibration time

// Globals to hold last readings for display
String lastTrend = "STABLE";
float lastPressure = 0, lastTemp = 0, lastAltitude = 0;

// Stale-read detection: if raw pressure comes back byte-for-byte identical
// across too many consecutive readings, that's a sign the sensor isn't
// taking fresh samples (I2C fault, stuck cache) rather than a genuinely
// stable environment -- real air pressure always drifts slightly.
float prevRawPressure = -1;
int staleCount = 0;
const int STALE_THRESHOLD = 3;

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // BMP280 init
  Serial.println("Initializing BMP280...");
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found at 0x76, trying 0x77...");
    if (!bmp.begin(0x77)) {
      Serial.println("BMP280 not found! Check wiring.");
      while (1) delay(10);
    }
  }
  Serial.println("BMP280 OK");

  // Confirm it's a genuine BMP280 (should print 0x58).
  // If this prints something else (0x00, 0xFF, or a random value),
  // that's a strong sign of a wiring problem or a counterfeit/different chip.
  Serial.print("Sensor ID: 0x");
  Serial.println(bmp.sensorID(), HEX);

  // Explicitly force NORMAL mode with continuous sampling. begin() usually
  // sets this by default, but setting it explicitly rules out the sensor
  // sitting in a low-power/forced mode that stops taking new samples,
  // which would explain identical readings repeating every cycle.
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                   Adafruit_BMP280::SAMPLING_X2,
                   Adafruit_BMP280::SAMPLING_X16,
                   Adafruit_BMP280::FILTER_X16,
                   Adafruit_BMP280::STANDBY_MS_500);

  // OLED init
  Serial.println("Initializing OLED...");
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED init failed!");
    while (1) delay(10);
  }
  Serial.println("OLED OK");

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Pressure Logger");
  display.println("Starting...");
  display.display();
  delay(1000);

  for (int i = 0; i < LOG_SIZE; i++) pressureLog[i] = 0;

  // One-time calibration using whatever the pot is set to right now.
  // Set the pot to your actual local altitude (in meters) BEFORE power-up,
  // or press/hold nothing -- this just runs once at boot.
  calibrateAltitude();

  // Wi-Fi + MQTT init
  connectWiFi();
  mqttClient.setServer(mqtt_server, 1883);
  connectMQTT();
}

void loop() {
  // Potentiometer is read continuously so you can see it and use it to
  // trigger a re-calibration, but it no longer feeds into every altitude
  // calculation directly.
  int potRaw = analogRead(POT_PIN); // 0-4095 on ESP32
  localAltitude = map(potRaw, 0, 4095, 0, 2000);

  if (millis() - lastRead > READ_INTERVAL) {
    lastRead = millis();
    takeReading();
  }

  // Page cycling every 5 seconds
  if (millis() - lastPageSwitch > 5000) {
    lastPageSwitch = millis();
    showPage1 = !showPage1;
    updateDisplay();
  }

  // Keep MQTT alive + publish every 60 sec
  mqttClient.loop();
  if (millis() - lastMqttPublish > MQTT_INTERVAL) {
    lastMqttPublish = millis();
    publishMQTT();
  }
}

// Calibrates seaLevelPressure ONCE using the current raw pressure reading
// and whatever localAltitude (from the pot) is set to at the moment this
// is called. Call this at setup(), or again later on demand (e.g. serial
// command) if you move to a different known altitude.
void calibrateAltitude() {
  float refPressure = bmp.readPressure() / 100.0F; // hPa
  seaLevelPressure = refPressure / pow(1.0 - (localAltitude / 44330.0), 5.255);
  needsCalibration = false;

  Serial.print("Calibrated. Ref pressure: ");
  Serial.print(refPressure);
  Serial.print(" hPa at altitude: ");
  Serial.print(localAltitude);
  Serial.print(" m -> seaLevelPressure: ");
  Serial.print(seaLevelPressure);
  Serial.println(" hPa");
}

void takeReading() {
  float pressure = bmp.readPressure() / 100.0F; // hPa
  float temp = bmp.readTemperature();

  // Stale-read check: if pressure is identical to the last reading,
  // count it. Too many in a row means the sensor probably isn't
  // sampling fresh data (I2C fault) rather than the room being
  // perfectly still.
  if (pressure == prevRawPressure) {
    staleCount++;
  } else {
    staleCount = 0;
  }
  prevRawPressure = pressure;

  if (staleCount >= STALE_THRESHOLD) {
    Serial.print("WARNING: pressure reading unchanged for ");
    Serial.print(staleCount + 1);
    Serial.println(" cycles in a row -- sensor may not be sampling. Check I2C wiring/pull-ups.");
  }

  // Uses the ONE-TIME calibrated seaLevelPressure, so altitude now
  // actually reflects changes in the real pressure reading instead of
  // just mirroring the potentiometer.
  float altitude = bmp.readAltitude(seaLevelPressure);

  // Store in circular buffer
  int oldestIndex = bufferFilled ? (logIndex + 1) % LOG_SIZE : 0;
  pressureLog[logIndex % LOG_SIZE] = pressure;
  logIndex++;
  if (logIndex >= LOG_SIZE) bufferFilled = true;

  // Trend calculation: newest vs oldest
  float oldest = pressureLog[oldestIndex];
  float newest = pressure;
  float diff = newest - oldest;

  String trend = "STABLE";
  if (bufferFilled) {
    if (diff > 0.5) trend = "RISING";        // +0.5 hPa
    else if (diff < -0.5) trend = "FALLING";
  }

  // LED indication
  if (trend == "FALLING") {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  } else {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
  }

  // Serial table output
  Serial.print("Pressure: "); Serial.print(pressure);
  Serial.print(" hPa | Temp: "); Serial.print(temp);
  Serial.print(" C | Altitude: "); Serial.print(altitude);
  Serial.print(" m | Trend: "); Serial.println(trend);

  // Sanity check: real sea-level-referenced pressure should sit roughly
  // 900-1050 hPa. If you see wild jumps (like 115 or 900+ swings between
  // boots), that's a wiring/power issue on the BMP280, not a code issue.
  if (pressure < 300 || pressure > 1100) {
    Serial.println("WARNING: pressure reading looks physically implausible. Check BMP280 wiring/power.");
  }

  // Update globals for display
  lastTrend = trend;
  lastPressure = pressure;
  lastTemp = temp;
  lastAltitude = altitude;
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);

  if (showPage1) {
    display.setTextSize(1);
    display.println("-- Sensor Data --");
    display.print("Pressure: "); display.print(lastPressure); display.println(" hPa");
    display.print("Temp: "); display.print(lastTemp); display.println(" C");
    display.print("Alt: "); display.print(lastAltitude); display.println(" m");
  } else {
    display.setTextSize(1);
    display.println("-- Trend --");
    display.setTextSize(2);
    if (lastTrend == "RISING") display.println("UP ^");
    else if (lastTrend == "FALLING") display.println("DOWN v");
    else display.println("STABLE ->");
    display.setTextSize(1);
    display.print("Pot (calib) alt: ");
    display.print((int)localAltitude);
    display.println(" m");
  }
  display.display();
}

// ---- Wi-Fi ----
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi FAILED to connect. Check credentials.");
  }
}

// ---- MQTT ----
void connectMQTT() {
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 5) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 2s");
      delay(2000);
      attempts++;
    }
  }
}

void publishMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping MQTT publish");
    return;
  }
  if (!mqttClient.connected()) connectMQTT();

  // Build JSON manually
  String payload = "{";
  payload += "\"pressure\":" + String(lastPressure, 2) + ",";
  payload += "\"trend\":\"" + lastTrend + "\"";
  payload += "}";

  bool ok = mqttClient.publish(mqtt_topic, payload.c_str());
  if (ok) {
    Serial.println("Published to MQTT: " + payload);
  } else {
    Serial.println("MQTT publish FAILED for payload: " + payload);
  }
}
