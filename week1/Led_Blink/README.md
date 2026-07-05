Link to TinkerCad Project:
https://www.tinkercad.com/things/6615MaInIXu/editel?returnTo=%2Fsearch%3Fq%3Dled%2520blink%26staffPicks%3D0%26type%3Dcircuits&sharecode=7Cumt581Dh8r4pLRkrD5GqMsUEBbZZWjbQkc1THjRUc


# Week 1: Smart LED Blink Controller

A version-controlled Arduino project that evolves a basic LED blink configuration into a dynamic, potentiometer-controlled variable speed flasher with serial monitoring capabilities.

---

## 🛠️ Hardware Required

* **1x** Arduino Uno R3 (or compatible microcontroller)
* **1x** LED (any color)
* **1x** 220Ω Resistor (to protect the LED)
* **1x** 10kΩ Potentiometer (for analog speed control)
* Jumper wires (for direct component-to-board wiring)

---

## 🔌 Circuit Diagram Description

This setup utilizes **direct component wiring** without a traditional breadboard for a compact footprint:

1.  **LED Circuit:** * The **Anode** (longer positive leg) of the LED connects to one side of the **220Ω resistor**. The other side of the resistor connects directly to **Digital Pin 13** on the Arduino.
    * The **Cathode** (shorter negative leg) of the LED connects directly to an Arduino **GND** pin.
2.  **Potentiometer Circuit:**
    * **Pin 1 (Left):** Connected to an Arduino **GND** pin.
    * **Pin 2 (Middle Wiper):** Connected to **Analog Input Pin A0** to feed the variable voltage reading back to the microcontroller.
    * **Pin 3 (Right):** Connected to the Arduino **5V** power pin.

---

## 🚀 How to Upload Code

Follow these steps using the official **Arduino IDE**:

1.  **Open the File:** Launch the Arduino IDE and open the `led_blink.ino` sketch located in `/week1/led_blink/`.
2.  **Connect the Hardware:** Plug your Arduino Uno into your computer's USB port using an appropriate USB cable.
3.  **Select Board & Port:**
    * Navigate to `Tools` > `Board` and select **Arduino Uno**.
    * Navigate to `Tools` > `Port` and select the COM port matching your connected Arduino.
4.  **Verify & Compile:** Click the **Verify** checkmark icon in the top left to ensure there are no syntax errors in the code.
5.  **Upload:** Click the **Upload** right-arrow icon next to it to flash the code onto your microcontroller.

---

## 🎯 Expected Output

* **Hardware Behavior:** The connected LED will begin blinking. Rotating the knob on the 10kΩ potentiometer clockwise or counter-clockwise will instantly increase or decrease the delay interval (speed) of the blinking sequence.
* **Serial Monitor Output:** Open the Serial Monitor (`Tools` > `Serial Monitor`) and set the baud rate to **9600**. You should see an ongoing incrementing log stream:
    ```text
    Blink count: 1
    Blink count: 2
    Blink count: 3
    ```

---

## 🔍 Troubleshooting Tips

* **LED does not light up at all:** Check the orientation of your LED. LEDs are polarized; if the longer leg (anode) isn't facing Pin 13 and the flat edge/shorter leg (cathode) isn't connected to GND, it won't illuminate.
* **Serial Monitor prints garbled characters or nothing:** Ensure that the baud rate dropdown menu in the bottom-right corner of your Arduino IDE Serial Monitor is explicitly set to **9600 baud** to match the `Serial.begin(9600);` statement in the code.
* **The LED stays completely on or off when twisting the potentiometer:** Verify your analog wiring. Ensure the middle wiper pin of the potentiometer is firmly hooked into **A0**, and that your power (5V) and ground (GND) connections are not flipped or shorting out against one another.
