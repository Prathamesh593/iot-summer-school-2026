#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

#define RELAY1  27   // CH2 - Light 1
#define RELAY2  26   // CH1 - Light 2
#define BTN1    0    // onboard BOOT button - Light 1 override
#define BTN2    35   // external button - Light 2 override
#define BT_LED  2    // built-in LED, BT connected indicator
#define BUZZER  25

bool light1On = false;
bool light2On = false;
unsigned long lastCommandTime = 0;
const unsigned long BT_TIMEOUT = 30UL * 60UL * 1000UL; // 30 minutes

void setup() {
  Serial.begin(115200);
  SerialBT.begin("IIT_IoT_HomeCtrl");
  Serial.println("Bluetooth started, pair with: IIT_IoT_HomeCtrl");

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT); // external pull-down resistor used
  pinMode(BT_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RELAY1, HIGH); // start OFF (active-LOW)
  digitalWrite(RELAY2, HIGH);
  lastCommandTime = millis();
}

void setLight1(bool state) {
  light1On = state;
  digitalWrite(RELAY1, state ? LOW : HIGH);
}

void setLight2(bool state) {
  light2On = state;
  digitalWrite(RELAY2, state ? LOW : HIGH);
}

void beep() {
  tone(BUZZER, 1000);
  delay(50);
  noTone(BUZZER);
}

void sendStatus() {
  String msg = "L1:" + String(light1On ? "ON" : "OFF") + " L2:" + String(light2On ? "ON" : "OFF");
  SerialBT.println(msg);
  Serial.println(msg);
}

void loop() {
  digitalWrite(BT_LED, SerialBT.hasClient() ? HIGH : LOW);

  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    lastCommandTime = millis();
    bool validCommand = true;

    switch (cmd) {
      case '1': setLight1(true); break;
      case '2': setLight1(false); break;
      case '3': setLight2(true); break;
      case '4': setLight2(false); break;
      case '5': setLight1(true); setLight2(true); break;
      case '6': setLight1(false); setLight2(false); break;
      case '?': break;
      default: validCommand = false; break;
    }

    if (validCommand) {
      if (cmd != '?') beep();
      sendStatus();
    }
  }

  static bool lastBtn1State = HIGH;
  bool currentBtn1State = digitalRead(BTN1);
  if (lastBtn1State == HIGH && currentBtn1State == LOW) {
    setLight1(!light1On);
    beep();
    sendStatus();
    lastCommandTime = millis();
  }
  lastBtn1State = currentBtn1State;

  static bool lastBtn2State = LOW;
  bool currentBtn2State = digitalRead(BTN2);
  if (lastBtn2State == LOW && currentBtn2State == HIGH) {
    setLight2(!light2On);
    beep();
    sendStatus();
    lastCommandTime = millis();
  }
  lastBtn2State = currentBtn2State;

  if (millis() - lastCommandTime > BT_TIMEOUT) {
    setLight1(false);
    setLight2(false);
  }

  delay(50);
}
