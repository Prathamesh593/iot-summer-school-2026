#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

#define LDR_PIN A0
#define TRIG_PIN 9
#define ECHO_PIN 10

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();
}

void loop() {

  // DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // LDR
  int lightRaw = analogRead(LDR_PIN);
  int lightPercent = map(lightRaw, 0, 1023, 0, 100);

  // HC-SR04
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0343 / 2;

  Serial.println("----- SENSOR DATA -----");

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Light: ");
  Serial.print(lightPercent);
  Serial.println(" %");

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.println("-----------------------");
  Serial.println();

  delay(5000);
}
