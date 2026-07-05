#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

const int redLED = 8;
const int greenLED = 9;

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  Serial.println("timestamp,temp_C,temp_F,humidity");
}

void loop() {
  float tempC = dht.readTemperature();
  float tempF = dht.readTemperature(true);
  float humidity = dht.readHumidity();

  if (isnan(tempC) || isnan(humidity)) {
    Serial.println("Failed to read from DHT22!");
    delay(2000);
    return;
  }

  Serial.print(millis());
  Serial.print(",");
  Serial.print(tempC);
  Serial.print(",");
  Serial.print(tempF);
  Serial.print(",");
  Serial.println(humidity);

  if (tempC > 35 || humidity > 80) {
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
  } else {
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
  }

  delay(2000);
}
