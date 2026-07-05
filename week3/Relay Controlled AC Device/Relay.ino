#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

const int ledPin = 8;      // LED = Relay/AC
const int buttonPin = 7;   // Manual Override Button

bool relayState = false;
bool manualOverride = false;
bool lastButtonState = HIGH;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(ledPin, LOW);

  Serial.begin(9600);
  dht.begin();
}

void loop() {
  float temp = dht.readTemperature();

  // Button Press Detection
  bool currentButton = digitalRead(buttonPin);

  if (currentButton == LOW && lastButtonState == HIGH) {
    manualOverride = !manualOverride;

    if (manualOverride) {
      relayState = !relayState;
      digitalWrite(ledPin, relayState);

      Serial.print("Manual Override -> Relay ");
      Serial.println(relayState ? "ON" : "OFF");
    } else {
      Serial.println("Manual Override OFF (Auto Mode)");
    }

    delay(200);
  }

  lastButtonState = currentButton;

  // Automatic Control (Only when Manual Override is OFF)
  if (!manualOverride && !isnan(temp)) {

    if (temp > 32 && !relayState) {
      relayState = true;
      digitalWrite(ledPin, HIGH);

      Serial.print("Temperature: ");
      Serial.print(temp);
      Serial.println(" C -> Relay ON (AC ON)");
    }

    if (temp < 28 && relayState) {
      relayState = false;
      digitalWrite(ledPin, LOW);

      Serial.print("Temperature: ");
      Serial.print(temp);
      Serial.println(" C -> Relay OFF (AC OFF)");
    }
  }

  // Show Temperature Every Second
  if (!isnan(temp)) {
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print(" C | Relay: ");
    Serial.println(relayState ? "ON" : "OFF");
  }

  delay(1000);
}
