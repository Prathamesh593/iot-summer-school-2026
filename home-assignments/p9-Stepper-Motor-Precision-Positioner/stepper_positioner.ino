/*
  Project 9: Stepper Motor Precision Positioner
  Arduino Uno R3 + 28BYJ-48 Stepper + ULN2003 Driver

  - Potentiometer (A0) sets TARGET angle (0-360 deg)
  - CW button:  rotate +45 deg (512 steps) clockwise
  - CCW button: rotate -45 deg (512 steps) counter-clockwise
  - HOME button: sets current position as logical 0 (motor does not move)
  - OLED shows: Current angle | Target angle | Direction | Steps taken

  28BYJ-48 with ULN2003 in half-step mode = 4096 steps/revolution (8 steps/cycle).
  This sketch uses the standard Stepper library at 2048 steps/rev (full-step
  equivalent via the library's internal 4-wire sequencing), which is the
  commonly used convention with this library for the 28BYJ-48. Adjust
  STEPS_PER_REV if your library/driver combination behaves differently --
  verify with a known full 360 deg rotation test first.

  BONUS: soft-start / soft-stop acceleration ramp on CW/CCW moves.
  Ramps 5->15 RPM over the first 20 steps, cruises at 15 RPM, then
  ramps 15->5 RPM over the last 20 steps. Prevents step skipping
  under load.
*/

#include <Stepper.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED (7-pin SPI variant: GND,VCC,D0,D1,RES,DC,CS) ----------
// D0 = SPI clock (SCLK), D1 = SPI data (MOSI)
// Using software SPI (any digital pins ok) to avoid clashing with the
// stepper driver pins, which already occupy 8,9,10,11.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI  A4   // -> D1 on OLED
#define OLED_CLK   A5   // -> D0 on OLED
#define OLED_DC    6
#define OLED_CS    A3
#define OLED_RESET 7
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                          OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// ---------- Stepper ----------
const int STEPS_PER_REV = 2048;     // see note above
Stepper myStepper(STEPS_PER_REV, 8, 10, 9, 11); // IN1,IN3,IN2,IN4 order for this library
const int STEP_RPM = 12;            // default/cruise speed for non-ramped calls
const int STEP_PER_PRESS = STEPS_PER_REV / 8; // 45 degrees = 512 steps @2048/rev

// ---------- Ramp settings (bonus) ----------
const int RAMP_STEPS = 20;   // steps to accelerate / decelerate over
const int RAMP_MIN_RPM = 5;
const int RAMP_MAX_RPM = 15;

// ---------- Pins ----------
const int POT_PIN   = A0;
const int BTN_CW     = 2;
const int BTN_CCW    = 3;
const int BTN_HOME   = 4;
const int POS_LED    = 5;   // lights when at (or very near) target

// ---------- State ----------
long currentSteps = 0;      // absolute step count, can go negative
int  targetAngle  = 0;      // 0-360, from potentiometer
String lastDir = "---";

// simple debounce
unsigned long lastCwPress = 0, lastCcwPress = 0, lastHomePress = 0;
const unsigned long DEBOUNCE_MS = 200;

void setup() {
  Serial.begin(115200);

  pinMode(BTN_CW, INPUT_PULLUP);
  pinMode(BTN_CCW, INPUT_PULLUP);
  pinMode(BTN_HOME, INPUT_PULLUP);
  pinMode(POS_LED, OUTPUT);

  myStepper.setSpeed(STEP_RPM);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("OLED init failed - check wiring (D0/D1/RES/DC/CS)"));
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Stepper Positioner"));
  display.println(F("Ready."));
  display.display();
  delay(1000);

  Serial.println(F("=== Stepper Positioner Ready ==="));
  Serial.println(F("millis,currentAngle,targetAngle,dir,steps"));
}

float stepsToAngle(long steps) {
  // angle wraps 0-360, handles negative steps correctly
  long mod = steps % STEPS_PER_REV;
  if (mod < 0) mod += STEPS_PER_REV;
  return mod * 360.0 / STEPS_PER_REV;
}

void updateTargetFromPot() {
  int raw = analogRead(POT_PIN);           // 0-1023 (Uno = 10-bit ADC)
  targetAngle = map(raw, 0, 1023, 0, 360);
}

// Moves 'totalSteps' (sign = direction) with soft-start/soft-stop.
// Ramps RAMP_MIN_RPM->RAMP_MAX_RPM over first RAMP_STEPS steps,
// cruises at RAMP_MAX_RPM in the middle, then ramps back down to
// RAMP_MIN_RPM over the last RAMP_STEPS steps. For very short moves
// (fewer than 2*RAMP_STEPS), uses a symmetric triangular profile instead.
void rampedStep(long totalSteps) {
  int dir = (totalSteps > 0) ? 1 : -1;
  long remaining = abs(totalSteps);

  long stepsDone = 0;
  while (stepsDone < remaining) {
    int rpm;

    if (remaining > 2 * RAMP_STEPS) {
      if (stepsDone < RAMP_STEPS) {
        // accelerating phase
        rpm = map(stepsDone, 0, RAMP_STEPS - 1, RAMP_MIN_RPM, RAMP_MAX_RPM);
      } else if (remaining - stepsDone <= RAMP_STEPS) {
        // decelerating phase
        long stepsFromEnd = remaining - stepsDone;
        rpm = map(stepsFromEnd, 1, RAMP_STEPS, RAMP_MIN_RPM, RAMP_MAX_RPM);
      } else {
        rpm = RAMP_MAX_RPM; // cruising phase
      }
    } else {
      // move too short for a full ramp: simple symmetric triangular profile
      long half = remaining / 2;
      long distFromEdge = min(stepsDone, remaining - stepsDone - 1);
      rpm = map(distFromEdge, 0, max(half - 1, 1), RAMP_MIN_RPM, RAMP_MAX_RPM);
    }

    rpm = constrain(rpm, RAMP_MIN_RPM, RAMP_MAX_RPM);
    myStepper.setSpeed(rpm);
    myStepper.step(dir);   // one physical step at a time so speed can vary mid-move
    stepsDone++;
  }

  myStepper.setSpeed(STEP_RPM); // restore default cruise speed
}

void handleButtons() {
  unsigned long now = millis();

  if (digitalRead(BTN_CW) == LOW && (now - lastCwPress) > DEBOUNCE_MS) {
    lastCwPress = now;
    Serial.println(F("CW button pressed: +45 deg (ramped)"));
    rampedStep(STEP_PER_PRESS);
    currentSteps += STEP_PER_PRESS;
    lastDir = "CW";
  }

  if (digitalRead(BTN_CCW) == LOW && (now - lastCcwPress) > DEBOUNCE_MS) {
    lastCcwPress = now;
    Serial.println(F("CCW button pressed: -45 deg (ramped)"));
    rampedStep(-STEP_PER_PRESS);
    currentSteps -= STEP_PER_PRESS;
    lastDir = "CCW";
  }

  if (digitalRead(BTN_HOME) == LOW && (now - lastHomePress) > DEBOUNCE_MS) {
    lastHomePress = now;
    Serial.println(F("HOME button pressed: zeroing position (no motor move)"));
    currentSteps = 0;
    lastDir = "HOME";
  }
}

void updateDisplay(float currentAngle) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("Current: "));
  display.print(currentAngle, 0);
  display.println(F(" deg"));

  display.print(F("Target:  "));
  display.print(targetAngle);
  display.println(F(" deg"));

  display.print(F("Dir: "));
  display.println(lastDir);

  display.print(F("Steps: "));
  display.println(currentSteps);

  display.display();
}

void logSerial(float currentAngle) {
  Serial.print(millis());
  Serial.print(",");
  Serial.print(currentAngle, 1);
  Serial.print(",");
  Serial.print(targetAngle);
  Serial.print(",");
  Serial.print(lastDir);
  Serial.print(",");
  Serial.println(currentSteps);
}

void loop() {
  updateTargetFromPot();
  handleButtons();

  float currentAngle = stepsToAngle(currentSteps);

  // Position indicator LED: on when current angle is within 5 degrees of target
  float diff = fabs(currentAngle - targetAngle);
  diff = min(diff, 360.0f - diff); // handle wraparound (e.g. 359 vs 2 deg)
  digitalWrite(POS_LED, diff <= 5.0 ? HIGH : LOW);

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 300) { // refresh OLED/serial a few times/sec, non-blocking
    updateDisplay(currentAngle);
    logSerial(currentAngle);
    lastUpdate = millis();
  }
}