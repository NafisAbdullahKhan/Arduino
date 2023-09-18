char buff[500];
String number = "";
int i = 0;

void setup(){
pinMode(53, OUTPUT);
digitalWrite(53, HIGH);
Serial.begin(9600);
Serial1.begin(9600); // the GPRS baud rate
delay(2500);

Serial.println("AT");
Serial1.println("AT"); //To check if GSM module is working
delay(2000);
Serial.println("AT+CLIP=1");
Serial1.println("AT+CLIP=1"); // set the SMS mode to text
delay(3000);
/*Serial.print("AT+CMGS=");
//Serial.print(34, DEC); //ASCII of “
Serial.print("\"+8801915217492\"\r");
//Serial.println(34,BYTE);

delay(1500);
Serial.println("This is a sms from Nafis. just saying hello."); //this is the message to be sent
delay(1500);
Serial.println("\x1A"); //Ascii code of ctrl+z to send the message
*/
}
void loop(){
  while(Serial1.available()){
    buff[i] = (char) Serial1.read();
    i++;
  } if(i > 0){
    String str = String(buff);
    Serial.println(str);
    Serial.println(i);
    if(str.substring(2,6) == "RING") {
      Serial.println("found");
      Serial1.println("ATH");
      number = str.substring(18,32);
      Serial.println(number);
      delay(2000);
      Serial1.println("AT+CMGF=1");
      delay(1500);
      Serial1.print("AT+CMGS=");
      //Serial.print(34, DEC); //ASCII of “
      Serial1.print("\""+number+"\"\r");
      //Serial.println(34,BYTE);
      
      delay(1500);
      Serial1.println("Hello, I'm Nafis. My cell phone is currently unreachable. Please try again later."); //this is the message to be sent
      delay(1500);
      Serial1.println("\x1A"); //Ascii code of ctrl+z to send the message
    }
    while(i > 0){
      buff[i-1] = 0x00;
      i--;
    }
  }
  delay(1500);
}
