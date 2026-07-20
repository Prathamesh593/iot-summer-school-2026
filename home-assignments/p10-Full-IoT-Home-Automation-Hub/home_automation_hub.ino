/*
  Project 10: Full IoT Home Automation Hub
  ESP32 (WROOM-32D, 11-pin breakout board)

  Sensors:  DHT22 (temp/humidity), MQ-2 (gas), PIR (motion), LM393 LDR (light)
  Actuators: 2-ch SSR relay (fan + light), passive buzzer
  Display:  OLED 128x64 SPI (SSD1306)
  Inputs:   BOOT button (fan override), 1 push button (light override)
  Network:  Wi-Fi web dashboard (AsyncWebServer) + MQTT publish (HiveMQ public broker)

  Automation Rules:
  1. Fan  (Relay CH1): AUTO ON if temp > 32C, AUTO OFF if temp < 28C (hysteresis)
                       Manual override via BOOT button (GPIO0) for 10 minutes
  2. Light(Relay CH2): AUTO ON if LDR dark AND PIR motion detected
                       AUTO OFF if LDR bright OR no motion for 3 minutes
                       Manual override via button (D32) for 10 minutes
  3. Gas Alert:        if MQ-2 > 60%, buzzer + OLED alert, force both relays OFF (safety),
                       publish ALERT to MQTT
                       MQ-2 heater needs to warm up - alerts are suppressed for the
                       first 2 minutes after boot (raw readings still shown/logged).

  Relay board is "Low Level Trigger": LOW = relay ON, HIGH = relay OFF

  CHANGELOG:
  - Fixed button handling: now edge-triggered (LOW transition) instead of level-triggered,
    so a stuck/floating pin can't spam toggle every 300ms.
  - Added 2-minute MQ-2 warm-up delay before gas alerts become active.
  - Fixed LDR mapping (confirmed via testing: this module reads HIGH in darkness).
  - Added Red LED (GPIO4) - lights up during gas alert, alongside buzzer.
  - Added web dashboard toggle buttons for Fan and Light relays (/toggle/fan, /toggle/light).
    Disabled during an active gas alert for safety.
  - Added uptime display to the web dashboard (previously OLED-only).
*/

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>

// ================= WIFI / MQTT CONFIG =================
const char* WIFI_SSID     = "''''''''''";
const char* WIFI_PASSWORD = "''''''''''";
const char* MQTT_BROKER   = "broker.hivemq.com";
const int   MQTT_PORT     = 1883;
const char* MQTT_TOPIC    = "iitjammu/prathamesh/home";   // change [yourname]

WiFiClient espClient;
PubSubClient mqttClient(espClient);
AsyncWebServer server(80);

// ================= PIN DEFINITIONS =================
#define DHT_PIN     33
#define DHT_TYPE    DHT22
#define MQ2_PIN     36   // VP, input-only ADC
#define PIR_PIN     12
#define LDR_PIN     39   // VN, input-only ADC
#define RELAY_FAN   15
#define RELAY_LIGHT 19
#define BUZZER_PIN  21
#define BTN_LIGHT   32   // manual override for light
#define BTN_FAN     0    // BOOT button, manual override for fan
#define LED_RED     4    // Red LED - lights up during gas alert

// OLED (hardware SPI)
#define OLED_MOSI 23
#define OLED_CLK  18
#define OLED_DC   16
#define OLED_CS   5
#define OLED_RESET 17
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                          OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

DHT dht(DHT_PIN, DHT_TYPE);

// Relay logic: board is active-LOW
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// ================= STATE =================
float temperature = 0, humidity = 0;
int gasPercent = 0;
int lightPercent = 0;
bool motionDetected = false;

bool fanOn = false;
bool lightOn = false;
bool gasAlert = false;

bool fanManualOverride = false;
bool lightManualOverride = false;
unsigned long fanOverrideStart = 0;
unsigned long lightOverrideStart = 0;
const unsigned long OVERRIDE_DURATION_MS = 10UL * 60 * 1000; // 10 min

unsigned long lastMotionTime = 0;
const unsigned long MOTION_TIMEOUT_MS = 3UL * 60 * 1000; // 3 min

unsigned long lastSensorRead = 0;
const unsigned long SENSOR_INTERVAL_MS = 5000;

unsigned long lastMqttPublish = 0;
const unsigned long MQTT_INTERVAL_MS = 30000;

unsigned long lastOledSwitch = 0;
int oledScreen = 0;
const unsigned long OLED_SWITCH_MS = 5000;

unsigned long bootTime = 0;

// ---- MQ-2 gas sensor warm-up ----
const unsigned long GAS_WARMUP_MS = 2UL * 60 * 1000; // 2 min MQ-2 heater warm-up
bool gasSensorReady = false;

// ---- button debounce + edge detection ----
unsigned long lastFanBtnPress = 0, lastLightBtnPress = 0;
const unsigned long DEBOUNCE_MS = 300;
bool lastFanBtnState = HIGH;   // previous raw pin state (HIGH = not pressed, INPUT_PULLUP)
bool lastLightBtnState = HIGH;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  bootTime = millis();

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LIGHT, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_LIGHT, INPUT_PULLUP);
  pinMode(BTN_FAN, INPUT_PULLUP); // BOOT button already pulled up on most boards
  pinMode(LED_RED, OUTPUT);

  digitalWrite(RELAY_FAN, RELAY_OFF);
  digitalWrite(RELAY_LIGHT, RELAY_OFF);
  digitalWrite(LED_RED, LOW);

  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("OLED init failed"));
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Home Automation Hub"));
  display.println(F("Booting..."));
  display.println(F("Gas sensor warming up"));
  display.display();

  connectWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  setupWebServer();
  server.begin();

  // capture initial button states so we don't fire a false edge at boot
  lastFanBtnState = digitalRead(BTN_FAN);
  lastLightBtnState = digitalRead(BTN_LIGHT);

  Serial.println(F("=== Home Automation Hub Ready ==="));
  Serial.println(F("MQ-2 warm-up in progress (2 min) - gas alerts suppressed until ready"));
}

// ================= WIFI =================
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("Connecting to WiFi"));
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print(F("WiFi connected. IP: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println(F("WiFi connect failed - will retry in loop"));
  }
}

void reconnectWiFiIfNeeded() {
  static unsigned long lastAttempt = 0;
  if (WiFi.status() != WL_CONNECTED && millis() - lastAttempt > 30000) {
    lastAttempt = millis();
    Serial.println(F("Retrying WiFi connection..."));
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

void reconnectMqttIfNeeded() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!mqttClient.connected()) {
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt > 10000) {
      lastAttempt = millis();
      Serial.print(F("Connecting to MQTT..."));
      String clientId = "ESP32HomeHub-" + String(random(0xffff), HEX);
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println(F("connected"));
      } else {
        Serial.print(F("failed, rc="));
        Serial.println(mqttClient.state());
      }
    }
  }
  mqttClient.loop();
}

// ================= SENSOR READING =================
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity = h;

  int gasRaw = analogRead(MQ2_PIN);          // 0-4095 (12-bit ADC)
  gasPercent = map(gasRaw, 0, 4095, 0, 100);

  int ldrRaw = analogRead(LDR_PIN);
  // CONFIRMED via testing: this module reads HIGH raw ADC (~3200-4095) in darkness
  // and LOW raw ADC (~400-550) in normal room light. Mapping flipped so that
  // lightPercent is LOW when dark and HIGH when bright, matching isDark/isBright below.
  lightPercent = map(ldrRaw, 0, 4095, 100, 0);

  bool pirNow = digitalRead(PIR_PIN) == HIGH;
  if (pirNow) {
    motionDetected = true;
    lastMotionTime = millis();
  } else if (millis() - lastMotionTime > 2000) {
    motionDetected = false; // small settle time after PIR pulse ends
  }
}

// ================= AUTOMATION RULES =================
void applyAutomationRules() {
  // ---- Gas sensor warm-up check ----
  if (!gasSensorReady && (millis() - bootTime > GAS_WARMUP_MS)) {
    gasSensorReady = true;
    Serial.println(F("MQ-2 warm-up complete - gas alerts now active"));
  }

  // ---- Rule 3: Gas safety shutdown takes priority over everything ----
  // Only acts once the sensor has had time to heat up and stabilize.
  if (gasSensorReady && gasPercent > 60) {
    if (!gasAlert) {
      Serial.println(F("GAS ALERT: forcing all relays OFF"));
    }
    gasAlert = true;
    fanOn = false;
    lightOn = false;
    digitalWrite(RELAY_FAN, RELAY_OFF);
    digitalWrite(RELAY_LIGHT, RELAY_OFF);
    digitalWrite(LED_RED, HIGH);
    tone(BUZZER_PIN, 1000); // continuous alarm tone
    return; // skip fan/light logic entirely while in gas alert
  } else {
    if (gasAlert) {
      Serial.println(F("Gas levels normal - resuming automation"));
      noTone(BUZZER_PIN);
      digitalWrite(LED_RED, LOW);
    }
    gasAlert = false;
  }

  // ---- Rule 1: Fan (temperature hysteresis) ----
  if (fanManualOverride) {
    if (millis() - fanOverrideStart > OVERRIDE_DURATION_MS) {
      fanManualOverride = false; // override expired, resume auto
      Serial.println(F("Fan manual override expired - back to AUTO"));
    }
  } else {
    if (!fanOn && temperature > 32.0) {
      fanOn = true;
      Serial.println(F("AUTO: Fan ON (temp > 32C)"));
    } else if (fanOn && temperature < 28.0) {
      fanOn = false;
      Serial.println(F("AUTO: Fan OFF (temp < 28C)"));
    }
  }
  digitalWrite(RELAY_FAN, fanOn ? RELAY_ON : RELAY_OFF);

  // ---- Rule 2: Light (LDR + PIR) ----
  if (lightManualOverride) {
    if (millis() - lightOverrideStart > OVERRIDE_DURATION_MS) {
      lightManualOverride = false;
      Serial.println(F("Light manual override expired - back to AUTO"));
    }
  } else {
    bool isDark = lightPercent < 20;   // adjust threshold after calibration
    bool isBright = lightPercent > 60;

    if (!lightOn && isDark && motionDetected) {
      lightOn = true;
      Serial.println(F("AUTO: Light ON (dark + motion)"));
    } else if (lightOn && (isBright || (millis() - lastMotionTime > MOTION_TIMEOUT_MS))) {
      lightOn = false;
      Serial.println(F("AUTO: Light OFF (bright or motion timeout)"));
    }
  }
  digitalWrite(RELAY_LIGHT, lightOn ? RELAY_ON : RELAY_OFF);

  if (!gasAlert) noTone(BUZZER_PIN);
}

// ================= MANUAL OVERRIDE BUTTONS =================
// Edge-triggered: only fires on the HIGH->LOW transition, not on every loop
// where the pin happens to read LOW. Prevents toggle-spam from a stuck,
// held, or floating/miswired pin.
void handleButtons() {
  unsigned long now = millis();

  bool fanBtnState = digitalRead(BTN_FAN);
  if (fanBtnState == LOW && lastFanBtnState == HIGH &&
      (now - lastFanBtnPress) > DEBOUNCE_MS) {
    lastFanBtnPress = now;
    fanOn = !fanOn;
    fanManualOverride = true;
    fanOverrideStart = now;
    digitalWrite(RELAY_FAN, fanOn ? RELAY_ON : RELAY_OFF);
    tone(BUZZER_PIN, 2000, 100);
    Serial.print(F("MANUAL: Fan toggled to "));
    Serial.println(fanOn ? "ON" : "OFF");
  }
  lastFanBtnState = fanBtnState;

  bool lightBtnState = digitalRead(BTN_LIGHT);
  if (lightBtnState == LOW && lastLightBtnState == HIGH &&
      (now - lastLightBtnPress) > DEBOUNCE_MS) {
    lastLightBtnPress = now;
    lightOn = !lightOn;
    lightManualOverride = true;
    lightOverrideStart = now;
    digitalWrite(RELAY_LIGHT, lightOn ? RELAY_ON : RELAY_OFF);
    tone(BUZZER_PIN, 2000, 100);
    Serial.print(F("MANUAL: Light toggled to "));
    Serial.println(lightOn ? "ON" : "OFF");
  }
  lastLightBtnState = lightBtnState;
}

// ================= OLED DISPLAY =================
void updateOled() {
  display.clearDisplay();
  display.setCursor(0, 0);

  if (gasAlert) {
    display.setTextSize(2);
    display.println(F("!! GAS !!"));
    display.println(F("ALERT!"));
    display.setTextSize(1);
    display.print(F("Gas: "));
    display.print(gasPercent);
    display.println(F("%"));
    display.display();
    return;
  }

  if (oledScreen == 0) {
    display.println(F("-- Climate --"));
    display.print(F("Temp: "));
    display.print(temperature, 1);
    display.println(F(" C"));
    display.print(F("Humidity: "));
    display.print(humidity, 1);
    display.println(F(" %"));
  } else if (oledScreen == 1) {
    display.println(F("-- Sensors --"));
    display.print(F("Gas:  "));
    display.print(gasPercent);
    display.print(F(" %"));
    if (!gasSensorReady) {
      unsigned long remainingSec = (GAS_WARMUP_MS - (millis() - bootTime)) / 1000;
      display.print(F(" (warmup "));
      display.print(remainingSec);
      display.println(F("s)"));
    } else {
      display.println();
    }
    display.print(F("PIR:  "));
    display.println(motionDetected ? "MOTION" : "clear");
    display.print(F("Light: "));
    display.print(lightPercent);
    display.println(F(" %"));
  } else {
    display.println(F("-- Relays --"));
    display.print(F("Fan:   "));
    display.println(fanOn ? "ON" : "OFF");
    display.print(F("Light: "));
    display.println(lightOn ? "ON" : "OFF");
    unsigned long upSec = (millis() - bootTime) / 1000;
    display.print(F("Up: "));
    display.print(upSec / 86400); display.print(F("d "));
    display.print((upSec % 86400) / 3600); display.print(F("h "));
    display.print((upSec % 3600) / 60); display.println(F("m"));
  }

  display.display();
}

// ================= SERIAL LOG =================
void logSerial() {
  Serial.print(millis());
  Serial.print(F(",temp=")); Serial.print(temperature);
  Serial.print(F(",hum=")); Serial.print(humidity);
  Serial.print(F(",gas=")); Serial.print(gasPercent);
  Serial.print(F(",gasReady=")); Serial.print(gasSensorReady);
  Serial.print(F(",pir=")); Serial.print(motionDetected);
  Serial.print(F(",light=")); Serial.print(lightPercent);
  Serial.print(F(",fan=")); Serial.print(fanOn);
  Serial.print(F(",lightRelay=")); Serial.print(lightOn);
  Serial.print(F(",alert=")); Serial.println(gasAlert);
}

// ================= MQTT PUBLISH =================
void publishMqtt() {
  if (!mqttClient.connected()) return;
  String payload = "{";
  payload += "\"temp\":" + String(temperature, 1) + ",";
  payload += "\"humidity\":" + String(humidity, 1) + ",";
  payload += "\"gas\":" + String(gasPercent) + ",";
  payload += "\"gas_ready\":" + String(gasSensorReady ? 1 : 0) + ",";
  payload += "\"pir\":" + String(motionDetected ? 1 : 0) + ",";
  payload += "\"light\":" + String(lightPercent) + ",";
  payload += "\"fan\":" + String(fanOn ? 1 : 0) + ",";
  payload += "\"light_relay\":" + String(lightOn ? 1 : 0) + ",";
  payload += "\"alert\":" + String(gasAlert ? 1 : 0);
  payload += "}";
  mqttClient.publish(MQTT_TOPIC, payload.c_str());
  Serial.print(F("MQTT published: "));
  Serial.println(payload);
}

// ================= WEB DASHBOARD =================
String buildJson() {
  String j = "{";
  j += "\"temp\":" + String(temperature, 1) + ",";
  j += "\"humidity\":" + String(humidity, 1) + ",";
  j += "\"gas\":" + String(gasPercent) + ",";
  j += "\"gas_ready\":" + String(gasSensorReady ? 1 : 0) + ",";
  j += "\"pir\":" + String(motionDetected ? 1 : 0) + ",";
  j += "\"light\":" + String(lightPercent) + ",";
  j += "\"fan\":" + String(fanOn ? 1 : 0) + ",";
  j += "\"light_relay\":" + String(lightOn ? 1 : 0) + ",";
  j += "\"alert\":" + String(gasAlert ? 1 : 0);
  j += "}";
  return j;
}

String buildUptimeString() {
  unsigned long upSec = (millis() - bootTime) / 1000;
  unsigned long days = upSec / 86400;
  unsigned long hours = (upSec % 86400) / 3600;
  unsigned long mins = (upSec % 3600) / 60;
  String s = String(days) + "d " + String(hours) + "h " + String(mins) + "m";
  return s;
}

String buildHtmlPage() {
  String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='10'>";
  html += "<title>Home Automation Hub</title>";
  html += "<style>body{font-family:sans-serif;background:#111;color:#eee;text-align:center;}";
  html += ".card{display:inline-block;margin:10px;padding:15px;border-radius:10px;background:#222;min-width:120px;}";
  html += ".alert{background:#a11;}";
  html += "button{padding:10px 20px;margin:5px;border:none;border-radius:6px;font-size:14px;cursor:pointer;}";
  html += ".btn-on{background:#2a2;color:#fff;} .btn-off{background:#555;color:#fff;}";
  html += "</style></head><body>";
  html += "<h1>ESP32 Home Automation Hub</h1>";
  if (gasAlert) html += "<h2 style='color:red;'>!! GAS ALERT - RELAYS FORCED OFF !!</h2>";
  if (!gasSensorReady) html += "<h3 style='color:orange;'>Gas sensor warming up...</h3>";
  html += "<div class='card'><h3>Temp</h3><p>" + String(temperature, 1) + " C</p></div>";
  html += "<div class='card'><h3>Humidity</h3><p>" + String(humidity, 1) + " %</p></div>";
  html += "<div class='card'><h3>Gas</h3><p>" + String(gasPercent) + " %</p></div>";
  html += "<div class='card'><h3>Light</h3><p>" + String(lightPercent) + " %</p></div>";
  html += "<div class='card'><h3>Motion</h3><p>" + String(motionDetected ? "YES" : "no") + "</p></div>";
  html += "<div class='card'><h3>Uptime</h3><p>" + buildUptimeString() + "</p></div>";

  html += "<div class='card'><h3>Fan</h3><p>" + String(fanOn ? "ON" : "OFF") + "</p>";
  html += "<p style='font-size:11px;color:#999;'>" + String(fanManualOverride ? "MANUAL" : "AUTO") + "</p>";
  html += "<form action='/toggle/fan' method='GET'><button class='" + String(fanOn ? "btn-on" : "btn-off") + "' type='submit'>Toggle Fan</button></form></div>";

  html += "<div class='card'><h3>Light Relay</h3><p>" + String(lightOn ? "ON" : "OFF") + "</p>";
  html += "<p style='font-size:11px;color:#999;'>" + String(lightManualOverride ? "MANUAL" : "AUTO") + "</p>";
  html += "<form action='/toggle/light' method='GET'><button class='" + String(lightOn ? "btn-on" : "btn-off") + "' type='submit'>Toggle Light</button></form></div>";

  html += "</body></html>";
  return html;
}

void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/html", buildHtmlPage());
  });
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "application/json", buildJson());
  });
  server.on("/toggle/fan", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (!gasAlert) {
      fanOn = !fanOn;
      fanManualOverride = true;
      fanOverrideStart = millis();
      digitalWrite(RELAY_FAN, fanOn ? RELAY_ON : RELAY_OFF);
      Serial.print(F("WEB: Fan toggled to "));
      Serial.println(fanOn ? "ON" : "OFF");
    }
    req->redirect("/");
  });
  server.on("/toggle/light", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (!gasAlert) {
      lightOn = !lightOn;
      lightManualOverride = true;
      lightOverrideStart = millis();
      digitalWrite(RELAY_LIGHT, lightOn ? RELAY_ON : RELAY_OFF);
      Serial.print(F("WEB: Light toggled to "));
      Serial.println(lightOn ? "ON" : "OFF");
    }
    req->redirect("/");
  });
}

// ================= MAIN LOOP =================
void loop() {
  reconnectWiFiIfNeeded();
  reconnectMqttIfNeeded();

  handleButtons();

  if (millis() - lastSensorRead > SENSOR_INTERVAL_MS) {
    lastSensorRead = millis();
    readSensors();
    applyAutomationRules();
    logSerial();
  }

  if (millis() - lastOledSwitch > OLED_SWITCH_MS) {
    lastOledSwitch = millis();
    oledScreen = (oledScreen + 1) % 3;
  }
  updateOled();

  if (millis() - lastMqttPublish > MQTT_INTERVAL_MS) {
    lastMqttPublish = millis();
    publishMqtt();
  }
}
