#include <DHTesp.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED SPI pins
#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     16
#define OLED_CS     5
#define OLED_RESET  17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define RED_LED    25
#define GREEN_LED  26
#define BUZZER     27

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

DHTesp dht;

float maxTemp = -1000;
float minTemp = 1000;

unsigned long lastCSVLog = 0;
unsigned long lastScreenSwitch = 0;
int screenPage = 0; // 0 = readings, 1 = comfort, 2 = max/min

void setup() {
  Serial.begin(115200);
  dht.setup(4, DHTesp::DHT22);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED init failed");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

String getComfortLabel(float t, float h) {
  if (t > 38 || h > 80) return "DANGER";
  if (t > 32) return "HOT";
  if (t < 20) return "COOL";
  return "COMFORT";
}

void loop() {
  TempAndHumidity data = dht.getTempAndHumidity();
  float t = data.temperature;
  float h = data.humidity;

  // update max/min
  if (t > maxTemp) maxTemp = t;
  if (t < minTemp) minTemp = t;

  String comfort = getComfortLabel(t, h);

  // threshold logic — LED + passive buzzer
  if (t > 38 || h > 80) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    tone(BUZZER, 1000);   // audible alert tone
    delay(1000);
    noTone(BUZZER);
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    noTone(BUZZER);
  }

  // cycle OLED screens every 5 seconds
  if (millis() - lastScreenSwitch > 5000) {
    screenPage = (screenPage + 1) % 3;
    lastScreenSwitch = millis();
  }

  display.clearDisplay();
  display.setCursor(0, 0);

  if (screenPage == 0) {
    display.println("Live Readings");
    display.print("Temp: "); display.print(t); display.println(" C");
    display.print("Hum: ");  display.print(h); display.println(" %");
  } else if (screenPage == 1) {
    display.println("Comfort Status");
    display.setTextSize(2);
    display.println(comfort);
    display.setTextSize(1);
  } else {
    display.println("Daily Max/Min");
    display.print("Max: "); display.print(maxTemp); display.println(" C");
    display.print("Min: "); display.print(minTemp); display.println(" C");
  }

  display.display();

  // CSV log every 5 seconds
  if (millis() - lastCSVLog > 5000) {
    Serial.print(millis()); Serial.print(",");
    Serial.print(t); Serial.print(",");
    Serial.print(h); Serial.print(",");
    Serial.println(comfort);
    lastCSVLog = millis();
  }

  delay(2000);
}
