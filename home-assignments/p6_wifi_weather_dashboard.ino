#include <DHTesp.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
...
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

const char* ssid = "Redmi";
const char* password = "Pratham@21";

#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     16
#define OLED_CS     5
#define OLED_RESET  17
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

AsyncWebServer server(80);
DHTesp dht;
Adafruit_BMP280 bmp;

float g_temp = 0, g_hum = 0, g_pressure = 0;
int g_light = 0;

#define HISTORY_SIZE 24
float tempHistory[HISTORY_SIZE];
int historyIndex = 0;
int historyCount = 0;

unsigned long lastSensorRead = 0;
unsigned long lastHistoryLog = 0;
unsigned long lastWifiCheck = 0;

const unsigned long SENSOR_INTERVAL = 5000;
const unsigned long HISTORY_INTERVAL = 10000; // 5 min (use 10000 temporarily for fast testing)

String buildHtmlPage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='30'>";
  html += "<title>Weather Dashboard</title>";
  html += "<script src='https://cdnjs.cloudflare.com/ajax/libs/Chart.js/4.4.0/chart.umd.min.js'></script>";
  html += "<style>";
  html += "body{font-family:Arial;text-align:center;background:#222;color:white;}";
  html += ".card{display:inline-block;margin:10px;padding:20px;border-radius:10px;width:150px;}";
  html += ".hot{background:#e74c3c;} .cool{background:#3498db;} .normal{background:#2ecc71;}";
  html += "canvas{max-width:600px;margin:20px auto;background:white;border-radius:10px;padding:10px;}";
  html += "</style></head><body>";
  html += "<h1>ESP32 Weather Dashboard</h1>";

  String tempClass = (g_temp > 30) ? "hot" : (g_temp < 20) ? "cool" : "normal";

  html += "<div class='card " + tempClass + "'><h3>Temperature</h3><p>" + String(g_temp) + " C</p></div>";
  html += "<div class='card normal'><h3>Humidity</h3><p>" + String(g_hum) + " %</p></div>";
  html += "<div class='card normal'><h3>Pressure</h3><p>" + String(g_pressure) + " hPa</p></div>";
  html += "<div class='card normal'><h3>Light</h3><p>" + String(g_light) + " %</p></div>";

  html += "<canvas id='tempChart'></canvas>";

  html += "<script>";
  html += "fetch('/history').then(r=>r.json()).then(data=>{";
  html += "new Chart(document.getElementById('tempChart'),{";
  html += "type:'line',";
  html += "data:{labels:data.labels,datasets:[{label:'Temperature (C)',data:data.temps,borderColor:'red',fill:false,tension:0.2}]},";
  html += "options:{scales:{y:{beginAtZero:false}}}";
  html += "});});";
  html += "</script>";

  html += "</body></html>";
  return html;
}

String buildJson() {
  String json = "{";
  json += "\"temp\":" + String(g_temp) + ",";
  json += "\"humidity\":" + String(g_hum) + ",";
  json += "\"pressure\":" + String(g_pressure) + ",";
  json += "\"light\":" + String(g_light);
  json += "}";
  return json;
}

String buildHistoryJson() {
  String json = "{\"labels\":[";
  for (int i = 0; i < historyCount; i++) {
    json += "\"" + String((historyCount - i) * -5) + "min\"";
    if (i < historyCount - 1) json += ",";
  }
  json += "],\"temps\":[";
  for (int i = 0; i < historyCount; i++) {
    int idx = (historyIndex - historyCount + i + HISTORY_SIZE) % HISTORY_SIZE;
    json += String(tempHistory[idx]);
    if (i < historyCount - 1) json += ",";
  }
  json += "]}";
  return json;
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  dht.setup(4, DHTesp::DHT22);
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found!");
    while (true);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED init failed");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  connectWiFi();

  display.clearDisplay();
  display.setCursor(0, 0);
  if (WiFi.status() == WL_CONNECTED) {
    display.println("WiFi Connected");
    display.print("SSID: "); display.println(ssid);
    display.print("IP: "); display.println(WiFi.localIP());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    display.println("WiFi FAILED");
    Serial.println("WiFi connection failed");
  }
  display.display();
  delay(3000);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", buildHtmlPage());
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", buildJson());
  });

  server.on("/history", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", buildHistoryJson());
  });

  server.begin();
}

void loop() {
  if (millis() - lastWifiCheck > 30000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, reconnecting...");
      connectWiFi();
    }
    lastWifiCheck = millis();
  }

  if (millis() - lastSensorRead > SENSOR_INTERVAL) {
    TempAndHumidity data = dht.getTempAndHumidity();
    g_temp = data.temperature;
    g_hum = data.humidity;
    g_pressure = bmp.readPressure() / 100.0F;

    int lightRaw = analogRead(34);
    g_light = map(lightRaw, 0, 4095, 0, 100);

    Serial.print("Temp: "); Serial.print(g_temp);
    Serial.print("  Hum: "); Serial.print(g_hum);
    Serial.print("  Pressure: "); Serial.print(g_pressure);
    Serial.print("  Light: "); Serial.println(g_light);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Temp: "); display.print(g_temp); display.println(" C");
    display.print("Hum: "); display.print(g_hum); display.println(" %");
    display.print("Pressure: "); display.println(g_pressure);
    display.print("Light: "); display.print(g_light); display.println(" %");
    display.display();

    lastSensorRead = millis();
  }

  if (millis() - lastHistoryLog > HISTORY_INTERVAL) {
    tempHistory[historyIndex] = g_temp;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) historyCount++;
    lastHistoryLog = millis();
  }
}