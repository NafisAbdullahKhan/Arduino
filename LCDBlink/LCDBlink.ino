#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(50, 51, 22, 23, 24, 25);

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // Turn off the blinking cursor:
  lcd.setCursor(0,1);
  lcd.noBlink();
  delay(500);
   // Turn on the blinking cursor:
  lcd.blink();
  delay(500);
}
