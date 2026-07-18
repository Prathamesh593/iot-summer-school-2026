# Project 1: Smart Room Climate Monitor

## Components Used
- ESP32 Dev Module (30-pin variant)
- AM2302 (DHT22) Temperature & Humidity Sensor
- 0.96" SSD1306 OLED, 128x64, 7-pin SPI/I2C combo module (wired in SPI mode)
- Red LED, Green LED (with 220Ω resistors)
- Small passive buzzer module (VCC/GND/I/O)

## Wiring Diagram (text)

**DHT22 (AM2302):**
- VCC → 3.3V
- GND → GND
- DATA → GPIO4

**OLED (SPI mode):**
- VCC → 3.3V
- GND → GND
- D0 (CLK) → GPIO18
- D1 (MOSI) → GPIO23
- RES → GPIO17
- DC → GPIO16
- CS → GPIO5

**Red LED:**
- GPIO25 → 220Ω resistor → LED anode
- LED cathode → GND

**Green LED:**
- GPIO26 → 220Ω resistor → LED anode
- LED cathode → GND

**Passive Buzzer Module:**
- VCC → 3.3V
- GND → GND
- I/O (signal) → GPIO27

## Libraries Used
- DHTesp by beegee_tokyo — v1.91
- Adafruit SSD1306 — v2.5.17
- Adafruit GFX Library — v1.12.6
- Adafruit BusIO — v1.17.4

## How to Run
1. Open `p1-climate-monitor.ino` in Arduino IDE 2.3.8
2. Select Board: ESP32 Dev Module
3. Select the correct COM port
4. Upload
5. Open Serial Monitor at 115200 baud

## Expected Output
- OLED cycles every 5 seconds through 3 screens:
  1. Live temperature & humidity readings
  2. Comfort status label (COOL / COMFORT / HOT / DANGER)
  3. Daily max/min temperature since power-on (bonus feature)
- Serial Monitor logs a CSV row every 5 seconds in the format: `millis,temp,humidity,status`
- Red LED + buzzer tone activate when temperature exceeds 38°C or humidity exceeds 80%
- Green LED stays on during normal conditions

## Bonus Challenge — Implemented
Daily max/min temperature tracker using `millis()` since power-on. Values persist across the session and display as a third rotating OLED screen.

## Build Notes
- Board used is a 30-pin ESP32 DevKit (not the 38-pin variant listed in the kit) — all GPIOs used are common to both.
- Sensor is actually the AM2302 (DHT22) module, not DHT11 as originally listed — code configured accordingly (`DHTesp::DHT22`).
- OLED is the 7-pin SPI/I2C combo module, wired and coded in SPI mode rather than I2C.
- Buzzer is a passive buzzer module (not active) — driven using `tone()`/`noTone()` rather than plain `digitalWrite()`.
