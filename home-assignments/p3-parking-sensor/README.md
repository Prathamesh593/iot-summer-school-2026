# Project 3: Ultrasonic Parking Assistant

## Components Used
- Arduino Uno R3
- HC-SR04 Ultrasonic Sensor
- Green LED, Yellow LED, Red LED (220Ω resistors each)
- Small Passive Buzzer Module (VCC, GND, I/O)

## Wiring Diagram (text)

**HC-SR04:**
- VCC → 5V
- GND → GND
- TRIG → Pin 9
- ECHO → Pin 10

**Green LED:** Pin 2 → 220Ω → LED anode, cathode → GND
**Yellow LED:** Pin 3 → 220Ω → LED anode, cathode → GND
**Red LED:** Pin 4 → 220Ω → LED anode, cathode → GND

**Buzzer:**
- VCC → 5V
- GND → GND
- I/O → Pin 6 (PWM-capable, required for `tone()`)

## Libraries Used
None external — uses built-in Arduino functions only (`digitalWrite`, `pulseIn`, `tone`, `noTone`, `millis`).

## How to Run
1. Open `p3-parking-sensor.ino` in Arduino IDE
2. Board: Arduino Uno
3. Select correct COM port
4. Upload
5. Open Serial Monitor at 9600 baud

## Expected Output
- Distance measured via HC-SR04, averaged over 5 readings per cycle to reduce sensor noise
- Four proximity zones based on averaged distance:
  - **SAFE** (>60cm): Green LED on, silent
  - **CAUTION** (30–60cm): Yellow LED on, beep every 800ms
  - **CLOSE** (15–30cm): Yellow + Red LEDs on, beep every 300ms
  - **DANGER** (<15cm): Red LED only, continuous buzzer tone
- Serial Monitor logs distance and current zone every ~300ms
- All timing (beep intervals, main loop) uses `millis()` — no blocking `delay()` in the zone/beep logic, so sensor readings stay responsive

## Physics of the Distance Calculation
The HC-SR04 emits an ultrasonic pulse via TRIG and measures how long it takes for the echo to return on the ECHO pin, using `pulseIn()`. Since sound travels at approximately 343 m/s (0.034 cm/microsecond) at room temperature, and the pulse travels to the object and back (round trip), the distance is calculated as:

`distance = (duration_in_microseconds × 0.034) / 2`

The division by 2 accounts for the round trip — the raw duration measures the full there-and-back time, but the actual distance to the object is only half that path.

## Build Notes
- Buzzer is a passive buzzer module, driven using `tone()`/`noTone()` on a PWM-capable pin.
- Board used is Arduino Uno, so HC-SR04 is wired directly at 5V logic (no voltage divider needed — that precaution applies only to 3.3V boards).
- OLED display was optional per the assignment and was not included in this implementation; distance and zone status are reported via Serial Monitor and the LED/buzzer indicators instead.
