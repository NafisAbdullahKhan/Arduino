#include <SoftwareSerial.h>

// For the electronic wiring , you should :
// Connect pinRx to the Pin2 of XBee(Tx , Dout)
// Connect pinTx to the Pin3 of XBee(Rx , Din)

// Define the pins on Arduino for XBee comminication
uint8_t pinRx = 10, pinTx = 11; // the pin on Arduino
long BaudRate = 9600, BaudRateX = 1200, sysTick = 0;
char GotChar;
int a = 0;
char xbeeStr[17];
// Initialize NewSoftSerial
SoftwareSerial mySerial( pinRx , pinTx );

void setup()  
{ // You shall see these messages in Arduino Serial Monitor
// This part is the official library , it will be used for talking to
// PC serial port
  Serial.begin(BaudRate);
  Serial.println("XBee Communication Test Start !");
  Serial.print("Xbee BaudRate:");
  Serial.println(BaudRateX);
  Serial.print("NewSoftSerial Rx Pin#");
  Serial.println(pinRx,DEC);
  Serial.print("NewSoftSerial Tx Pin#");
  Serial.println(pinTx,DEC);

  // This part is the NewSoftSerial for talking to XBee
  mySerial.begin(BaudRateX);
  mySerial.println("Powered by NewSoftSerial !");
}

void loop()                    
{
  sysTick++ ; // a system timer
/* //for debug
  Serial.print("Xbee Timer :");
  Serial.println(sysTick);
  mySerial.print("Xbee Timer :");
  mySerial.println(sysTick);
*/
// Monitor Rx from PC , if the data is available then read
// it to "GotChar".    Then ask XBee send the data out 
// wirelessly.
  if ( Serial.available() ) {
      GotChar = Serial.read();
      mySerial.print(GotChar);
  }
// Monitor data from XBee , if the data is available then 
// read it to "GotChar".    Then send it back to PC.
//Serial.println(mySerial.read());
//delay(1000);
  if (mySerial.available()) {
    xbeeStr[a] = mySerial.read();
    Serial.println(String(xbeeStr[a], HEX));
    a++;
  }
  else{
    //Serial.println((char)xbeeStr[4]);
    a = 0;
  }
}
