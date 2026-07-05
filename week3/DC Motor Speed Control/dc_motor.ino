const int pot = A0;
const int dirBtn = 2;
const int startBtn = 3;

const int enablePin = 9;
const int motorPin1 = 7;
const int motorPin2 = 8;

bool motorOn = true;
bool forward = true;

bool oldDirBtn = HIGH;
bool oldStartBtn = HIGH;

void setup() {
  Serial.begin(9600);

  pinMode(dirBtn, INPUT_PULLUP);
  pinMode(startBtn, INPUT_PULLUP);

  pinMode(enablePin, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);

  setDirection();
}

void loop() {

  bool dirState = digitalRead(dirBtn);
  bool startState = digitalRead(startBtn);

  if (dirState == LOW && oldDirBtn == HIGH) {
    forward = !forward;
    setDirection();
    delay(50);
  }
  oldDirBtn = dirState;

  if (startState == LOW && oldStartBtn == HIGH) {
    motorOn = !motorOn;
    delay(50);
  }
  oldStartBtn = startState;

  int value = analogRead(pot);
  int speed = map(value, 0, 1023, 0, 255);
  int percent = map(speed, 0, 255, 0, 100);

  if (motorOn) {
    analogWrite(enablePin, speed);
  } else {
    analogWrite(enablePin, 0);
  }

  showData(percent);

  delay(100);
}

void setDirection() {
  if (forward) {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
  } else {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
  }
}

void showData(int percent) {

  Serial.print("Motor: ");

  if (motorOn) {
    Serial.print("ON");
  } else {
    Serial.print("OFF");
  }

  Serial.print(" | Direction: ");

  if (forward) {
    Serial.print("Forward");
  } else {
    Serial.print("Reverse");
  }

  Serial.print(" | Speed: ");

  if (motorOn) {
    Serial.print(percent);
    Serial.println("%");
  } else {
    Serial.println("0%");
  }
}
