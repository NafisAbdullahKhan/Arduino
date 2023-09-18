void setup(){
pinMode(53, OUTPUT);
digitalWrite(53, HIGH);
Serial.begin(9600); // the GPRS baud rate
delay(2500);

Serial.println("AT"); //To check if GSM module is working
delay(2000);
Serial.println("AT+CMGF=1"); // set the SMS mode to text
delay(3000);
Serial.print("AT+CMGS=");
//Serial.print(34, DEC); //ASCII of “
Serial.print("\"+8801915217492\"\r");
//Serial.println(34,BYTE);

delay(1500);
Serial.println("This is a sms from Nafis. just saying hello."); //this is the message to be sent
delay(1500);
Serial.println("\x1A"); //Ascii code of ctrl+z to send the message
}
void loop(){
}
