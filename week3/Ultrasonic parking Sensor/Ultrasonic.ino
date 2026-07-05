const int trigPin = 9;
const int echoPin = 10;
const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;
const int buzzer = 8;

unsigned long previousMillis = 0;
bool state = false;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  pinMode(buzzer, OUTPUT);
}

void loop() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);

  float distance = (duration * 0.034) / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  unsigned long currentMillis = millis();

  if (distance > 50) {

    Serial.println("SAFE");

    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    digitalWrite(buzzer, LOW);

  }

  else if (distance > 20) {

    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);

    if (currentMillis - previousMillis >= 500) {
      previousMillis = currentMillis;
      state = !state;
      digitalWrite(buzzer, state);
    }

  }

  else if (distance > 10) {

    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);

    if (currentMillis - previousMillis >= 200) {
      previousMillis = currentMillis;
      state = !state;
      digitalWrite(buzzer, state);
    }

  }

  else {

    if (currentMillis - previousMillis >= 100) {
      previousMillis = currentMillis;
      state = !state;

      digitalWrite(greenLED, state);
      digitalWrite(yellowLED, state);
      digitalWrite(redLED, state);
    }

    digitalWrite(buzzer, HIGH);

  }

  delay(50);
}
