// Traffic Light Controller

const int redLED = 2;
const int yellowLED = 3;
const int greenLED = 4;
const int buttonPin = 7;

void setup() {
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {

  // Pedestrian Button
  if (digitalRead(buttonPin) == LOW) {
    digitalWrite(redLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, LOW);

    Serial.print(millis());
    Serial.println(" ms : RED (Pedestrian)");

    delay(8000);
    return;
  }

  // RED
  digitalWrite(redLED, HIGH);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);

  Serial.print(millis());
  Serial.println(" ms : RED");

  delay(5000);

  // YELLOW
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, HIGH);
  digitalWrite(greenLED, LOW);

  Serial.print(millis());
  Serial.println(" ms : YELLOW");

  delay(2000);

  // GREEN
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, HIGH);

  Serial.print(millis());
  Serial.println(" ms : GREEN");

  delay(4000);
}
