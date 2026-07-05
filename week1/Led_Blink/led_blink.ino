/*
 * Author: Prathamesh Naik
 * Date: 5th July 2026
 * Description: LED blink controller with dynamic potentiometer speed 
 * control and serial tracking for Git assignment.
 */

int blinkCount = 0;
const int potPin = A0;

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int potValue = analogRead(potPin);

  digitalWrite(13, HIGH);
  delay(potValue);
  digitalWrite(13, LOW);
  delay(potValue);

  blinkCount++;
  Serial.print("Blink count: ");
  Serial.println(blinkCount);
}
