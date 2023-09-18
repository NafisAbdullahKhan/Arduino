uint16_t boxid = 300;
uint8_t b = 0;
long starttime = 0;
char serSndStr[] = {
  'B',
  0x01,
  0x01,
  'I',
  '/0'
};

byte serRcvStr[4];

void setup(){
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop(){
  if(millis()-starttime >= 1000){
    Serial1.print(serSndStr);
    starttime = millis();
  }
  if(Serial1.available() && (serRcvStr[b] = Serial1.read()) > 0){
    //serRcvStr[b] = Serial1.read();
    Serial.println(String(b)+" "+String(serRcvStr[b]));
    if(serRcvStr[0] == 'B'){
      b++;
      if(b == 4) b = 0;
    }
    if(serRcvStr[0] == 'B' && serRcvStr[3] == 'I'){
      Serial.println(String(b)+" "+String(serRcvStr[1])+" "+String(serRcvStr[2]));
    }
  }
  Serial1.flush();
  Serial.flush();
}


