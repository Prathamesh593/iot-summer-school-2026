#include <DHTesp.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SOIL_PIN   34
#define DHT_PIN    4
#define RELAY2     27
#define OVERRIDE_BTN 0

#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     16
#define OLED_CS     5
#define OLED_RESET  17
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

DHTesp dht;

bool pumpOn = false;
bool manualOverride = false;
unsigned long overrideStart = 0;
const unsigned long OVERRIDE_DURATION = 5000;

void setup() {
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  pinMode(RELAY2, OUTPUT);
  pinMode(OVERRIDE_BTN, INPUT_PULLUP);
  digitalWrite(RELAY2, HIGH); // start OFF

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED init failed");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void loop() {
  int rawMoisture = analogRead(SOIL_PIN);
  int moisturePercent = map(rawMoisture, 2300, 600, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  TempAndHumidity data = dht.getTempAndHumidity();

  if (digitalRead(OVERRIDE_BTN) == LOW && !manualOverride) {
    manualOverride = true;
    overrideStart = millis();
    Serial.println("Manual override: force-watering for 5s");
  }

  if (manualOverride) {
    digitalWrite(RELAY2, LOW);
    if (millis() - overrideStart > OVERRIDE_DURATION) {
      manualOverride = false;
    }
  } else {
    if (!pumpOn && moisturePercent < 30) {
      pumpOn = true;
      digitalWrite(RELAY2, LOW);
    }
    if (pumpOn && moisturePercent > 40) {
      pumpOn = false;
      digitalWrite(RELAY2, HIGH);
    }
  }

  String soilStatus = (moisturePercent < 30) ? "DRY" : (moisturePercent > 70) ? "WET" : "OPTIMAL";
  bool pumpActive = pumpOn || manualOverride;

  // OLED display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Soil: "); display.print(moisturePercent); display.println("%");
  display.print("Temp: "); display.print(data.temperature); display.println("C");
  display.print("Status: "); display.println(soilStatus);
  display.print("Pump: "); display.println(pumpActive ? "ON" : "OFF");

  // simple bar graph for moisture
  int barWidth = map(moisturePercent, 0, 100, 0, 100);
  display.drawRect(0, 50, 104, 10, SSD1306_WHITE);
  display.fillRect(2, 52, barWidth, 6, SSD1306_WHITE);

  display.display();

  Serial.print("Soil: ");
  Serial.print(moisturePercent);
  Serial.print("% | Temp: ");
  Serial.print(data.temperature);
  Serial.print("C | Status: ");
  Serial.print(soilStatus);
  Serial.print(" | Pump: ");
  Serial.println(pumpActive ? "ON" : "OFF");

  delay(2000);
}
