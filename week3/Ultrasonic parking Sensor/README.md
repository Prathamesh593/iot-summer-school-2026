# Q22 - Ultrasonic Parking Sensor

## Wokwi Project
https://wokwi.com/projects/468586133087234049

## Components
- Arduino Uno
- HC-SR04 Ultrasonic Sensor
- Green LED
- Yellow LED
- Red LED
- Piezo Buzzer

## Working
- Distance > 50 cm: SAFE (Green LED ON)
- Distance 20-50 cm: Yellow LED ON, buzzer beeps every 500 ms
- Distance 10-20 cm: Red LED ON, buzzer beeps every 200 ms
- Distance < 10 cm: All LEDs flash rapidly, buzzer continuous

## Formula
distance = (duration * 0.034) / 2
