
byte buff[10];

int rel1 = 27, rel2 = 33, rel3 = 39, rel4 = 45;

int i = 0;

long starttime = 0, 
curtime = 0, 
rel1time = 0, 
rel2time = 0, 
rel3time = 0, 
rel4time = 0;

void setup (){
  Serial.begin(9600);
  pinMode(rel1, OUTPUT);
  pinMode(rel2, OUTPUT);
  pinMode(rel3, OUTPUT);
  pinMode(rel4, OUTPUT);
}

void loop (){
    if(Serial.available()){
      buff[i] = Serial.read();
      i++;
    } else{
      if(buff[0] == '1'){
        digitalWrite(rel1, HIGH);
        rel1time = millis();
      } else if(buff[0] == '2'){
        digitalWrite(rel2, HIGH);
        rel2time = millis();
      } else if(buff[0] == '3'){
        digitalWrite(rel3, HIGH);
        rel3time = millis();
      } else if(buff[0] == '4'){
        digitalWrite(rel4, HIGH);
        rel4time = millis();
      } else if(buff[0] == 'C'){
        digitalWrite(rel1, LOW);
        digitalWrite(rel2, LOW);
        digitalWrite(rel3, LOW);
        digitalWrite(rel4, LOW);
      }
      i = 0;
      
      if(millis() - starttime >= 1000){
        Serial.print("a");
        starttime = millis();
      }
    }
    
    if(millis() - rel1time >= 10000){
      digitalWrite(rel1, LOW);
      rel1time = millis();
    }
    
    if(millis() - rel2time >= 10000){
      digitalWrite(rel2, LOW);
      rel2time = millis();
    }
    
    if(millis() - rel3time >= 10000){
      digitalWrite(rel3, LOW);
      rel3time = millis();
    }
    
    if(millis() - rel4time >= 10000){
      digitalWrite(rel4, LOW);
      rel4time = millis();
    }
}
