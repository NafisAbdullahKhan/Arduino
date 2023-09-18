#include <Wire.h>

#define LED_PIN 13
char x = 'a';

void setup()
{
  Wire.begin(); // Start I2C Bus as Master
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  Serial.begin(9600);

}
void loop()
{
  Wire.beginTransmission(9); // transmit to device #9
  Wire.write(x);              // sends x 
  Wire.endTransmission();    // stop transmitting
  x++;
  if (x > 5) x=0;
  delay(450);
}
