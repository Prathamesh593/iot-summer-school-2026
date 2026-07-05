int ldrPin = A0;
int pirPin = 2;
int ledPin = 3;

int ldrThreshold = 500;
unsigned long delayTime = 3000;

int fullBright = 255; 
int dimBright = 51;
int offBright = 0; 

unsigned long lastMotionTime = 0;

bool motionActive = false;
bool isNight = false;

void setup() {
  Serial.begin(9600);

  pinMode(ldrPin, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);

  analogWrite(ledPin, offBright);

  logMessage("SYSTEM INITIALIZED");
}

void loop() {

  int ldrReading = analogRead(ldrPin);
  int pirReading = digitalRead(pirPin);

  if (ldrReading > ldrThreshold) {

    if (isNight) {
      isNight = false;
      motionActive = false;

      analogWrite(ledPin, offBright);

      logMessage("DAYLIGHT DETECTED - LED OFF");
    }

    return; 
  }

  if (!isNight) {
    isNight = true;

    analogWrite(ledPin, dimBright);

    logMessage("DARKNESS DETECTED - Motion Detection Mode Enabled");
  }

  if (pirReading == HIGH) {

    if (!motionActive) {
      logMessage("MOTION DETECTED - LED at Full Brightness (100%)");
    }

    motionActive = true;
    lastMotionTime = millis();

    analogWrite(ledPin, fullBright);
  }

 
  if (motionActive && (millis() - lastMotionTime >= delayTime)) {

    analogWrite(ledPin, dimBright);

    motionActive = false;

    logMessage("TIMEOUT - No Motion for 30 Seconds. LED Dimmed to 20%");
  }

  delay(100);
}

void logMessage(String text) {

  unsigned long totalSecs = millis() / 1000;

  int s = totalSecs % 60;
  int m = (totalSecs / 60) % 60;
  int h = (totalSecs / 3600) % 24;

  char timeStamp[12];
  sprintf(timeStamp, "[%02d:%02d:%02d]", h, m, s);

  Serial.print(timeStamp);
  Serial.print(" EVENT: ");
  Serial.println(text);
}
