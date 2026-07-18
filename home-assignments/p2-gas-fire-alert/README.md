# Project 2: Gas & Fire Safety Alert System

## Components Used
- Arduino Uno R3
- MQ-2 Gas Sensor Module
- IR Infrared Flame Sensor Module (3-pin: VCC, GND, OUT)
- Green LED, Yellow LED, Red LED (220Ω resistors each)
- Small Passive Buzzer Module (VCC, GND, I/O)
- 6-pin Tactile Push Button (silence override)

## Wiring Diagram (text)

**MQ-2:**
- VCC → 5V
- GND → GND
- AO → A0

**Flame Sensor:**
- VCC → 5V
- GND → GND
- OUT → Pin 7

**Green LED:** Pin 10 → 220Ω → LED anode, cathode → GND
**Yellow LED:** Pin 11 → 220Ω → LED anode, cathode → GND
**Red LED:** Pin 12 → 220Ω → LED anode, cathode → GND

**Buzzer:**
- VCC → 5V
- GND → GND
- I/O → Pin 9

**Silence Button:**
- One leg → Pin 2
- Opposite-side leg → GND
- Uses internal pull-up (`INPUT_PULLUP`), no external resistor needed

## Libraries Used
None external — uses built-in Arduino functions only (`analogRead`, `digitalRead`, `tone`, `noTone`, `millis`).

## How to Run
1. Open `p2-gas-fire-alert.ino` in Arduino IDE
2. Board: Arduino Uno
3. Select correct COM port
4. Upload
5. Open Serial Monitor at 9600 baud

## Expected Output
- On boot, a 90-second MQ-2 warm-up period runs (Serial prints a warm-up message) before sensing logic begins
- Three states based on gas% and flame detection:
  - **SAFE** (<30% gas, no flame): Green LED on, silent
  - **WARNING** (30–60% gas): Yellow LED on, buzzer beeps once per second
  - **DANGER** (>60% gas OR flame detected): Red LED on, buzzer sounds continuously
- Serial Monitor logs every 200ms: `GAS: X% | FLAME: YES/NONE | STATUS: SAFE/WARNING/DANGER`
- Pressing the silence button during DANGER mutes the buzzer for 30 seconds while keeping the red LED active; the danger condition is re-checked every loop, so the alarm auto-reactivates immediately if still in danger after the mute period, or clears immediately if the danger condition ends before the 30 seconds are up

## Why the MQ-2 Warm-Up Matters
The MQ-2 sensor uses a heated metal-oxide element to detect gas concentration. Immediately after power-on, this element hasn't reached its stable operating temperature yet, so raw readings climb steadily and unpredictably for roughly 1–2 minutes before settling into a reliable baseline. Trusting readings during this window can produce false WARNING/DANGER triggers unrelated to any actual gas presence. This project adds a fixed 90-second warm-up delay in `setup()` before any sensing logic runs, to avoid exactly this issue.

## Why the Flame Sensor Is Active-Low
The flame sensor's OUT pin is driven by an onboard LM393 comparator, which pulls the output LOW when the sensed infrared intensity crosses the threshold set by its onboard potentiometer (i.e., when a flame or strong IR source is detected). When no flame is present, the comparator output rests HIGH. This is the reverse of what's often intuitively expected (HIGH = triggered), so the code explicitly checks for `LOW` to mean "flame detected": `bool flameDetected = (digitalRead(FLAME_PIN) == LOW);`

## Build Notes
- Buzzer is a passive buzzer module (not active) — driven using `tone()`/`noTone()` rather than `digitalWrite()`.
- Flame sensor sensitivity required manual trimmer calibration to avoid false triggers from ambient room lighting.
- Silence button is a 6-pin tactile switch; only 2 of the 6 legs (one from each internally-connected side) are wired.
