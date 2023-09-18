#include <Wire.h>

#define LED_PIN 13
#define LED_1 22
#define LED_2 23

byte x = 1;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  
  Serial.begin(9600);
  Wire.begin(9);                // Start I2C Bus as a Slave (Device Number 9)
  Wire.onReceive(receiveEvent); // register event
  
}

void loop() {
  //Serial.println(x);
    //Serial.println("Hello");
  //If value received is 0 blink LED 1
  if (x == 0) {
    digitalWrite(LED_2, HIGH);
    delay(200);
    digitalWrite(LED_2, LOW);
    delay(200);
  }
  //If value received is 1 blink LED 2
  if (x == 1) {
    digitalWrite(LED_1, HIGH);
    delay(200);
    digitalWrite(LED_1, LOW);
    delay(200);
  }
}

void receiveEvent(int howMany) {
  while(Wire.available()){
    x = Wire.read();    // receive byte as an integer
    //Serial.println(Wire.read());
    Serial.println(x);
    
    //Serial.println("Hello!");
    //digitalWrite(LED_1, HIGH);
  }
  
  //Serial.println(howMany);
  //Serial.println("Hello!");
}
