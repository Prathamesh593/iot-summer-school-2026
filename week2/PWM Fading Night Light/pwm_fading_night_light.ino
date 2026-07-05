const int ledPin = 9;
const int buttonPin = 2;

int mode = 1;
bool lastButton = HIGH;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("Mode 1: Slow Breathing");
}

void loop() {

  bool button = digitalRead(buttonPin);

  if (lastButton == HIGH && button == LOW) {
    mode++;
    if (mode > 3) mode = 1;

    if (mode == 1)
      Serial.println("Mode 1: Slow Breathing");
    else if (mode == 2)
      Serial.println("Mode 2: Fast Pulse");
    else
      Serial.println("Mode 3: SOS");

    delay(300);
  }

  lastButton = button;

  if (mode == 1) {
    breathe(15);
  }
  else if (mode == 2) {
    breathe(2);
  }
  else {
    sos();
  }
}

void breathe(int d) {

  for (int i = 0; i <= 255; i++) {
    analogWrite(ledPin, i);
    delay(d);
  }

  for (int i = 255; i >= 0; i--) {
    analogWrite(ledPin, i);
    delay(d);
  }

}

void sos() {

  dot();
  dot();
  dot();

  dash();
  dash();
  dash();

  dot();
  dot();
  dot();

  delay(1000);
}

void dot() {
  analogWrite(ledPin, 255);
  delay(200);
  analogWrite(ledPin, 0);
  delay(200);
}

void dash() {
  analogWrite(ledPin, 255);
  delay(600);
  analogWrite(ledPin, 0);
  delay(200);
}
