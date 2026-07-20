# Project 9: Stepper Motor Precision Positioner

## Components Used
- Arduino Uno R3
- 28BYJ-48 Stepper Motor + ULN2003 Driver Board
- Potentiometer 10kΩ (target angle control)
- Push Buttons ×3 (CW, CCW, Home)
- OLED Display 0.96" I2C (SSD1306)
- 1 LED (position indicator) + 220Ω resistor
- Breadboard + jumper wires

## Wiring Summary
| Component | Pin |
|---|---|
| ULN2003 IN1 | Arduino Pin 8 |
| ULN2003 IN2 | Arduino Pin 9 |
| ULN2003 IN3 | Arduino Pin 10 |
| ULN2003 IN4 | Arduino Pin 11 |
| ULN2003 VCC | 5V |
| ULN2003 GND | GND |
| Potentiometer wiper | A0 |
| Potentiometer outer pins | 5V, GND |
| CW button | Pin 2 → GND (INPUT_PULLUP) |
| CCW button | Pin 3 → GND (INPUT_PULLUP) |
| Home button | Pin 4 → GND (INPUT_PULLUP) |
| Position LED | Pin 5 → 220Ω → LED → GND |
| OLED VCC | 3.3V (**not 5V** — will damage the display) |
| OLED GND | GND |
| OLED D0 (SCLK) | A5 |
| OLED D1 (MOSI) | A4 |
| OLED RES | Pin 7 |
| OLED DC | Pin 6 |
| OLED CS | A3 |

**On the 6-pin button:** despite having 6 legs, it's electrically only 2 nodes — most tactile buttons have 2 legs per side, internally joined for mechanical stability on a breadboard/PCB, not 6 independent signals. Use any one leg from each side: one to the Arduino pin, the other to GND.

**On the OLED:** this is the 7-pin **SPI** variant (GND, VCC, D0, D1, RES, DC, CS), not the more common 4-pin I2C one. The sketch uses Adafruit_SSD1306's software-SPI constructor so any digital pins work — this avoids clashing with pins 8–11, which are already used by the ULN2003 stepper driver.

**Note on library pin order:** the Arduino `Stepper` library expects wire order `IN1, IN3, IN2, IN4` — this is intentional and matches how the internal step sequencing is generated. Wiring it in numeric order (IN1,IN2,IN3,IN4) will make the motor buzz/vibrate without turning.

## How to Run
1. Install the `Stepper` (built-in) and `Adafruit_SSD1306` / `Adafruit_GFX` libraries via the Arduino Library Manager.
2. Wire as above.
3. Upload `stepper_positioner.ino`.
4. Open Serial Monitor at 115200 baud.
5. Turn the potentiometer to set a target angle (0–360°). Press CW/CCW to move the motor 45° per press. Press Home to zero the position reference without moving the motor.

## Expected Output
- OLED cycles live: `Current: X deg`, `Target: Y deg`, `Dir: CW/CCW/HOME`, `Steps: N`
- Serial Monitor logs CSV: `millis,currentAngle,targetAngle,dir,steps` a few times per second
- Position LED lights up when current angle is within 5° of target

## Why 4096 Steps/Revolution — and Why the Code Uses 2048

The 28BYJ-48's **internal motor shaft** genuinely needs 4096 half-steps for one full revolution of the *output* — but that number already accounts for the motor's internal **gear reduction** (roughly 64:1 inside the gearbox). The motor coil itself only requires a handful of electrical steps to go around once; the gearbox is what multiplies that into thousands of steps per output shaft revolution — that's also why the motor is slow but has high torque and fine angular resolution (360° / 4096 ≈ 0.088° per step).

This sketch uses the Arduino `Stepper` library with `STEPS_PER_REV = 2048`. The `Stepper` library's default 4-step (not half-step) sequencing for this driver combination completes a full revolution in 2048 of its own "steps" rather than 4096 — so the constant here matches the library's own step-counting convention, not the motor's raw physical step count. **Before trusting the angle numbers**, run a quick calibration check: command a known number of steps (e.g. one full `2048`) and confirm the shaft returns to its starting position. If it overshoots or undershoots, adjust `STEPS_PER_REV` until one commanded revolution matches one physical revolution.

## Gearing's Effect on Resolution
Because of the internal gear reduction, the 28BYJ-48 trades speed for precision: each electrical step advances the output shaft by a very small angle, giving much finer positioning resolution than a direct-drive stepper of the same step count — at the cost of a low top speed (this sketch's cruise speed is capped at 15 RPM; going much above that causes skipped steps under load, losing position tracking accuracy since this is an open-loop system with no encoder feedback).

## Soft-Start / Soft-Stop Acceleration Ramp (Bonus)
Each 45° button move (512 steps) is driven through a `rampedStep()` function instead of a single constant-speed `step()` call:
- **First 20 steps:** speed ramps up from 5 RPM to 15 RPM (soft start)
- **Middle steps:** cruises at 15 RPM
- **Last 20 steps:** speed ramps down from 15 RPM to 5 RPM (soft stop)
- For moves shorter than 40 steps, a symmetric triangular ramp is used instead (up then straight back down, no flat cruise section)

This avoids the sudden torque demand of instantly commanding full speed, which is what causes the motor to skip steps under mechanical load — starting and stopping gradually keeps the rotor synchronized with the commanded step sequence.

## Known Limitation
This is open-loop control — there's no feedback sensor confirming the shaft actually moved as commanded. If the motor stalls or skips steps (e.g., driven too fast, or under mechanical load), `currentSteps` will silently drift from the true physical position until a manual "Home" re-reference is performed.
