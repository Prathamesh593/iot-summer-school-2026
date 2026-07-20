# ESP32 IoT Home Automation Hub

**IIT Jammu Summer School 2026 — Project 10**

A full home automation hub built on an ESP32 (WROOM-32D), combining local sensor
automation, manual overrides, a live web dashboard, MQTT publishing, and an OLED
status display.

---

## Features

- **4 sensors read every 5 seconds:** DHT22 (temp/humidity), MQ-2 (gas), PIR (motion),
  LM393 LDR (light)
- **Automation Rule 1 — Fan (Relay CH1):** AUTO ON if temp > 32°C, AUTO OFF if temp <
  28°C (hysteresis). Manual override via BOOT button (GPIO0), lasts 10 minutes.
- **Automation Rule 2 — Light (Relay CH2):** AUTO ON if dark (LDR < 20%) AND motion
  detected. AUTO OFF if bright (LDR > 60%) OR no motion for 3 minutes. Manual override
  via push button (GPIO32), lasts 10 minutes.
- **Automation Rule 3 — Gas Safety Alert:** if MQ-2 > 60%, triggers buzzer + Red LED,
  forces both relays OFF, and publishes an alert flag over MQTT. Suppressed for the
  first 2 minutes after boot while the MQ-2 heater warms up.
- **Web dashboard** (AsyncWebServer, port 80): live sensor values, automation status,
  uptime, and toggle buttons for both relays. Manual toggle buttons are disabled during
  an active gas alert for safety.
- **MQTT publishing** to `broker.hivemq.com` every 30 seconds as JSON.
- **OLED display** (SSD1306, 128x64): cycles through 3 screens every 5 seconds —
  Climate, Sensors, Relays + Uptime. Gas alert overrides the screen cycle with a
  dedicated alert screen.
- **Uptime tracking** via `millis()`, shown on both OLED and web dashboard as
  `Xd Yh Zm`.

---

## System Architecture

```
                              +---------------------------+
                              |        ESP32 (WROOM-32D)   |
                              |                             |
   Sensors ------------------>|  readSensors()              |
   - DHT22 (temp/humidity)    |      |                       |
   - MQ-2  (gas)              |      v                       |
   - PIR   (motion)           |  applyAutomationRules()      |
   - LDR   (light)            |      |         ^              |
                              |      |         |  manual       |
   Manual Inputs ------------>|      |         |  override      |
   - BOOT button (fan)        |      v         |  flags         |
   - Push button (light)      |  Relay outputs -+              |
                              |      |                          |
                              |      +--> Fan Relay (GPIO15)    |
                              |      +--> Light Relay (GPIO19)  |
                              |                                 |
                              |  Gas Alert -----> Buzzer (GPIO21)|
                              |             -----> Red LED(GPIO4)|
                              |                                 |
                              |  updateOled() ---> OLED (SPI)    |
                              |  logSerial()  ---> Serial Monitor|
                              |  publishMqtt()---> MQTT (WiFi)   |
                              |  setupWebServer()-> Web Dashboard|
                              +---------------------------+
                                     |              |
                                     v              v
                          HiveMQ MQTT Broker   Browser Dashboard
                          (broker.hivemq.com)  (http://<esp32-ip>/)
```

**Data flow, once per 5-second sensor cycle:**
1. `readSensors()` reads all 4 sensors, updates global state variables.
2. `applyAutomationRules()` checks gas safety first (highest priority), then applies
   fan and light logic, respecting any active manual overrides.
3. Relay pins are written based on the resulting `fanOn` / `lightOn` state.
4. `logSerial()` prints the full state to Serial Monitor for debugging.

**Independent of the 5s cycle:**
- `handleButtons()` — checked every loop iteration for instant response
- `updateOled()` — screen refreshed every 5s (or immediately on gas alert)
- `publishMqtt()` — publishes current state every 30s
- Web dashboard — served on-demand whenever a browser requests `/`, `/data`,
  `/toggle/fan`, or `/toggle/light`

---

## Automation Rules (Full List)

| # | Rule | Trigger | Action | Priority |
|---|------|---------|--------|----------|
| 1 | **Fan control** | Temp > 32°C | Fan relay ON | Normal |
| 1 | **Fan control** | Temp < 28°C | Fan relay OFF | Normal |
| 1 | **Fan control** | 28°C–32°C (dead zone) | No change — hysteresis prevents relay chatter near the threshold | Normal |
| 2 | **Light control** | LDR < 20% (dark) AND PIR motion detected | Light relay ON | Normal |
| 2 | **Light control** | LDR > 60% (bright) | Light relay OFF | Normal |
| 2 | **Light control** | No motion for 3 continuous minutes | Light relay OFF | Normal |
| 3 | **Gas safety alert** | MQ-2 reading > 60% (after 2-min sensor warm-up) | Buzzer ON + Red LED ON + both relays forced OFF + MQTT alert flag set | **Highest — overrides rules 1 & 2 and all manual overrides** |

**Why hysteresis on the fan?** Using a single threshold (e.g. "ON above 32°C, OFF
below 32°C") causes the relay to rapidly cycle on/off if temperature hovers right at
the line. The 28°C/32°C gap means the fan won't turn back off until temp drops a
meaningful amount below where it turned on, protecting the relay from wear.

**Why the gas alert overrides everything else:** Rule 3 is checked first in
`applyAutomationRules()` and returns immediately if triggered — it skips fan/light
logic entirely for that cycle, so a manual override on either relay is bypassed during
an active gas alert. This is intentional: gas safety must never be blocked by a user
having toggled a relay on manually.

---

## Manual Override Logic

Both relays support manual overrides that temporarily take priority over automation:

**How to trigger an override:**
- **Fan:** press the BOOT button (GPIO0), or use the "Toggle Fan" button on the web
  dashboard
- **Light:** press the push button (GPIO32), or use the "Toggle Light" button on the
  web dashboard

**What happens when triggered:**
1. The relay's state is immediately flipped (ON→OFF or OFF→ON).
2. A `*ManualOverride` flag is set to `true` for that relay.
3. A timestamp (`*OverrideStart = millis()`) is recorded.
4. While the override flag is `true`, `applyAutomationRules()` skips its normal
   auto ON/OFF checks for that relay entirely — the relay stays exactly as the user
   left it, regardless of sensor readings.

**How the override expires:**
- Every automation cycle (every 5s), the code checks whether
  `millis() - *OverrideStart > 10 minutes`.
- Once 10 minutes have passed, the override flag clears automatically and the relay
  returns to full automatic control on the very next cycle — no further action needed
  from the user.

**Button debounce and edge-detection:** both physical buttons are read using
edge-detection (only firing on the HIGH→LOW transition, not on every loop where the
pin happens to read LOW), combined with a 300ms debounce window. This prevents a
single physical press from registering as multiple toggles, and prevents a
stuck/floating pin from spamming toggles continuously.

**Interaction with gas alerts:** if a gas alert becomes active while a manual override
is in effect, the gas safety shutdown still forces the relay OFF (see Rule 3 above).
The manual override flag itself is not cleared by this — once the gas alert clears,
whichever override state (manual vs. auto) and remaining timer was in effect resumes
normally.

---

| Component        | ESP32 Pin | Notes                                   |
|-------------------|-----------|------------------------------------------|
| DHT22 DATA        | GPIO33    | Needs 4.7k–10k Ω pull-up if not built in |
| MQ-2 AO           | GPIO36    | Input-only ADC (VP). VCC must be 5V.     |
| PIR OUT           | GPIO12    | boot strapping pin, see note below       |
| LDR AO            | GPIO39    | Input-only ADC (VN)                      |
| Relay CH1 (Fan)   | GPIO15    | Board is active-LOW (LOW = relay ON)     |
| Relay CH2 (Light) | GPIO19    | Board is active-LOW (LOW = relay ON)     |
| Buzzer            | GPIO21    | Passive buzzer, driven via `tone()`      |
| Red LED           | GPIO4     | Add a 220-330 ohm resistor in series     |
| Button — Light    | GPIO32    | INPUT_PULLUP, active-LOW                 |
| Button — Fan      | GPIO0     | BOOT button, INPUT_PULLUP, active-LOW    |
| OLED (SPI)        | MOSI 23, CLK 18, DC 16, CS 5, RESET 17 | SSD1306, 128x64 |

**GPIO12 note:** this is an ESP32 boot strapping pin. If anything pulls it HIGH during
power-up it can force the wrong flash voltage and cause boot resets. If you see
unexplained resets, move PIR to a different GPIO (e.g. 14 or 27) instead.

---

## Required Libraries (Arduino IDE Library Manager)

- `WiFi` (bundled with ESP32 core)
- `ESPAsyncWebServer`
- `DHT sensor library` (Adafruit)
- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `PubSubClient`

---

## Configuration

Update these constants at the top of the sketch before uploading:

```cpp
const char* WIFI_SSID     = "your-wifi-name";
const char* WIFI_PASSWORD = "your-wifi-password";
const char* MQTT_TOPIC    = "iitjammu/[yourname]/home";  // change [yourname]
```

---

## Calibration Notes

- **LDR mapping:** LM393 modules vary — some read HIGH ADC values in darkness, others
  read LOW. This sketch is currently mapped assuming the module reads **HIGH when
  dark** (confirmed by testing on this hardware). If your module behaves the opposite
  way, swap the `map()` call in `readSensors()`:
  ```cpp
  lightPercent = map(ldrRaw, 0, 4095, 100, 0);  // current: dark = HIGH raw -> LOW %
  // or, if your module is opposite:
  lightPercent = map(ldrRaw, 0, 4095, 0, 100);
  ```
  Test by covering the sensor and watching the raw ADC value in Serial Monitor.

- **MQ-2 gas threshold (60%):** MQ-2 needs a warm-up period (built-in 2-minute delay
  in this sketch) and its baseline reading in clean air varies per unit. Watch the raw
  `gas=` value in Serial Monitor after warm-up completes and adjust the 60% threshold
  in `applyAutomationRules()` if your sensor's baseline sits meaningfully higher or
  lower.

- **Fan hysteresis (28°C / 32°C):** adjust these two constants in
  `applyAutomationRules()` to match your room/climate.

---

## Web Dashboard

- `GET /` — full HTML dashboard with live values, uptime, and relay toggle buttons
  (auto-refreshes every 10s)
- `GET /data` — raw JSON of current sensor/relay state
- `GET /toggle/fan` — manually toggles the fan relay (sets 10-min manual override)
- `GET /toggle/light` — manually toggles the light relay (sets 10-min manual override)

Toggle routes are ignored while a gas alert is active, to prevent overriding the
safety shutdown.

---

## MQTT Payload Format

Published every 30 seconds to `MQTT_TOPIC`:

```json
{
  "temp": 31.4,
  "humidity": 74.0,
  "gas": 8,
  "gas_ready": 1,
  "pir": 0,
  "light": 72,
  "fan": 1,
  "light_relay": 0,
  "alert": 0
}
```

---

## Troubleshooting

| Symptom | Likely cause |
|---|---|
| Temp/humidity always `0.00` or NaN | DHT wiring, missing pull-up resistor, or wrong DATA pin |
| Gas/light ADC stuck at `0` | Sensor not powered, or AO wire loose/on wrong pin |
| Random resets, `POWERON_RESET` mid-session | PIR (or other wire) on GPIO12 interfering with boot strapping |
| Button toggles rapidly on its own | Floating/miswired button pin |
| `invalid header: 0xffffffff` boot loop after upload | Corrupted flash write — erase flash fully and re-upload |
| Upload fails with "Failed to communicate with flash chip" | Other connected hardware or power sag during upload — disconnect peripherals and retry |

---

## Changelog

- Fixed button handling to be edge-triggered instead of level-triggered, preventing
  toggle-spam from a stuck or floating pin.
- Added 2-minute MQ-2 warm-up delay before gas alerts become active.
- Fixed inverted LDR mapping (confirmed via testing that this module reads HIGH in
  darkness).
- Added Red LED (GPIO4) alongside the buzzer for gas alerts.
- Added web dashboard toggle buttons for Fan and Light relays.
- Added uptime display to the web dashboard (previously OLED-only).
