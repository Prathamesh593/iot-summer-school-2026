int blinkCount = 0;
const int potPin = A0; // Potentiometer connected to Analog pin A0

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int potValue = analogRead(potPin); // Read potentiometer (0 to 1023)

  digitalWrite(13, HIGH);
  delay(potValue); // Speed controlled by potentiometer
  digitalWrite(13, LOW);
  delay(potValue);

  blinkCount++;
  Serial.print("Blink count: ");
  Serial.println(blinkCount);
}
