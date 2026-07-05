const int buzzer = 8;

const int btn1 = 2;
const int btn2 = 3;
const int btn3 = 4;
const int btn4 = 5;
const int modeBtn = 6;

bool minorMode = false;

void setup() {
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(modeBtn, INPUT_PULLUP);
}

void loop() {

  // 🎚 Mode toggle button
  if (digitalRead(modeBtn) == LOW) {
    minorMode = !minorMode;
    delay(300); // debounce
  }

  bool b1 = digitalRead(btn1) == LOW;
  bool b2 = digitalRead(btn2) == LOW;
  bool b3 = digitalRead(btn3) == LOW;
  bool b4 = digitalRead(btn4) == LOW;

  // 🎼 Notes (Major / Minor)
  int doNote = minorMode ? 247 : 262;
  int reNote = 294;
  int miNote = minorMode ? 311 : 330;
  int faNote = 349;

  // 🎹 Chord logic
  if ((b1 + b2 + b3 + b4) >= 2) {
    tone(buzzer, 392);
  }
  else if (b1) {
    tone(buzzer, doNote);
  }
  else if (b2) {
    tone(buzzer, reNote);
  }
  else if (b3) {
    tone(buzzer, miNote);
  }
  else if (b4) {
    tone(buzzer, faNote);
  }
  else {
    noTone(buzzer);
  }
}
