#include <LiquidCrystal.h>
#include <Keypad.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// LCD RS,E,D4,D5,D6,D7

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int greenLED = 10;
const int redLED = 13;
const int buzzer = A4;

String password = "2004";
String input = "";

int attempts = 0;

void setup() {
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.begin(9600);

  lcd.begin(16,2);
  lcd.print("ENTER PIN:");
}

void loop() {

  char key = keypad.getKey();

  if(key){

    tone(buzzer,1000,100);

    if(key>='0' && key<='9'){
      input += key;

      lcd.setCursor(input.length()-1,1);
      lcd.print("*");
    }

    if(input.length()==4){

      if(input==password){

        lcd.clear();
        lcd.print("ACCESS");
        lcd.setCursor(0,1);
        lcd.print("GRANTED");

        digitalWrite(greenLED,HIGH);
        digitalWrite(redLED,LOW);

        Serial.println("ACCESS GRANTED");

        delay(3000);

        digitalWrite(greenLED,LOW);

        attempts=0;

      }else{

        lcd.clear();
        lcd.print("ACCESS");
        lcd.setCursor(0,1);
        lcd.print("DENIED");

        digitalWrite(redLED,HIGH);
        tone(buzzer,2000,500);

        Serial.println("ACCESS DENIED");

        delay(2000);

        digitalWrite(redLED,LOW);

        attempts++;

        if(attempts>=3){

          lcd.clear();
          lcd.print("LOCKED");
          lcd.setCursor(0,1);
          lcd.print("10 SECONDS");

          Serial.println("SYSTEM LOCKED");

          delay(10000);

          attempts=0;
        }
      }

      input="";

      lcd.clear();
      lcd.print("ENTER PIN:");
    }
  }
}
