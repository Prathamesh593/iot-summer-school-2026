void setup() {
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);
  delay(500); // Changed to 500ms
  digitalWrite(13, LOW);
  delay(500); // Changed to 500ms
}
