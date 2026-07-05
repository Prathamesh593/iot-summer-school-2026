int blinkCounter = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("Serial Command Interface Ready");
}

void loop() {

  if (Serial.available() > 0) {

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "LED ON") {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("LED is ON");
    }

    else if (cmd == "LED OFF") {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("LED is OFF");
    }

    else if (cmd.startsWith("BLINK ")) {

      int n = cmd.substring(6).toInt();

      if (n >= 1 && n <= 9) {

        for (int i = 0; i < n; i++) {
          digitalWrite(LED_BUILTIN, HIGH);
          delay(500);
          digitalWrite(LED_BUILTIN, LOW);
          delay(500);
        }

        blinkCounter += n;
        Serial.println("Blink Done");
      }
      else {
        Serial.println("ERROR: Invalid blink number");
      }

    }

    else if (cmd == "STATUS") {
      Serial.print("Blink Counter = ");
      Serial.println(blinkCounter);
    }

    else if (cmd == "RESET") {
      blinkCounter = 0;
      Serial.println("Blink Counter Reset");
    }

    else {
      Serial.println("ERROR: Unknown command");
    }

  }
}
