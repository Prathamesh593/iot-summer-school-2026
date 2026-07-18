#define TRIG_PIN   9
#define ECHO_PIN   10
#define GREEN_LED  2
#define YELLOW_LED 3
#define RED_LED    4
#define BUZZER     6

unsigned long lastBeep = 0;
bool beepState = false;

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  float readings[5];
  for (int i = 0; i < 5; i++) {
    readings[i] = measureDistance();
    delay(20);
  }
  float sum = 0;
  for (int i = 0; i < 5; i++) sum += readings[i];
  float avgDistance = sum / 5;

  String zone;
  unsigned long beepInterval = 0;

  if (avgDistance > 60) {
    zone = "SAFE";
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);
  }
  else if (avgDistance > 30) {
    zone = "CAUTION";
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    beepInterval = 800;
  }
  else if (avgDistance > 15) {
    zone = "CLOSE";
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    beepInterval = 300;
  }
  else {
    zone = "DANGER";
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 1200); // continuous
  }

  // non-blocking beep for CAUTION/CLOSE zones
  if (beepInterval > 0) {
    if (millis() - lastBeep > beepInterval) {
      beepState = !beepState;
      if (beepState) tone(BUZZER, 1000);
      else noTone(BUZZER);
      lastBeep = millis();
    }
  }

  Serial.print("Distance: ");
  Serial.print(avgDistance);
  Serial.print(" cm | Zone: ");
  Serial.println(zone);

  delay(300);
}
