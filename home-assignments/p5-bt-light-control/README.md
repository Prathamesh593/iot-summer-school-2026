# Project 5: Bluetooth Home Light Controller

## Components Used
- ESP32 Dev Module (30-pin) — built-in Classic Bluetooth
- 5V 2-Channel SSR Relay Module (Low Level Trigger, with fuse)
- Small Passive Buzzer Module (VCC, GND, I/O)
- 6-pin Tactile Push Button (external, Light 2 override)

## Wiring Diagram (text)

**Relay:**
- DC+ → VIN (5V from USB)
- DC- → GND
- CH1 → GPIO26 (Light 2)
- CH2 → GPIO27 (Light 1)

**Buzzer:**
- VCC → 3.3V
- GND → GND
- I/O → GPIO25

**Light 1 Manual Override:**
- ESP32's onboard BOOT button (GPIO0) — no external wiring required

**Light 2 Manual Override (external button):**
- One leg → GPIO35, same row as one leg of a 10kΩ resistor
- Other leg of resistor → GND (pull-down)
- Button's opposite leg → 3.3V
- (This pin uses an external pull-down since GPIO35 doesn't reliably support internal pulls on all ESP32 variants)

**Bluetooth Connected Indicator:**
- ESP32's onboard built-in LED (GPIO2) — no external wiring required

## Libraries Used
- BluetoothSerial (built into ESP32 Arduino core, no separate install needed)

## How to Run
1. Open `p5-bt-light-control.ino` in Arduino IDE
2. Board: ESP32 Dev Module
3. Select correct COM port
4. Upload
5. On phone: install "Serial Bluetooth Terminal" (Play Store)
6. Pair phone with Bluetooth device named "IIT_IoT_HomeCtrl" in phone's Bluetooth settings
7. Open the app, connect to the paired device

## Supported Commands (sent from phone via Bluetooth terminal)
| Command | Action |
|---|---|
| `1` | Light 1 ON |
| `2` | Light 1 OFF |
| `3` | Light 2 ON |
| `4` | Light 2 OFF |
| `5` | ALL Lights ON |
| `6` | ALL Lights OFF |
| `?` | Status query (no state change) |

Each valid toggle command triggers a 50ms buzzer beep and sends back a status string, e.g. `L1:ON L2:OFF`.

## Expected Output
- Relay channels switch lights ON/OFF based on Bluetooth commands or physical button presses
- Buzzer beeps briefly (50ms) on every toggle command (not on status queries)
- Status string sent back to phone after every toggle
- Onboard LED (GPIO2) lights up whenever a Bluetooth client is actively connected
- If no Bluetooth command is received for 30 minutes, both lights automatically turn OFF (energy saving)
- Physical buttons (BOOT for Light 1, external GPIO35 button for Light 2) work independently of Bluetooth, toggling their respective light and broadcasting a status update

## Build Notes
- Relay is a Solid State Relay (SSR), confirmed active-LOW ("Low Level Trigger" printed on board).
- GPIO35 button uses an external pull-down resistor rather than `INPUT_PULLUP`, since this pin doesn't reliably support internal pull resistors on all ESP32 board variants — resting state reads LOW, pressed state reads HIGH (opposite polarity from the BOOT button's pull-up wiring).
- Bonus MIT App Inventor / Blynk app was not implemented for this submission; control was tested and demonstrated via the Serial Bluetooth Terminal Android app instead.
