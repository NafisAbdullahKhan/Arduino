#include <SoftwareSerial.h>
#include <OneButton.h>
//#include <Bounce2.h>
#include <LiquidCrystal.h>
#include "Mirf.h"
#include "MirfHardwareSpiDriver.h"
#include "MirfSpiDriver.h"
#include "nRF24L01.h"

#define BUFFSIZE 17

OneButton btnRed(A0, true);
OneButton btnBlue(A1, true);
OneButton btnStart(A2, true);
OneButton btnReset(A3, true);

LiquidCrystal lcd(50, 51, 22, 23, 24, 25);
//uint8_t btnred,btnblue,btnstart,btnreset;

uint8_t mode = 1, cursr = 0;

typedef enum {
  START,
  LIMIT,
  INTERVAL,
  MODE,
  PENDING,
  GAME
} 
SCREEN;

SCREEN screen = START;

String modeList[] = {
  "Domination Mode","Rush Mode", "Push Mode", "Launch Mode"};
int capture_time=900,TIME=900,a=0;

int points_A=0,points_B=0, ptAinc = 0, ptBinc = 0;

boolean GAMEON=false,A=false,B=false, redReleased = true, blueReleased = true;
//int SCREEN=0;//0-off 1-start 2-score limit set 3-interval set 4-pending 5-gameon
int score_limit=10,TIMER=1;
long time=0,starttime=0, rt = 0, bt = 0;

int LED_A=26,LED_B=27,BUZZER=28;

//XBEE START
String xbeeStr = "";

byte *xbeeSBuff  = (byte *)malloc(sizeof(byte));
/*byte xbeeSBuff[BUFFSIZE] = {
 'S',
 'T',
 0x00,
 0x00,
 'A',
 0x00,
 0x00,
 'a',
 0x00,
 0x00,
 'B',
 0x00,
 0x00,
 'b',
 0x00,
 0x00,
 'E'};*/

uint8_t pinRx = 10, pinTx = 11; // the pin on Arduino
SoftwareSerial xbee( pinRx , pinTx );
//XBEE END

void setup() {
  lcd.begin(20, 4);

  btnRed.setClickTicks(100);
  btnBlue.setClickTicks(100);
  btnStart.setClickTicks(100);
  btnReset.setClickTicks(100);

  btnRed.setPressTicks(500);
  btnBlue.setPressTicks(500);
  btnStart.setPressTicks(500);
  btnReset.setPressTicks(500);

  btnRed.attachClick(onClickBtnRed);
  btnBlue.attachClick(onClickBtnBlue);
  btnStart.attachClick(onClickBtnStart);
  btnReset.attachClick(onClickBtnReset);

  btnRed.attachDuringLongPress(duringLongPressBtnRed);
  btnBlue.attachDuringLongPress(duringLongPressBtnBlue);
  //btnRed.attachLongPressStop(longPressStopBtnRed);
  //btnBlue.attachLongPressStop(longPressStopBtnBlue);

  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  pinMode(BUZZER,OUTPUT);

  //  pinMode(10,OUTPUT);
  //analogWrite(10,122);
  //analogWrite(6, 172);
  digitalWrite(BUZZER,LOW);
  digitalWrite(LED_A,HIGH);
  digitalWrite(LED_B,HIGH);

  Serial.begin(9600);
  xbee.begin(19200);
  start_screen();
}

void loop() {
  btnRed.tick();
  btnBlue.tick();
  btnStart.tick();
  btnReset.tick();
  /*if(btnred==1 && btnblue==1 && GAMEON==false){
   start_screen();
   }*/


  if((millis()-starttime)/1000>=1){
    xbeeStr = "";
    *xbeeSBuff = (byte)'S';
    *(xbeeSBuff+1) = (byte)'T';
    *(xbeeSBuff+2) = capture_time;
    *(xbeeSBuff+4) = (byte)'A';
    *(xbeeSBuff+5) = points_A;
    *(xbeeSBuff+7) = (byte)'a';
    *(xbeeSBuff+8) = 0x0000;
    *(xbeeSBuff+10) = (byte)'B';
    *(xbeeSBuff+11) = points_B;
    *(xbeeSBuff+13) = (byte)'b';
    *(xbeeSBuff+14) = 0x0000;
    *(xbeeSBuff+16) = (byte)'E';
    /*xbee.write("ST"+(byte)capture_time+"A"+(byte)points_A
     +"a0B"+(byte)points_B
     +"b0E");*/
    xbee.write(*xbeeSBuff);
    Serial.println("S1:"+*xbeeSBuff);
    Serial.println("R1:"+xbeeStr);
    if(GAMEON==true) {
      capture_time--;
      game_screen_time_update();
    }
    starttime=millis();
  }

  if(GAMEON==true){
    /*if((millis()-starttime)/1000>=1){
     capture_time--;
     starttime=millis();
     }*/
    //game_screen_time_update();
    if(!btnRed.isLongPressed() && !redReleased){
      if(rt>=20){
        time=millis();
        A=true;
        B=false;
        digitalWrite(LED_A,HIGH);
        digitalWrite(LED_B,LOW);
      }
      rt = 0;
      redReleased = true;
      game_screen();
    }

    if(!btnBlue.isLongPressed() && !blueReleased){
      if(bt>=20){
        time=millis();
        A=false;
        B=true;
        digitalWrite(LED_A,LOW);
        digitalWrite(LED_B,HIGH);
      }
      bt = 0;
      blueReleased = true;
      game_screen();
    }
  }

  switch(mode){
  case 1: 
  case 2:
    {
      dom_rush_mode();
      break;
    }
  default: 
    break;
  };
  if (xbee.available()) {
    xbeeStr+=xbee.read();
    Serial.println(a+xbeeStr);
    a++;
  }
  else{
    a = 0;
    if(xbeeStr.startsWith("S") && xbeeStr.endsWith("E")){

      int AT = (int)xbeeStr.substring(xbeeStr.indexOf("A")+1,
      xbeeStr.indexOf("a")).toInt();
      int BT = (int)xbeeStr.substring(xbeeStr.indexOf("B")+1,
      xbeeStr.indexOf("b")).toInt();
      int AI = (int)xbeeStr.substring(xbeeStr.indexOf("a")+1,
      xbeeStr.indexOf("B")).toInt();
      int BI = (int)xbeeStr.substring(xbeeStr.indexOf("b")+1,
      xbeeStr.indexOf("E")).toInt();

      if(screen == GAME && GAMEON == true){
        Serial.println("R2:"+xbeeStr);

        if(AI == 0 && AT > points_A) points_A = AT;
        if(BI == 0 && BT > points_B) points_B = BT;
        points_A += AI;
        Serial.println("PA"+String(points_A));
        points_B += BI;
        Serial.println("PB"+String(points_B));
        game_red_pt_update();
        game_blue_pt_update();
      }
      else{
        capture_time = (int)xbeeStr.substring(xbeeStr.indexOf("T")+1,
        xbeeStr.indexOf("A")).toInt();
        points_A = AT;
        Serial.println("PA"+String(points_A));
        points_B = BT;
        Serial.println("PB"+String(points_B));
      }
      xbeeStr = "";
    }
    if(ptAinc>0 || ptBinc>0){
      if(ptAinc>0)points_A++;
      if(ptBinc>0)points_B++;
      *(xbeeSBuff+2) = capture_time;
      *(xbeeSBuff+5) = points_A;
      *(xbeeSBuff+8) = ptAinc;
      *(xbeeSBuff+11) = points_B;
      *(xbeeSBuff+14) = ptBinc;
      /*xbee.print("ST"+byte(capture_time)
       +"A"+byte(points_A)
       +"a"+byte(ptAinc)
       +"B"+byte(points_B)
       +"b"+byte(ptBinc)+"E");*/
      Serial.println("S2:ST"+String(capture_time)
        +"A"+String(points_A)
        +"a"+String(ptAinc)
        +"B"+String(points_B)
        +"b"+String(ptBinc)+"E");
      if(screen == GAME && GAMEON == true){
        game_red_pt_update();
        game_blue_pt_update();
      }
      ptAinc = 0;
      ptBinc = 0;
      Serial.println("R3:"+xbeeStr);
    }
    xbee.overflow();
  }
}

void start_screen(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  SET CAPTURE TIME");
  lcd.setCursor(4,1);
  lcd.print(capture_time,10);
  lcd.setCursor(9,1);
  lcd.print("SECONDS");
  lcd.setCursor(0,3);
  lcd.print("R=+, B=-, START=NEXT");
  screen=START;
}
void update_start_screen(){
  lcd.setCursor(0,0);
  lcd.print("  SET CAPTURE TIME");
  lcd.setCursor(4,1);
  lcd.print(capture_time,10);
  lcd.setCursor(9,1);
  lcd.print("SECONDS");
  lcd.setCursor(0,3);
  lcd.print("R=+, B=-, START=NEXT");
  screen=START;
}

void limit_set(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  SET SCORE LIMIT");
  lcd.setCursor(4,1);
  lcd.print(score_limit,10);
  lcd.setCursor(9,1);
  lcd.print("POINTS");
  lcd.setCursor(0,3);
  lcd.print("R=+, B=-, START=NEXT");
  screen=LIMIT;
}
void limit_set_update(){
  lcd.setCursor(4,1);
  lcd.print(score_limit,10);
  lcd.setCursor(9,1);
  lcd.print("POINTS");
  screen=LIMIT;
}
void set_inteval(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" SET CAPTURE TIMER");
  lcd.setCursor(7,1);
  lcd.print(TIMER,10);
  lcd.setCursor(9,1);
  lcd.print("X");
  lcd.setCursor(0,2);
  lcd.print("MULTIPLES OF 20 SEC.");
  lcd.setCursor(0,3);
  lcd.print("R=+, B=-, START=NEXT");
  screen=INTERVAL;
}
void set_inteval_update(){
  lcd.setCursor(7,1);
  lcd.print(TIMER,10);
  screen=INTERVAL;
}

void select_mode(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Select Game Mode:");
  for(int i = cursr ; i < cursr+2; i++){
    lcd.setCursor(1,i+1-cursr);
    //lcd.print(capture_time,10);
    //lcd.setCursor(9,1);
    lcd.print(((i==cursr)?">":" ")+String(i+1)+". "+modeList[i]);
  }
  lcd.setCursor(0,3);
  lcd.print("R=+, B=-,   START=OK");
  screen=MODE;
}

void pending_screen(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("REMAINING TIME:");
  lcd.print(capture_time,10);
  lcd.setCursor(0,1);
  lcd.print("RED  POINTS: ");
  lcd.print(points_A,10);
  lcd.setCursor(0,2);
  lcd.print("BLUE POINTS: ");
  lcd.print(points_B,10);
  lcd.setCursor(0,3);
  lcd.print("PRESS START BUTTON");
  screen=PENDING;
}

void game_screen(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("REMAINING TIME:");
  lcd.print(capture_time,10);
  lcd.setCursor(0,1);
  lcd.print("RED  POINTS: ");
  lcd.print(points_A,10);
  lcd.setCursor(0,2);
  lcd.print("BLUE POINTS: ");
  lcd.print(points_B,10);
  lcd.setCursor(0,3);
  screen=GAME;
}
void game_screen_time_update(){
  lcd.setCursor(15,0);
  //lcd.print("REMAINING TIME:");
  lcd.print(capture_time,10);
  screen=GAME;
}

void game_red_pt_update(){
  lcd.setCursor(13,1);
  //lcd.print("REMAINING TIME:");
  lcd.print(points_A,10);
  screen=GAME;
}

void game_blue_pt_update(){
  lcd.setCursor(13,2);
  //lcd.print("REMAINING TIME:");
  lcd.print(points_B,10);
  screen=GAME;
}
void result_show(char r){
  lcd.clear();
  if(r == 'A'){
    lcd.setCursor(5,1);
    lcd.print("RED TEAM WON");
    digitalWrite(LED_B,LOW);
  }
  else if(r == 'B'){
    lcd.setCursor(3,1);
    lcd.print("BLUE TEAM WON");
    digitalWrite(LED_A,LOW);
  }
  else{
    lcd.setCursor(5,1);
    lcd.print("MATCH DRAWN");
    digitalWrite(LED_A,LOW);
    digitalWrite(LED_B,LOW);
  }
  digitalWrite(BUZZER,HIGH);
  GAMEON=false;
  A=false;
  B=false;

  //digitalWrite(LED_B,LOW);
  screen=GAME;
  capture_time=TIME;
  points_A=0;
  points_B=0;
}


void dom_rush_mode(){
  if(A==true && GAMEON==true){
    if(millis()-time>TIMER*1000){
      time=millis();
      ptAinc++;
      //points_A++;
      A = false;
      game_screen();
    }
  }
  if(B==true && GAMEON==true){
    if(millis()-time>TIMER*1000){
      time=millis();
      ptBinc++;
      //points_B++;
      B = false;
      game_screen();
    }
  }
  if((mode == 1 
    && points_A > (int)score_limit/2 
    || points_B > (int)score_limit/2) 
    || score_limit <= points_A + points_B 
    || capture_time<0){
    if(points_A>points_B){
      result_show('A');
    }
    else if(points_A<points_B){
      result_show('B');
    }
    else {
      result_show('D');
    }
  }
}

void push_mode(){
  if(A==true && GAMEON==true){
    if(millis()-time>TIMER*1000){
      time=millis();
      points_A++;
      game_screen();
    }
  }
  if(B==true && GAMEON==true){
    if(millis()-time>TIMER*1000){
      time=millis();
      points_B++;
      game_screen();
    }
  }
  if(score_limit<points_A){
    lcd.clear();
    lcd.setCursor(5,1);

    lcd.print("RED TEAM WON");
    digitalWrite(BUZZER,HIGH);
    GAMEON=false;
    A=false;
    B=false;
    //digitalWrite(LED_A,LOW);
    digitalWrite(LED_B,LOW);

    screen=GAME;
    capture_time=TIME;
    points_A=0;
    points_B=0;
  }
  if(score_limit<points_B){
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("BLUE TEAM WON");
    digitalWrite(BUZZER,HIGH);
    GAMEON=false;
    A=false;
    B=false;
    digitalWrite(LED_A,LOW);
    //digitalWrite(LED_B,LOW);
    screen=GAME;
    capture_time=TIME;
    points_A=0;
    points_B=0;
  }
  if(capture_time<0){
    if(points_A>points_B){
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("RED TEAM WON");
      digitalWrite(BUZZER,HIGH);
      GAMEON=false;
      A=false;
      B=false;
      //digitalWrite(LED_A,LOW);
      digitalWrite(LED_B,LOW);
      screen=GAME;
      capture_time=TIME;
      points_A=0;
      points_B=0;
    }
    else if(points_A<points_B){
      lcd.clear();
      lcd.setCursor(3,1);
      lcd.print("BLUE TEAM WON");
      digitalWrite(BUZZER,HIGH);
      GAMEON=false;
      A=false;
      B=false;
      digitalWrite(LED_A,LOW);
      //digitalWrite(LED_B,LOW);
      screen=GAME;
      capture_time=TIME;
      points_A=0;
      points_B=0;
    }
    else {
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("MATCH DRAWN");
      digitalWrite(BUZZER,HIGH);
      GAMEON=false;
      A=false;
      B=false;
      digitalWrite(LED_A,LOW);
      digitalWrite(LED_B,LOW);
      screen=GAME;
      capture_time=TIME;
      points_A=0;
      points_B=0;
    }
  }
}

void onClickBtnRed(){
  if(screen==START){
    GAMEON=false;
    if(capture_time<1000){
      capture_time++;
    }
    else
      capture_time=0; 

    TIME=capture_time;
    update_start_screen();
  }

  if(screen==LIMIT && GAMEON==false){
    score_limit++;
    limit_set_update();
  }

  if(screen==INTERVAL && GAMEON==false){
    if(TIMER<capture_time)
      TIMER++;
    else
      TIMER=0;
    set_inteval_update();
  }

  if(screen==MODE && GAMEON==false && cursr < sizeof(modeList)/sizeof(String)-1){
    cursr++;
    select_mode();
  }
}

void onClickBtnBlue(){
  if(screen==START){
    GAMEON=false;
    if(capture_time>0){
      capture_time--;
    }
    else
      capture_time=999;

    TIME=capture_time;
    update_start_screen();
  }

  if(screen==LIMIT && GAMEON==false){
    score_limit--;
    limit_set_update();
  }

  if(screen==INTERVAL && GAMEON==false){
    if(TIMER>0)
      TIMER--;
    else
      TIMER=capture_time;

    set_inteval_update();
  }

  if(screen==MODE && GAMEON==false && cursr > 0){
    cursr--;
    select_mode();
  }
}

void onClickBtnStart(){
  if(screen==START && GAMEON==false){
    screen=LIMIT;
    limit_set();
    return;
  }

  if(screen==LIMIT && GAMEON==false){
    screen=INTERVAL;
    set_inteval();
    return;
  }

  if(screen==INTERVAL && GAMEON==false){
    screen=MODE;
    select_mode();
    return;
  }

  if(screen==MODE && GAMEON==false){
    mode = cursr + 1;
    //if(mode == 1) score_limit=(int)score_limit/2;
    screen=PENDING;
    pending_screen();
    return;
  }

  if(screen==PENDING && GAMEON==false){
    screen=GAME;
    GAMEON=true;
    starttime=millis();
    game_screen();
    return;
  }
}

void onClickBtnReset(){
  digitalWrite(BUZZER,LOW);
  GAMEON=false;
  A=false;
  B=false;
  digitalWrite(LED_A,LOW);
  digitalWrite(LED_B,LOW);
  capture_time=TIME;
  points_A=0;
  points_B=0;
  xbeeStr == "";
  start_screen();
}

void duringLongPressBtnRed(){
  redReleased = false;
  if(rt<20){
    lcd.setCursor(rt,3);
    lcd.print(">");
    A=false;
    B=false;
    for(int k = 0; k < TIMER; k++){
      delay(1000);
      capture_time--;
      game_screen_time_update();
    }
    rt++;
  }
}

void duringLongPressBtnBlue(){
  blueReleased = false;
  if(bt<20){
    lcd.setCursor(19-bt,3);
    lcd.print("<");
    A=false;
    B=false;
    for(int k = 0; k < TIMER; k++){
      delay(1000);
      capture_time--;
      game_screen_time_update();
      game_red_pt_update();
      game_blue_pt_update();
    }
    bt++;
  }
}

void longPressStopBtnRed(){

}

void longPressStopBtnBlue(){

}








