int blinkCount = 0; // Initialize counter

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600); // Start serial communication
}

void loop() {
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);

  blinkCount++;
  Serial.print("Blink count: ");
  Serial.println(blinkCount);
}
