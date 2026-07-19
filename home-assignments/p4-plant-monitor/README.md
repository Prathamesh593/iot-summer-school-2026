# Project 4: Smart Plant Watering Monitor

## Components Used
- ESP32 Dev Module (30-pin)
- Capacitive Soil Moisture Sensor V2.0
- AM2302 (DHT22) Temperature & Humidity Sensor
- 5V 2-Channel SSR Relay Module (Low Level Trigger, with fuse)
- 0.96" SSD1306 OLED, 128x64, 7-pin SPI/I2C combo (wired in SPI mode)

## Wiring Diagram (text)

**Soil Moisture Sensor:**
- VCC → 3.3V
- GND → GND
- AOUT → GPIO34

**DHT22 (AM2302):**
- VCC → 3.3V
- GND → GND
- DATA → GPIO4

**Relay (SSR, Channel 2 used):**
- DC+ → VIN (5V from USB)
- DC- → GND
- CH2 → GPIO27

**OLED (SPI mode):**
- VCC → 3.3V
- GND → GND
- D0 (CLK) → GPIO18
- D1 (MOSI) → GPIO23
- RES → GPIO17
- DC → GPIO16
- CS → GPIO5

**Manual Override Button:**
- Uses the ESP32's onboard BOOT button (GPIO0) — no external wiring required

## Calibration Values (measured for this specific sensor)
- Dry (open air): raw ADC ≈ 2300
- Wet (submerged): raw ADC ≈ 600
- These values are used directly in the `map()` function for accurate 0–100% conversion.

## Libraries Used
- DHTesp by beegee_tokyo
- Adafruit SSD1306
- Adafruit GFX Library
- Adafruit BusIO

## How to Run
1. Open `p4-plant-monitor.ino` in Arduino IDE
2. Board: ESP32 Dev Module
3. Select correct COM port
4. Upload
5. Open Serial Monitor at 115200 baud

## Expected Output
- OLED displays live soil moisture %, temperature, soil status (DRY/OPTIMAL/WET), pump state, and a moisture bar graph
- Serial Monitor logs the same data every 2 seconds
- Relay (pump simulation) turns ON when moisture drops below 30%, and OFF only once moisture rises above 40% (hysteresis prevents rapid on/off switching near the threshold)
- Pressing the ESP32's onboard BOOT button (GPIO0) forces the pump ON for 5 seconds regardless of current moisture reading, then automatically resumes normal automatic mode

## Build Notes
- Relay module used is a Solid State Relay (SSR), not a mechanical relay. It is confirmed "Low Level Trigger" (active-LOW) per the board's own silkscreen label, consistent with typical mechanical relay logic.
- An external LED connected across the relay's output terminals (to visually simulate a pump load) showed a constant dim glow rather than a clean on/off blink, even when the relay was confirmed switching correctly (verified via the module's own onboard channel indicator LED). This is due to a small internal leakage current inherent to SSR designs, which is negligible for real inductive/resistive loads like an actual pump but enough to faintly light a low-current LED. The relay's own onboard indicator LED was used as the primary visual confirmation of switching in the demo video instead.
- Manual override uses the ESP32's onboard BOOT button (GPIO0) rather than a separately wired push button, since GPIO0 is directly accessible on the board.
