# Week 3
This folder contains Week 3 assignments
# Q21 - Environmental Monitoring Station

Sensor: DHT22

Library: DHT sensor library by Adafruit
Version: 1.4.x


# Q28 - Keypad + LCD Password Protected System 🔐

## 📌 Problem Statement
Using a 4x4 membrane keypad and 16x2 LCD, build a password-protected access system:
- Display "ENTER PIN" on LCD
- User enters 4-digit PIN
- Correct PIN → "ACCESS GRANTED" + Green LED
- Wrong PIN → "ACCESS DENIED" + Red LED + Buzzer
- After 3 wrong attempts → system locks for 10 seconds

## ⚙️ Features
- 4-digit password input
- LCD display output
- LED indication system
- Buzzer alert system
- Security lock after 3 failed attempts

## 🧰 Components Used
- Arduino UNO
- 4x4 Keypad
- 16x2 LCD (I2C)
- Green LED
- Red LED
- Buzzer

## 🔌 Working
1. LCD shows "ENTER PIN"
2. User enters PIN using keypad
3. Press # to submit
4. System checks password:
   - Correct → ACCESS GRANTED
   - Wrong → ACCESS DENIED
5. After 3 wrong attempts → 10 sec lock

## 💻 Author
Week 3 IoT Assignment Submission
