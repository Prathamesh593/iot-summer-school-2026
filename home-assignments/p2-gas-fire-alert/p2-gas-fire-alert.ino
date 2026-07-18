#define MQ2_PIN     A0
#define FLAME_PIN   7
#define GREEN_LED   10
#define YELLOW_LED  11
#define RED_LED     12
#define BUZZER      9
#define SILENCE_BTN 2

unsigned long lastBeep = 0;
bool beepState = false;

bool silenced = false;
unsigned long silenceStart = 0;
const unsigned long SILENCE_DURATION = 30000; // 30 seconds

void setup() {
  Serial.begin(9600);
  pinMode(FLAME_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SILENCE_BTN, INPUT_PULLUP);

  Serial.println("Warming up MQ-2, please wait...");
  delay(90000); // 90 second warm-up, runs once at boot only
  Serial.println("Warm-up complete, monitoring started.");
}

void loop() {
  int gasValue = analogRead(MQ2_PIN);
  int gasPercent = map(gasValue, 0, 1023, 0, 100);
  bool flameDetected = (digitalRead(FLAME_PIN) == LOW);

  String status;
  bool isDanger = (gasPercent > 60 || flameDetected);
  bool isWarning = (!isDanger && gasPercent > 30);

  if (digitalRead(SILENCE_BTN) == LOW && !silenced) {
    silenced = true;
    silenceStart = millis();
  }

  if (silenced && millis() - silenceStart > SILENCE_DURATION) {
    silenced = false;
  }

  if (isDanger) {
    status = "DANGER";
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    if (!silenced) {
      tone(BUZZER, 1500);
    } else {
      noTone(BUZZER);
    }
  }
  else if (isWarning) {
    status = "WARNING";
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    silenced = false;

    if (millis() - lastBeep > 1000) {
      beepState = !beepState;
      if (beepState) tone(BUZZER, 1000);
      else noTone(BUZZER);
      lastBeep = millis();
    }
  }
  else {
    status = "SAFE";
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);
    silenced = false;
  }

  Serial.print("GAS: ");
  Serial.print(gasPercent);
  Serial.print("% | FLAME: ");
  Serial.print(flameDetected ? "YES" : "NONE");
  Serial.print(" | STATUS: ");
  Serial.print(status);
  Serial.println(silenced ? " (SILENCED)" : "");

  delay(200);
}
