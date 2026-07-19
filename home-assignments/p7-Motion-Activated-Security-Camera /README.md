# Project 7: Motion-Activated Security Camera

## Components Used
- ESP32-CAM Module (AI-Thinker, with OV2640 camera)
- ESP32-CAM-MB Programmer Baseboard (HW-381) — provides built-in USB and auto-reset circuitry
- PIR Motion Sensor (HC-SR501)
- Red LED (220Ω resistor)
- Small Passive Buzzer Module

## Wiring Diagram (text)

**PIR Sensor:**
- VCC → 5V
- GND → GND
- OUT → GPIO13

**Red LED:**
- GPIO12 → 220Ω resistor → LED anode
- LED cathode → GND

**Buzzer:**
- I/O → GPIO15
- VCC → 5V (or 3V3)
- GND → GND

## Important: Reserved Pins on ESP32-CAM
GPIO2 and GPIO14 are reserved internally for SD card interface communication, even when the SD card is not in use. Connecting external components (LED, buzzer) to these pins — or even just calling `pinMode()` on them — causes the camera's I2C/SCCB communication with its sensor chip to fail (`i2c.master: probe device timeout` errors, camera init failure). GPIO12, GPIO13, and GPIO15 were confirmed safe and used instead for all external wiring in this project.

## Programming Instructions (ESP32-CAM Upload Procedure)

The ESP32-CAM module has no onboard USB port and must be programmed via an external serial connection. There are two common ways to do this:

### Method A: Bare ESP32-CAM + USB-to-TTL (FTDI) Adapter
This is the standard method when no programmer baseboard is available.

**Wiring (FTDI ↔ ESP32-CAM):**
```
FTDI TX  → ESP32-CAM RX (U0R)
FTDI RX  → ESP32-CAM TX (U0T)
FTDI GND → ESP32-CAM GND
FTDI 5V  → ESP32-CAM 5V
```
Note: TX connects to RX and RX connects to TX — crossed, not straight-through.

**GPIO0 Jumper (required for upload mode):**
```
ESP32-CAM GPIO0 → GND
```
This connection must be made **before and during** every upload. It forces the ESP32-CAM into programming/download mode on boot. Once the upload completes, **disconnect the GPIO0–GND jumper**, then press the ESP32-CAM's RESET button to boot normally — the board will not run the uploaded program correctly while this jumper remains connected.

**Upload steps:**
1. Connect GPIO0 to GND
2. Click Upload in Arduino IDE
3. If it doesn't connect, press RESET on the board right as "Connecting....." appears in the console
4. Once upload succeeds, disconnect the GPIO0–GND jumper
5. Press RESET again to boot the uploaded program normally

### Method B: ESP32-CAM-MB Baseboard (used for this project)
The CAM-MB baseboard used in this build integrates a USB connector and automatic reset/programming-mode circuitry, removing the need for a separate FTDI adapter or a manual GPIO0 jumper wire.

**Steps:**
1. Plug the ESP32-CAM module into the baseboard's connector
2. Connect a USB cable from the baseboard directly to the computer
3. Select Board: **AI-Thinker ESP32-CAM** in Arduino IDE
4. Click Upload — the baseboard's circuitry automatically handles entering/exiting programming mode

**If upload fails with "Failed to connect to ESP32: No serial data received":**
Press and hold the **RST** button on the baseboard as soon as "Connecting....." appears in the Arduino IDE console, hold for 1-2 seconds, then release once upload progress begins. If that doesn't work, hold the **IO0** button, press and release **RST** once while still holding IO0, then release IO0 and click Upload.

## Board Settings (both methods)
- Board: AI-Thinker ESP32-CAM
- Upload Speed: 115200
- Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS) — required, since the camera example needs more flash space than the default scheme provides

## Libraries Used
No external libraries beyond the ESP32 board package's built-in camera driver (`esp_camera.h`) and `WiFi.h`. This project builds on the stock **CameraWebServer** example (File → Examples → ESP32 → Camera → CameraWebServer), which includes supporting files `board_config.h`, `camera_pins.h`, and `app_httpd.cpp` — all included in this folder, required for the sketch to compile.

## How to Run
1. Ensure all files in this folder (`.ino`, `board_config.h`, `camera_pins.h`, `app_httpd.cpp`) are together in the same sketch folder
2. In `board_config.h`, confirm `#define CAMERA_MODEL_AI_THINKER` is uncommented (all other CAMERA_MODEL lines should remain commented out)
3. In the main `.ino`, replace `ssid` and `password` with your own 2.4GHz Wi-Fi credentials (ESP32 does not support 5GHz networks)
4. Follow the programming instructions above for your specific hardware setup
5. After upload, open Serial Monitor at 115200 baud — note the printed IP address
6. On a device connected to the same Wi-Fi network, open `http://<printed-ip>/` in a browser to view the live stream

## Expected Output
- Live MJPEG camera stream viewable in browser at the ESP32's IP address
- PIR sensor triggers an ALERT state on motion detection: red LED turns on, buzzer clicks briefly, Serial Monitor logs `STATE: ALERT - motion detected` with a timestamped entry stored in an internal 5-event rolling log
- After 60 seconds with no further motion, system returns to STANDBY: LED turns off, Serial Monitor logs `STATE: STANDBY - no motion for 60s`
- Camera stream remains active and responsive throughout motion detection cycles

## Build Notes
- Initial buzzer implementation used `tone()`, which internally relies on the ESP32's LEDC (PWM/timer) hardware — the same hardware subsystem the camera uses for its own operation (`config.ledc_channel`, `config.ledc_timer`). This caused the entire system to hang after the first motion trigger. Fixed by replacing `tone()` with a simple `digitalWrite()` HIGH/LOW pulse for the buzzer, which avoids the LEDC conflict entirely at the cost of a less musical "click" sound rather than a defined tone.
- The PIR sensor's onboard sensitivity trimmer required adjustment to avoid false triggers from ambient light or minor temperature shifts in the room; verified correct behavior by fully covering the sensor and confirming it reported "no motion" before relying on it in the full system.
- The camera module's ribbon cable connector loosened multiple times during development from routine handling while wiring nearby components, causing intermittent `i2c.master: probe device timeout` camera initialization failures. Reseating the connector and closing its locking flap resolved this each time.
- Event log entries are currently recorded to Serial Monitor and an internal array rather than displayed directly on the served web page.
