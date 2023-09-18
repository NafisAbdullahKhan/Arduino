#include <MemoryFree.h>
#include <SoftwareSerial.h>

// For the electronic wiring , you should :
// Connect pinRx to the Pin2 of XBee(Tx , Dout)
// Connect pinTx to the Pin3 of XBee(Rx , Din)

// Define the pins on Arduino for XBee comminication
uint8_t pinRx = 10, pinTx = 11; // the pin on Arduino
long BaudRate = 19200, BaudRateX = 19200, starttime = 0;
int a = 0;
char xbeeStr[] = {'S',0xFF,0xFF,'E'};
// Initialize NewSoftSerial
SoftwareSerial xbee( pinRx , pinTx );

void setup()  
{ // You shall see these messages in Arduino Serial Monitor
// This part is the official library , it will be used for talking to
// PC serial port
  Serial.begin(BaudRate);
  

  // This part is the NewSoftSerial for talking to XBee
  xbee.begin(BaudRateX);
}

void loop()                    
{
  if((millis()-starttime)>=1000){
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());
    xbee.write(xbeeStr);
    starttime = millis();
  }
}
