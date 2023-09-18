#include <StandardCplusplus.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <utility.h>
#include <vector>
#include <Keypad.h>
#include <EEPROM.h>
#include <MemoryFree.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <OneButton.h>

#define BUFFSIZE 19

std::vector<uint16_t> boxIDACache, boxIDBCache;

OneButton btnRed(A0, true);
OneButton btnBlue(A1, true);
OneButton btnStart(A2, true);
OneButton btnReset(A3, true);
OneButton btnCfg(A4, true);
OneButton btnBuzz(A5, true);

LiquidCrystal lcd(50, 51, 22, 23, 24, 25);

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {
    '1','2','3','A'                                                                      }
  ,{
    '4','5','6','B'                                                                      }
  ,{
    '7','8','9','C'                                                                      }
  ,{
    '*','0','#','D'                                                                      }
};
byte rowPins[ROWS] = {
  A15,A14, A13, A12}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {
  A11, A10, A9, A8}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//uint8_t box_id = 0;

typedef enum {
  NUM_BOX,
  WIN_BOX,
  ID,
  CAP_TIME,
  LIMIT,
  INTERVAL,
  LNCH_TIMER,
  MODE,
  PENDING,
  GAME,
  END,
  TRX
} 
SCREEN;

SCREEN screen = PENDING;

String modeList[] = {
  "Domination Mode","Rush Mode", "Push Mode", "Launch Mode",
  "Critical Mass Mode", "Dead Man Mode", "TRX Test Mode"};

String numHold = "";

uint16_t capture_time = 0, captime, lnchtimer,
numbox, winbox, boxid, boxReduceId = 0, limit, countbox = 0, cntRBox;

uint8_t mode, timer, deact_A = 0, deact_B = 0, a = 0, b = 0,
blinkColL, blinkColR, blinkCol, blinkRow;

uint16_t points_A = 0, points_B = 0, score_A = 0, score_B = 0,
boxes_A = 0, boxes_B = 0, rt = 0, bt = 0, boxHold;

boolean GAMEON = false, A = false, B = false,
uncapture = false, boxReduced = false, boxReducedReq = false, 
bl = false, update = false, coupling = false,
redReleased = true, blueReleased = true;
//int SCREEN=0;//0-off 1-start 2-score limit set 3-interval set 4-pending 5-gameon

long time = 0, starttime = 0, starttime2 = 0;

char capturer = 'D';

uint16_t LED_A = 26,LED_B = 27,BUZZER = 28, BUZZER_BECON = 29;

//XBEE START
byte xbeeStr[BUFFSIZE], serRcvStr[4]; 

//byte *xbeeSBuff  = (byte *)malloc(sizeof(byte));
char xbeeSBuff[] = {
  'S',
  0x01,//T  1
  0x01,
  0x01,//A  3
  0x01,
  0x01,//a  5
  0x01,
  0x01,//B  7
  0x01,
  0x01,//b  9
  0x01,
  0x01,//boxes_A 11
  0x01,
  0x01,//boxes_B 13
  0x01,
  0x01,//boxid   15
  0x01,
  'E',
  '\0'};

char serSndBuff[] = {
  'B',
  0x01,
  0x01,
  'I',
  '/0'
};

uint8_t pinRx = 10, pinTx = 11; // the pin on Arduino
SoftwareSerial xbee( pinRx , pinTx );
//XBEE END

void setup() {
  numHold.reserve(4);
  //keypad.addEventListener(keypadEvent);
  btnRed.setClickTicks(100);
  btnBlue.setClickTicks(100);
  btnStart.setClickTicks(100);
  btnReset.setClickTicks(100);
  btnBuzz.setClickTicks(100);
  btnCfg.setClickTicks(500);

  btnRed.setPressTicks(500);
  btnBlue.setPressTicks(500);
  btnStart.setPressTicks(500);
  btnReset.setPressTicks(500);

  btnRed.attachClick(onClickBtnRed);
  btnBlue.attachClick(onClickBtnBlue);
  btnStart.attachClick(onClickBtnStart);
  btnReset.attachClick(onClickBtnReset);
  btnCfg.attachClick(onClickBtnCfg);
  btnBuzz.attachClick(onClickBtnBuzz);

  btnRed.attachDuringLongPress(duringLongPressBtnRed);
  btnBlue.attachDuringLongPress(duringLongPressBtnBlue);
  //btnRed.attachLongPressStop(longPressStopBtnRed);
  //btnBlue.attachLongPressStop(longPressStopBtnBlue);

  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  pinMode(BUZZER_BECON,OUTPUT);

  //  pinMode(10,OUTPUT);
  //analogWrite(10,122);
  //analogWrite(6, 172);
  digitalWrite(BUZZER,LOW);
  digitalWrite(BUZZER_BECON,LOW);
  digitalWrite(LED_A,HIGH);
  digitalWrite(LED_B,HIGH);

  //Mode
  mode = EEPROM.read(0);
  if(mode == 255){
    mode = 1;
    EEPROM.write(0, mode);
  }
  //Num Box
  numbox = EEPROM.read(1)*256+EEPROM.read(2);
  if(numbox == 65535){
    numbox = 1;
    EEPROM.write(1, 0);
    EEPROM.write(2, 1);
  }
  //Win Box
  winbox = EEPROM.read(3)*256+EEPROM.read(4);
  if(winbox == 65535){
    winbox = 1;
    EEPROM.write(3, 0);
    EEPROM.write(4, 1);
  }
  //Box ID
  boxid = EEPROM.read(5)*256+EEPROM.read(6);
  if(boxid == 65535){
    boxid = 0;
    EEPROM.write(5, 0);
    EEPROM.write(6, 0);
  }
  //TIME
  captime = EEPROM.read(7)*256+EEPROM.read(8);
  if(captime == 65535){
    captime = 900;
    EEPROM.write(7, floor(captime/256));
    EEPROM.write(8, captime%256);
  }
  capture_time = captime;
  //LIMIT
  limit = EEPROM.read(9)*256+EEPROM.read(10);
  if(limit == 65535){
    limit = 40;
    EEPROM.write(9, 0);
    EEPROM.write(10, limit);
  }
  //TIMER
  timer = EEPROM.read(11);
  if(timer == 255){
    timer = 1;
    EEPROM.write(11, timer);
  }
  //LNCHTIMER
  lnchtimer = EEPROM.read(12)*256+EEPROM.read(13);
  if(lnchtimer == 65535){
    lnchtimer = 25;
    EEPROM.write(12, 0);
    EEPROM.write(13, lnchtimer);
  }
  //captime = 50;
  capture_time = captime;
  if(mode == 6) limit = captime;
  //limit = 10;
  //EEPROM.write(12,255);
  //EEPROM.write(13,255);
  /*for(int i = 0; i < 14; i++){
   EEPROM.write(i,255);
   }*/

  Serial.begin(9600);
  Serial1.begin(9600);
  xbee.begin(19200);
  lcd.begin(20, 4);
  screen_select();
}

void loop() {
  //char key = keypad.getKey();

  //if (key){
  //Serial.println(key);
  //}
  keyPadActions();
  btnRed.tick();
  btnBlue.tick();
  btnStart.tick();
  btnReset.tick();
  btnCfg.tick();
  btnBuzz.tick();

  if((millis()-starttime)>=1000){
    if(A == true){
      if(digitalRead(LED_A) == HIGH) digitalWrite(LED_A, LOW);
      else digitalWrite(LED_A, HIGH);
    }
    if(B == true){
      if(digitalRead(LED_B) == HIGH) digitalWrite(LED_B, LOW);
      else digitalWrite(LED_B, HIGH);
    }
    if(mode == 5) {
      serSndBuff[1] = floor(boxid / 256) + 1;
      serSndBuff[2] = (boxid % 256) + 1;
      Serial1.print(serSndBuff);
    }
    Serial.println("cntRBox "+String(cntRBox));
    cntRBox = 0;
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());
    Serial.println("SA:"+String(score_A)+"SB:"+String(score_B)
      +"BA:"+String(boxes_A)+"BB:"+String(boxes_B));
    for(uint8_t i = 0; i < sizeof(xbeeStr); i++){
      xbeeStr[i] = 0x00;
    }
    sendData();
    if(GAMEON==true) {
      if(capture_time > 0) capture_time--;
      if(screen == GAME) game_screen_time_update();
    }

    if(mode == 4 && screen == END){
      if(A == true){
        if(lnchtimer == EEPROM2BytesRead(12,13)) boxHold = boxes_A;
        lnch_time_remainning();
        if(lnchtimer > 0)lnchtimer--;
      }
      if(B == true){
        if(lnchtimer == EEPROM2BytesRead(12,13)) boxHold = boxes_B;
        lnch_time_remainning();
        if(lnchtimer > 0)lnchtimer--;
      }
    }

    if((millis()-starttime2)>=1000 * timer){
      if(((mode != 3 && mode != 4 && mode != 5)
        || (mode == 3 && boxid == boxes_A + 1)
        || (mode == 4 && deact_A == 0)
        || (mode == 5 && coupling))
        && (!redReleased && rt<20 && screen == GAME && !A)){
        if(mode == 5) rt = 20;
        else{
          lcd.setCursor(rt,3);
          lcd.print(F(">"));
          //A=false;
          //B=false;
          rt++;
        }
      }
      if(((mode != 3 && mode != 4 && mode != 5)
        || (mode == 3 && numbox - boxid == boxes_B)
        || (mode == 4 && deact_B == 0)
        || (mode == 5 && coupling)) 
        && (!blueReleased && bt<20 && screen == GAME && !B)){
        if(mode == 5) rt = 20;
        else{
          lcd.setCursor(19-bt,3);
          lcd.print(F("<"));
          bt++;
        }
      }
      starttime2 = millis();
    }

    starttime=millis();
  }

  if(screen==GAME){
    if(!btnRed.isLongPressed() && !redReleased){
      if(mode == 6) A = false;
      if(rt>=20){
        time=millis();
        A=true;
        B=false;
        //digitalWrite(LED_A,HIGH);
        //digitalWrite(LED_B,HIGH);
      }
      rt = 0;
      redReleased = true;
      screen_select();
    }

    if(!btnBlue.isLongPressed() && !blueReleased){
      if(mode == 6) B = false;
      if(bt>=20){
        time=millis();
        A=false;
        B=true;
        //digitalWrite(LED_A,HIGH);
        //digitalWrite(LED_B,HIGH);
      }
      bt = 0;
      blueReleased = true;
      screen_select();
    }
  }
  captureCalc();
  XbeeRCalc();
  if(mode == 5) ser1RCalc();
}

void sendData(){
  if(uncapture){
    if(countbox + 1 == boxid) countbox++;
    xbeeSBuff[0] = 'B';
    xbeeSBuff[1] = capturer;
    xbeeSBuff[2] = floor(boxid / 256) + 1;
    xbeeSBuff[3] = (boxid % 256) + 1;
    xbeeSBuff[4] = 0x01;
    xbeeSBuff[5] = 0x01;
    xbeeSBuff[6] = 'R';
  }
  else{
    xbeeSBuff[0] = 'S';
    xbeeSBuff[1] = floor(capture_time / 256) + 1;
    xbeeSBuff[2] = (capture_time % 256) + 1;
    xbeeSBuff[3] = floor(score_A / 256) + 1;
    xbeeSBuff[4] = (score_A % 256) + 1;
    xbeeSBuff[7] = floor(score_B / 256) + 1;
    xbeeSBuff[8] = (score_B % 256) + 1;
    if(update || mode == 4){
      Serial.println("Update true");
      if(mode == 2 || mode == 3 || mode == 4 || mode == 5){
        xbeeSBuff[5] = floor(deact_A / 256) + 1;
        xbeeSBuff[6] = (deact_A % 256) + 1;
        xbeeSBuff[9] = floor(deact_B / 256) + 1;
        xbeeSBuff[10] = (deact_B % 256) + 1;
      }
      else{
        xbeeSBuff[5] = floor(points_A / 256) + 1;
        xbeeSBuff[6] = (points_A % 256) + 1;
        xbeeSBuff[9] = floor(points_B / 256) + 1;
        xbeeSBuff[10] = (points_B % 256) + 1;
      }
    }
    else{
      xbeeSBuff[5] = 0x01;
      xbeeSBuff[6] = 0x01;
      xbeeSBuff[9] = 0x01;
      xbeeSBuff[10] = 0x01;
    }
    xbeeSBuff[11] = floor(boxes_A / 256) + 1;
    xbeeSBuff[12] = (boxes_A % 256) + 1;
    xbeeSBuff[13] = floor(boxes_B / 256) + 1;
    xbeeSBuff[14] = (boxes_B % 256) + 1;
    xbeeSBuff[15] = floor(boxid / 256) + 1;
    xbeeSBuff[16] = (boxid % 256) + 1;
    xbeeSBuff[17] = 'E';
  }

  xbee.write(xbeeSBuff);
  Serial.println("Points1:"+String(points_A)+" "+String(points_B));
  Serial.println("Scores1:"+String(score_A)+" "+String(score_B));
  Serial.println("xbeeSBuff[5]:"+String(xbeeSBuff[5],HEX)+" xbeeSBuff[6]:"+String(xbeeSBuff[6],HEX));
  Serial.println("xbeeSBuff[9]:"+String(xbeeSBuff[9],HEX)+" xbeeSBuff[10]:"+String(xbeeSBuff[10],HEX));
}

void ser1RCalc(){
  if(Serial1.available() && (serRcvStr[b] = Serial1.read()) > 0){
    //serRcvStr[b] = Serial1.read();
    Serial.println("b:"+String(b)+" "+String(serRcvStr[b]));
    if(serRcvStr[0] == 'B'){
      b++;
      if(b == 4) b = 0;
    }
    if(serRcvStr[0] == 'B' && serRcvStr[3] == 'I'){
      Serial.println(String(b)+" "+String((serRcvStr[1] - 1) * 256+serRcvStr[2] - 1));
      if((serRcvStr[1] - 1) * 256 + serRcvStr[2] - 1 != boxid) {
        Serial.println("OK");
        coupling = true;
      }
    }
  }
  Serial1.flush();
}

void XbeeRCalc(){
  if (xbee.available()) {
    xbeeStr[a] = (byte)xbee.read();
    Serial.print(String(a)+" ");
    Serial.println(xbeeStr[a],HEX);
    if(xbeeStr[0] == 'S' 
      || xbeeStr[0] == 'B' 
      || xbeeStr[0] == 'T' 
      || xbeeStr[0] == 'U'
      || xbeeStr[0] == 'C') a++;
    else a = 0;

    if(a >= BUFFSIZE - 1){
      a = 0;
      cntRBox++;
      if(xbeeStr[0] == 'S' && xbeeStr[BUFFSIZE-2] == 'E'){
        uint16_t AT = (xbeeStr[3] - 1) * 256 + xbeeStr[4] - 1;
        uint16_t AI = (xbeeStr[5] - 1) * 256 + xbeeStr[6] - 1;
        uint16_t BT = (xbeeStr[7] - 1) * 256 + xbeeStr[8] - 1;
        uint16_t BI = (xbeeStr[9] - 1) * 256 + xbeeStr[10] - 1;
        uint16_t BA = (xbeeStr[11] - 1) * 256 + xbeeStr[12] - 1;
        uint16_t BB = (xbeeStr[13] - 1) * 256 + xbeeStr[14] - 1;
        uint16_t Bid = (xbeeStr[15] - 1) * 256 + xbeeStr[16] - 1;

        if(boxReduceId > 0) {
          boxReduceId = 0;
          boxReduced = false;
          boxReducedReq = false;
        }

        if(screen == END || screen == GAME){
          Serial.print(F("T1 "));
          Serial.println(capture_time);
          Serial.println("DiffBA:"+String(BA)+" "
            +String(boxes_A)+"DiffB:"+String(BB)+" "+String(boxes_B));

          Serial.println("5:"+String(xbeeStr[5])+" 6:"+String(xbeeStr[6]));

          Serial.println("AI:"+String(AI)+" BI:"+String(BI)
            +" AT:"+String(AT)+" BT:"+String(BT)
            +" BA:"+String(BA)+" BB:"+String(BB)
            +" ScoreA:"+String(score_A)+" scoreB:"+String(score_B));

          if(BA > boxes_A) boxes_A += BA - boxes_A;
          if(BB > boxes_B) boxes_B += BB - boxes_B;

          if(mode == 4){
            deact_A = AI;
            deact_B = BI;
          }
          else{
            boolean bxIDAFnd = false, bxIDBFnd = false;
            for(uint16_t i = 0; i < boxIDACache.size(); i++){
              if(boxIDACache[i] == Bid){
                bxIDAFnd = true;
                break;
              }
            }
            for(uint16_t i = 0; i < boxIDBCache.size(); i++){
              if(boxIDBCache[i] == Bid){
                bxIDBFnd = true;
                break;
              }
            }

            if(AI == 0){
              if(bxIDAFnd){
                for(uint16_t i = 0; i < boxIDACache.size(); i++){
                  if(boxIDACache[i] == Bid){
                    boxIDACache.erase(boxIDACache.begin()+i);
                    bxIDAFnd = false;
                    break;
                  }
                }
              }
              if(AT > score_A) score_A = AT;
            }
            if(BI == 0){
              if(bxIDAFnd){
                for(uint16_t i = 0; i < boxIDBCache.size(); i++){
                  if(boxIDBCache[i] == Bid){
                    boxIDBCache.erase(boxIDACache.begin()+i);
                    bxIDBFnd = false;
                    break;
                  }
                }
              }
              if(BT > score_B) score_B = BT;
            }

            if(AI > 0 || BI > 0){
              if(!bxIDAFnd){
                boxIDACache.push_back(Bid);
                score_A += AI;
              }
              if(!bxIDBFnd){
                boxIDBCache.push_back(Bid);
                score_B += BI;
              }
              xbeeSBuff[0] = 'U';
              xbeeSBuff[1] =floor(Bid / 256) + 1;
              xbeeSBuff[2] = (Bid % 256) + 1;
              xbeeSBuff[3] = 'P';
              xbee.write(xbeeSBuff);
            }
          }
          if(mode == 2 || mode == 3 || mode == 4 || mode == 5) {
            score_A = boxes_A;
            score_B = boxes_B;
          }
          Serial.println("PA1"+String(score_A));
          Serial.println("PB1"+String(score_B));
          if(screen == END){
            red_score_update();
            blue_score_update();
          }
        }
        else{
          capture_time = (xbeeStr[1] - 1) * 256 + xbeeStr[2] - 1;;
          Serial.print(F("T2 "));
          Serial.println(capture_time);
          score_A = AT;
          Serial.println("SA2"+String(score_A));
          score_B = BT;
          Serial.println("SB2"+String(score_B));
        }
      }
      if(xbeeStr[0] == 'C' && xbeeStr[BUFFSIZE-2] == 'G' && screen == PENDING){
        Serial.println("Works");
        Serial.println(String(xbeeStr[1] - 1)+
        " "+String(xbeeStr[1] - 1)+
        " "+String((xbeeStr[8] - 1) * 256 + xbeeStr[9] - 1));
        /*mode = xbeeStr[1] - 1;
        numbox = (xbeeStr[2] - 1) * 256 + xbeeStr[3] - 1;
        winbox = (xbeeStr[4] - 1) * 256 + xbeeStr[5] - 1;
        boxid = (xbeeStr[6] - 1) * 256 + xbeeStr[7] - 1;
        captime = (xbeeStr[8] - 1) * 256 + xbeeStr[9] - 1;
        timer = xbeeStr[10] - 1;
        limit = (xbeeStr[11] - 1) * 256 + xbeeStr[12] - 1;
        lnchtimer = (xbeeStr[13] - 1) * 256 + xbeeStr[14] - 1;
        capture_time = captime;*/
      }
      if(xbeeStr[0] == 'U' && xbeeStr[3] == 'P'){
        uint16_t Bid = (xbeeStr[1] - 1) * 256 + xbeeStr[2] - 1;
        if(Bid == boxid) update = false;
      }
      if(xbeeStr[0] == 'B' && xbeeStr[6] == 'R'
        && ((xbeeStr[2] - 1) * 256 + xbeeStr[3] - 1 == boxReduceId
        || boxReduceId == 0)){
        if(xbeeStr[1] == 'D')  boxReducedReq = false;
        if(!boxReduced){
          boxReduceId = (xbeeStr[2] - 1) * 256 + xbeeStr[3] - 1;;
          if(xbeeStr[1] == 'A'){
            boxes_A--;
            score_A = boxes_A;
            if(score_A == 0) deact_A = 0;
          }
          if(xbeeStr[1] == 'B'){
            boxes_B--;
            score_B = boxes_B;
            if(score_A == 0) deact_B = 0;
          }
          boxReduced = true;
          boxReducedReq = true;
        }
        if(boxReducedReq){
          xbeeSBuff[0] = 'B';
          xbeeSBuff[1] =floor(boxid / 256) + 1;
          xbeeSBuff[2] = (boxid % 256) + 1;
          xbeeSBuff[3] = xbeeStr[2];
          xbeeSBuff[4] = xbeeStr[3];
          xbeeSBuff[5] = 'O';
          xbee.write(xbeeSBuff);
        }
      }
      if(xbeeStr[0] == 'B' && xbeeStr[5] == 'O'
        && (xbeeStr[3] - 1) * 256 + xbeeStr[4] - 1 == boxid){

        uint16_t brid = (xbeeStr[1] - 1) * 256 + xbeeStr[2] - 1;;
        //boolean boxExist = false;
        if(brid == countbox+1){
          countbox++;
          if(countbox == boxid) countbox++;
        }
        if(brid <= countbox){
          xbeeSBuff[0] = 'B';
          xbeeSBuff[1] = 'D';
          xbeeSBuff[2] =floor(boxid / 256) + 1;
          xbeeSBuff[3] = (boxid % 256) + 1;
          xbeeSBuff[4] =floor(brid / 256) + 1;
          xbeeSBuff[5] = (brid % 256) + 1;
          xbeeSBuff[6] = 'R';
          xbee.write(xbeeSBuff);
        }
        if(countbox == numbox-1){
          uncapture = false;
          countbox = 0;
        }
      }
      if(xbeeStr[0] == 'B' && xbeeStr[1] == 'Z'){
        digitalWrite(BUZZER_BECON,HIGH);
      }
      if(xbeeStr[0] == 'T' && xbeeStr[1] == 'R'){
        xbee.print("OK");
      }
      if(screen == TRX && xbeeStr[0] == 'O' && xbeeStr[1] == 'K'){
        lcd.setCursor(0,1);
        lcd.print(F("TRX Test Successful!"));
      }
      for(uint8_t i = 0; i < sizeof(xbeeStr); i++){
        xbeeStr[i] = 0x00;
      }
    }
  }
  xbee.overflow();
}

void captureCalc(){
  if(A==true && screen==GAME){
    if(mode == 3 || mode == 4 || (mode == 5 && coupling)) points_A = limit;
    else if(millis() - time > 1000){
      time=millis();
      points_A++;
      game_red_pt_update();
    }
  }

  if(B==true && screen==GAME){
    if(mode == 3 || mode == 4 || (mode == 5 && coupling)) points_B = limit;
    else if(millis() - time > 1000){
      time=millis();
      points_B++;
      game_blue_pt_update();
    }
  }
  if(screen != END
    && (boxes_A + boxes_B >= numbox
    || ((mode == 2 || mode == 4 || mode == 5)
    && (boxes_A >= winbox 
    || boxes_B >= winbox))
    || (mode == 3
    && (boxes_A > (uint16_t)floor(numbox/2)
    || boxes_B > (uint16_t)floor(numbox/2))))) result_show();

  if(mode == 4 && screen==END && lnchtimer <= 0){
    if(A==true){
      if(boxes_A <= boxHold) deact_A = 1;
    }
    if(B==true){
      if(boxes_B <= boxHold) deact_B = 1;
    }
    //update = true;
    /*xbeeSBuff[5] =floor(act_A / 256) + 1;
     xbeeSBuff[6] = (act_A % 256) + 1;
     xbeeSBuff[9] =floor(act_B / 256) + 1;
     xbeeSBuff[10] = (act_B % 256) + 1;*/
  }
  //Serial.println("ActA"+String(act_A)+"ActB"+String(act_B));
  if(screen != END && (points_A >= limit 
    || points_B >= limit
    || capture_time <= 0)){
    update = true;
    //Serial.println("Acts:"+String(act_A)+" "+String(act_B));
    /*xbeeSBuff[1] =floor(capture_time / 256) + 1;
     xbeeSBuff[2] = (capture_time % 256) + 1;
     xbeeSBuff[3] =floor(score_A / 256) + 1;
     xbeeSBuff[4] = (score_A % 256) + 1;
     xbeeSBuff[7] =floor(score_B / 256) + 1;
     xbeeSBuff[8] = (score_B % 256) + 1;*/

    if(mode == 1 || mode == 6){
      //update = true;
      Serial.println("Points:"+String(points_A)+" "+String(points_B));
      Serial.println("Scores:"+String(score_A)+" "+String(score_B));
      /*xbeeSBuff[5] =floor(points_A / 256) + 1;
       xbeeSBuff[6] = (points_A % 256) + 1;
       xbeeSBuff[9] =floor(points_B / 256) + 1;
       xbeeSBuff[10] = (points_B % 256) + 1;
       Serial.println("PointsByte:"+String(xbeeSBuff[5],HEX)+" "+String(xbeeSBuff[6],HEX));*/
      score_A += points_A;
      score_B += points_B;
    }

    //int one = 1; 
    //int zero = 0;
    if(points_A>points_B){
      boxes_A++;
      if(mode == 2 || mode == 3 || mode == 4 || mode == 5){
        if(mode != 4){
          deact_A = 1;
          deact_B = 0;
          //score_A ++;
        }
        score_A = boxes_A;
        /*xbeeSBuff[5] =floor(act_A / 256) + 1;
         xbeeSBuff[6] = (act_A % 256) + 1;
         xbeeSBuff[9] =floor(act_B / 256) + 1;
         xbeeSBuff[10] = (act_B % 256) + 1;*/
      }
      capturer = 'A';
    }
    else if(points_A<points_B){
      boxes_B++;
      if(mode == 2 || mode == 3 || mode == 4 || mode == 5){
        if(mode != 4){
          deact_A = 0;
          deact_B = 1;
          //score_B ++;
        }
        score_B = boxes_B;
        /*xbeeSBuff[5] =floor(act_A / 256) + 1;
         xbeeSBuff[6] = (act_A % 256) + 1;
         xbeeSBuff[9] =floor(act_B / 256) + 1;
         xbeeSBuff[10] = (act_B % 256) + 1;*/
      }
      capturer = 'B';
    }
    else {
      capturer = 'D';
    }
    /*xbeeSBuff[11] =floor(boxes_A / 256) + 1;
     xbeeSBuff[12] = (boxes_A % 256) + 1;
     xbeeSBuff[13] =floor(boxes_B / 256) + 1;
     xbeeSBuff[14] = (boxes_B % 256) + 1;*/
    //xbee.write(xbeeSBuff);
    sendData();
    result_show();
  }

  if(screen == END
    && (boxes_A + boxes_B >= numbox 
    || capture_time <= 0
    || ((mode == 2 || mode == 4 || mode == 5)
    && (boxes_A >= winbox 
    || boxes_B >= winbox))
    || (mode == 3
    && (boxes_A > (uint16_t)floor(numbox/2)
    || boxes_B > (uint16_t)floor(numbox/2))))){
    uint8_t aaa = boxes_A + boxes_B;
    if(mode == 4) lnchtimer = 0;
    //Serial.println("BxA:"+String(boxes_A)+" BxB:"+String(boxes_A));
    if(((mode == 1  || mode == 6) && score_A > score_B)
      ||((mode == 2 || mode == 4 || mode == 5) 
      && boxes_A >= winbox)
      || (mode == 3
      && boxes_A > (uint16_t)floor(numbox/2))){
      lcd.setCursor(0,4);
      lcd.print(F("    RED TEAM WON    "));
    }
    else if(((mode == 1  || mode == 6) && score_A < score_B)
      ||((mode == 2 || mode == 4 || mode == 5) 
      && boxes_B >= winbox)
      || (mode == 3
      && boxes_B > (uint16_t)floor(numbox/2))){
      lcd.setCursor(0,4);
      lcd.print(F("   BLUE TEAM WON    "));
    }
    if(score_A == score_B || boxes_A >= winbox && boxes_B >= winbox){
      lcd.setCursor(0,4);
      lcd.print(F("     MATCH DRAWN    "));
    }
  }
}

void screen_select(){
  lcd.noBlink();
  switch (screen){
  case MODE:
    {
      select_mode();
      break;
    }
  case NUM_BOX:
    {
      num_box();
      break;
    }
  case WIN_BOX:
    {
      win_box();
      break;
    }
  case ID:
    {
      id_screen();
      break;
    }
  case CAP_TIME:
    {
      cap_time();
      break;
    }
  case LIMIT:
    {
      limit_set();
      break;
    }
  case INTERVAL:
    {
      set_interval();
      break;
    }
  case LNCH_TIMER:
    {
      launch_timer();
      break;
    }
  case PENDING:
    {
      pending_screen();
      break;
    }
  case GAME:
    {
      game_screen();
      break;
    }
  case END:
    {
      result_show();
      break;
    }
  case TRX:
    {
      trx_test();
      break;
    }
  default:
    break;
  };
}

void screen_nav(){
  lcd.setCursor(0,3);
  lcd.print(F("<A   B>    C = NEXT "));
  lcd.setCursor(blinkCol,blinkRow);
  lcd.blink();
}

void num_box(){
  blinkColL = 4;
  blinkColR = blinkColL+3;
  blinkCol = blinkColL;
  blinkRow = 1;
  numHold = String(numbox);
  while(numHold.length() < blinkColR-blinkColL+1){
    numHold += " ";
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("SET NUMBER OF BOXES"));
  lcd.setCursor(blinkCol,blinkRow);
  lcd.print(numbox,10);
  lcd.setCursor(9,1);
  lcd.print(F("BOXES"));
  screen_nav();
  screen=NUM_BOX;
}

void update_num_boxes(){
  lcd.setCursor(4,1);
  lcd.print(numbox,10);
}

void win_box(){
  blinkColL = 4;
  blinkColR = blinkColL+3;
  blinkCol = blinkColL;
  blinkRow = 1;
  numHold = String(winbox);
  while(numHold.length() < blinkColR-blinkColL+1){
    numHold += " ";
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("    BOXES TO WIN    "));
  lcd.setCursor(blinkCol,blinkRow);
  lcd.print(winbox,10);
  lcd.setCursor(9,1);
  lcd.print(F("BOXES"));
  screen_nav();
  lcd.setCursor(blinkCol,blinkRow);
}

void update_win_boxes(){
  lcd.setCursor(4,1);
  lcd.print(winbox,10);
}

void id_screen(){
  blinkColL = 13;
  blinkColR = blinkColL+3;
  blinkCol = blinkColL;
  blinkRow = 1;
  numHold = String(boxid);
  while(numHold.length() < blinkColR-blinkColL+1){
    numHold += " ";
  }
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print(F("  Set box ID: "));
  lcd.setCursor(blinkCol,blinkRow);
  lcd.print(boxid,10);
  screen_nav();
  screen=ID;
}

void update_id_screen(){
  lcd.setCursor(15,1);
  lcd.print(boxid,10);
}

void cap_time(){
  blinkColL = 4;
  blinkColR = blinkColL+3;
  blinkCol = blinkColL;
  blinkRow = 1;
  numHold = String(captime);
  while(numHold.length() < blinkColR-blinkColL+1){
    numHold += " ";
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("  SET CAPTURE TIME  "));
  lcd.setCursor(blinkCol,blinkRow);
  lcd.print(captime,10);
  lcd.setCursor(9,1);
  lcd.print(F("SECONDS"));
  screen_nav();
  screen=CAP_TIME;
}

void update_time_screen(){
  lcd.setCursor(4,1);
  lcd.print(captime,10);
}

void limit_set(){
  blinkColL = 4;
  blinkColR = blinkColL+3;
  blinkCol = blinkColL;
  blinkRow = 1;
  numHold = String(limit);
  while(numHold.length() < blinkColR-blinkColL+1){
    numHold += " ";
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("  SET SCORE LIMIT"));
  lcd.setCursor(blinkCol,blinkRow);
  lcd.print(limit,10);
  lcd.setCursor(9,1);
  lcd.print(F("POINTS"));
  screen_nav();
  screen=LIMIT;
}

void limit_set_update(){
  lcd.setCursor(4,1);
  lcd.print(limit,10);
}

void set_interval(){
  blinkColL = 7;
  blinkColR = blinkColL+3;
  blinkCol = blinkColL;
  blinkRow = 1;
  numHold = String(timer);
  while(numHold.length() < blinkColR-blinkColL+1){
    numHold += " ";
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F(" SET CAPTURE TIMER"));
  lcd.setCursor(blinkCol,blinkRow);
  lcd.print(timer,10);
  lcd.setCursor(9,1);
  lcd.print(F("X"));
  lcd.setCursor(0,2);
  lcd.print(F("MULTIPLES OF 20 SEC."));
  screen_nav();
  screen=INTERVAL;
}

void set_interval_update(){
  lcd.setCursor(7,1);
  lcd.print(timer,10);
}

void launch_timer(){
  blinkColL = 4;
  blinkColR = blinkColL+3;
  blinkCol = blinkColL;
  blinkRow = 1;
  numHold = String(lnchtimer);
  while(numHold.length() < blinkColR-blinkColL+1){
    numHold += " ";
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("  SET LAUNCH TIMER"));
  lcd.setCursor(blinkCol,blinkRow);
  lcd.print(lnchtimer,10);
  lcd.setCursor(9,1);
  lcd.print(F("SECONDS"));
  screen_nav();
  screen=LNCH_TIMER;
}

void update_lnchtimer(){
  lcd.setCursor(4,1);
  lcd.print(lnchtimer, 10);
}

void lnch_time_remainning(){
  lcd.setCursor(0,3);
  lcd.print("Time remainning:     ");
  lcd.setCursor(17,3);
  lcd.print(lnchtimer, 10);
}

void select_mode(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Select Game Mode:"));
  for(uint8_t i = mode-1 ; i < mode+1 && i < (sizeof(modeList)/sizeof(String)); i++){
    lcd.setCursor(1,i-mode+2);
    //lcd.print(capture_time,10);
    //lcd.setCursor(9,1);
    String str = "";
    if(modeList[i].length() > 15){
      str = modeList[i].substring(0, 15);
    }
    else str = modeList[i];
    lcd.print(((i==mode-1)?">":" ")+String(i+1)+". "+str);
    if(modeList[i].length() > 15){
      lcd.setCursor(5,i-mode+3);
      lcd.print(modeList[i].substring(15, modeList[i].length()));
      i++;
    }
  }
  lcd.setCursor(0,3);
  lcd.print(F("R=+, B=-, START=NEXT"));
  screen=MODE;
}

void pending_screen(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("REMAINING TIME:"));
  lcd.print(capture_time,10);
  lcd.setCursor(0,1);
  if(mode == 3){
    lcd.print(F("Box ID: "));
    lcd.print(boxid,10);
  }
  else{
    lcd.print(F("RED  POINTS: "));
    lcd.print(points_A,10);
    lcd.setCursor(0,2);
    lcd.print(F("BLUE POINTS: "));
    lcd.print(points_B,10);
  }
  lcd.setCursor(0,3);
  lcd.print(F("PRESS START BUTTON"));
  screen=PENDING;
}

void game_screen(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("REMAINING TIME:      "));
  lcd.setCursor(15,0);
  lcd.print(capture_time,10);
  lcd.setCursor(0,1);
  if(mode == 3){
    lcd.print(F("Box ID: "));
    lcd.print(boxid,10);
  }
  else if(mode != 4 && mode != 5){
    lcd.print(F("RED  POINTS: "));
    lcd.print(points_A,10);
    lcd.setCursor(0,2);
    lcd.print(F("BLUE POINTS: "));
    lcd.print(points_B,10);
  }
  lcd.setCursor(0,3);
  screen=GAME;
}

void game_screen_time_update(){
  lcd.setCursor(15,0);
  lcd.print("     ");
  lcd.setCursor(15,0);
  lcd.print(capture_time,10);
}

void game_red_pt_update(){
  lcd.setCursor(13,1);
  lcd.print(points_A,10);
}

void game_blue_pt_update(){
  lcd.setCursor(13,2);
  lcd.print(points_B,10);
}

void result_show(){
  lcd.clear();
  if(capturer == 'A'){
    lcd.setCursor(1,0);
    lcd.print(F("RED TEAM CAPTURED"));
    //digitalWrite(LED_B,HIGH);
  }
  else if(capturer == 'B'){
    lcd.setCursor(1,0);
    lcd.print(F("BLUE TEAM CAPTURED"));
    //digitalWrite(LED_A,HIGH);
  }
  else{
    lcd.setCursor(5,0);
    lcd.print(F("BOX DRAWN"));
    //digitalWrite(LED_A,LOW);
    //digitalWrite(LED_B,LOW);
  }
  lcd.setCursor(1,1);
  if(mode==1 || mode == 6) lcd.print(F("Red Score: "));
  else if(mode==2 || mode == 3 || mode == 4 || mode == 5) lcd.print(F("Red Boxes: "));
  lcd.setCursor(11,1);
  lcd.print(score_A,10);
  lcd.setCursor(0,2);
  if(mode==1 || mode == 6) lcd.print(F("Blue Score: "));
  else if(mode==2 || mode == 3 || mode == 4 || mode == 5) lcd.print(F("Blue Boxes: "));
  lcd.setCursor(11,2);
  lcd.print(score_B,10);
  digitalWrite(BUZZER,HIGH);
  screen=END;
  //points_A=0;
  //points_B=0;
}

void red_score_update(){
  lcd.setCursor(11,1);
  lcd.print(score_A,10);
}

void blue_score_update(){
  lcd.setCursor(11,2);
  lcd.print(score_B,10);
}

void trx_test(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("  TRANSMISSION TEST "));
  lcd.setCursor(0,3);
  lcd.print(F("START=Send  RESET=OK"));
  screen=TRX;
}

void keyPadActions(){
  if (keypad.getKeys()){
    for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
    {
      if ( keypad.key[i].stateChanged )   // Only find keys that have changed state.
      {
        switch (keypad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
        case PRESSED:
          {
            if(keypad.key[i].kchar >= '0' && keypad.key[i].kchar <= '9'){
              if(GAME == false){
                Serial.println(keypad.key[i].kchar);
                lcd.setCursor(blinkCol,blinkRow);
                lcd.print(keypad.key[i].kchar);
                numHold[blinkCol-blinkColL] = keypad.key[i].kchar;
                Serial.println("numHold:"+numHold);
                if(blinkCol < blinkColR)blinkCol++;
                else lcd.setCursor(blinkCol,blinkRow);
                lcd.blink();
              }
            }
            else if(keypad.key[i].kchar == 'A'){
              if(GAME == false){
                if(blinkCol > blinkColL)blinkCol--;
                lcd.setCursor(blinkCol,blinkRow);
                lcd.blink();
              }
            }
            else if(keypad.key[i].kchar == 'B'){
              if(GAME == false){
                String numHoldT = numHold;
                numHoldT.trim();
                if(blinkCol-blinkColL < numHoldT.length()-1)blinkCol++;
                lcd.setCursor(blinkCol,blinkRow);
                lcd.blink();
              }
            }
            else if(keypad.key[i].kchar == 'C'){
              if(screen==NUM_BOX && GAMEON==false){
                numHold.trim();
                numbox = numHold.toInt();
                if(EEPROM.read(1)*256+EEPROM.read(2) != numbox){
                  EEPROM.write(1, floor(numbox/256));
                  EEPROM.write(2, numbox%256);
                }
                screen=ID;
                screen_select();
              }
              else if(screen==ID && GAMEON==false){
                numHold.trim();
                boxid = numHold.toInt();
                if(boxid > 0){
                  if(EEPROM.read(5)*256+EEPROM.read(6) != boxid){
                    EEPROM.write(5, floor(boxid/256));
                    EEPROM.write(6, boxid%256);
                  }
                  if(mode == 2 || mode == 4 || mode == 5) screen=WIN_BOX;
                  else screen=CAP_TIME;
                  screen_select();
                }
              }
              else if(screen==WIN_BOX && GAMEON==false){
                numHold.trim();
                winbox = numHold.toInt();
                if(EEPROM.read(3)*256+EEPROM.read(4) != winbox){
                  EEPROM.write(3, floor(winbox/256));
                  EEPROM.write(4, winbox%256);
                }
                screen=CAP_TIME;
                screen_select();
              }
              else if(screen==CAP_TIME && GAMEON==false){
                numHold.trim();
                captime = numHold.toInt();
                if(EEPROM.read(7)*256+EEPROM.read(8) != captime){
                  EEPROM.write(7, floor(captime/256));
                  EEPROM.write(8, captime%256);
                }
                capture_time = captime;
                if(mode == 3 || mode == 4) screen=INTERVAL;
                else screen=LIMIT;
                screen_select();
              }
              else if(screen==LIMIT && GAMEON==false){
                numHold.trim();
                limit = numHold.toInt();
                if(EEPROM.read(9)*256+EEPROM.read(10) != limit){
                  EEPROM.write(9, floor(limit/256));
                  EEPROM.write(10, limit%256);
                }
                screen=INTERVAL;
                screen_select();
              }
              else if(screen==INTERVAL && GAMEON==false){
                numHold.trim();
                timer = numHold.toInt();
                if(EEPROM.read(11) != timer) EEPROM.write(11, timer);
                if (mode == 4) screen=LNCH_TIMER;
                else screen=PENDING;
                screen_select();
              }
              else if(screen==LNCH_TIMER && GAMEON==false){
                numHold.trim();
                lnchtimer = numHold.toInt();
                if(EEPROM2BytesRead(12,13) != lnchtimer){
                  EEPROM2BytesWrite(12, 13, lnchtimer);
                }
                screen=PENDING;
                screen_select();
              }
              else if(screen==PENDING && GAMEON==false){
                screen=GAME;
                GAMEON=true;
                capture_time -= 2;
                starttime=millis();
                screen_select();
              }
            }
            break;
          }
        case HOLD:
          {
            if(keypad.key[i].kchar == '1'){
              Serial.println(keypad.key[i].kchar);
            }
            break;
          }
        case RELEASED:
          {
            if(keypad.key[i].kchar == '1'){
              Serial.println(keypad.key[i].kchar);
            }
            break;
          }
        case IDLE:
          break;
        }
      }
    }
  }
}

void onClickBtnRed(){
  if(screen==MODE && GAMEON==false && mode < sizeof(modeList)/sizeof(String)){
    mode++;
    screen_select();
  }

  if(screen==NUM_BOX && GAMEON == false){
    GAMEON=false;
    if(numbox < 4000) numbox++;
    update_num_boxes();
  }

  if(screen==WIN_BOX && GAMEON == false){
    GAMEON=false;
    if(winbox < numbox) winbox++;
    update_win_boxes();
  }

  if(screen==ID && GAMEON == false){
    GAMEON=false;
    if(boxid < numbox) boxid++;
    update_id_screen();
  }

  if(screen==CAP_TIME && GAMEON == false){
    GAMEON=false;
    if(captime<4000) captime++;
    else captime = 30;
    update_time_screen();
  }

  if(screen==LIMIT && GAMEON==false){
    if(limit < 65278) limit++;
    limit_set_update();
  }

  if(screen==INTERVAL && GAMEON==false){
    if(timer<captime)
      timer++;
    else
      timer = 1;
    set_interval_update();
  }

  if(screen==LNCH_TIMER && GAMEON == false){
    if(lnchtimer<65278) lnchtimer++;
    update_lnchtimer();
  }
}

void onClickBtnBlue(){
  if(screen==MODE && GAMEON==false && mode > 1){
    mode--;
    screen_select();
  }

  if(screen==NUM_BOX && GAMEON == false){
    GAMEON=false;
    if(numbox > 1) numbox--;
    update_num_boxes();
  }

  if(screen==WIN_BOX && GAMEON == false){
    GAMEON=false;
    if(winbox>1) winbox--;
    update_win_boxes();
  }

  if(screen==ID && GAMEON == false){
    GAMEON=false;
    if(boxid>0) boxid--;
    update_id_screen();
  }

  if(screen==CAP_TIME && GAMEON == false){
    GAMEON=false;
    if(captime>30) captime--;
    update_time_screen();
  }

  if(screen==LIMIT && GAMEON==false){
    if(limit>0) limit--;
    limit_set_update();
  }

  if(screen==INTERVAL && GAMEON==false){
    if(timer > 1) timer--;
    set_interval_update();
  }

  if(screen==LNCH_TIMER && GAMEON == false){
    if(lnchtimer>0) lnchtimer--;
    update_lnchtimer();
  }
}

void EEPROM2BytesWrite(uint16_t ad1, uint16_t ad2, uint16_t value){
  EEPROM.write(ad1, floor(value/256));
  EEPROM.write(ad2, value%256);
}

uint16_t EEPROM2BytesRead(uint16_t ad1, uint16_t ad2){
  return EEPROM.read(ad1)*256+EEPROM.read(ad2);
}

void onClickBtnStart(){
  if(screen==MODE && GAMEON==false){
    if(mode == sizeof(modeList)/sizeof(String)) screen = TRX;
    else{
      if(EEPROM.read(0)!=mode) EEPROM.write(0,mode);
      screen=NUM_BOX;
    }
    screen_select();
    return;
  }

  if(screen==TRX && GAMEON==false){
    lcd.setCursor(0,1);
    lcd.print(F("  Please wait.....  "));
    xbee.print("TR");
    return;
  }

  if(screen==NUM_BOX && GAMEON==false){
    numHold.trim();
    numbox = numHold.toInt();
    if(EEPROM.read(1)*256+EEPROM.read(2) != numbox){
      EEPROM.write(1, floor(numbox/256));
      EEPROM.write(2, numbox%256);
    }
    screen=ID;
    screen_select();
    return;
  }

  if(screen==ID && GAMEON==false){
    if(boxid > 0){
      if(EEPROM.read(5)*256+EEPROM.read(6) != boxid){
        EEPROM.write(5, floor(boxid/256));
        EEPROM.write(6, boxid%256);
      }
      if(mode == 2 || mode == 4 || mode == 5 || mode == 6) screen=WIN_BOX;
      else screen=CAP_TIME;
      screen_select();
    }
    return;
  }

  if(screen==WIN_BOX && GAMEON==false){
    if(EEPROM.read(3)*256+EEPROM.read(4) != winbox){
      EEPROM.write(3, floor(winbox/256));
      EEPROM.write(4, winbox%256);
    }
    screen=CAP_TIME;
    screen_select();
    return;
  }

  if(screen==CAP_TIME && GAMEON==false){
    if(EEPROM.read(7)*256+EEPROM.read(8) != captime){
      EEPROM.write(7, floor(captime/256));
      EEPROM.write(8, captime%256);
    }
    capture_time = captime;
    if(mode == 3 || mode == 4) screen=INTERVAL;
    else screen=LIMIT;
    screen_select();
    return;
  }

  if(screen==LIMIT && GAMEON==false){
    if(EEPROM.read(9)*256+EEPROM.read(10) != limit){
      EEPROM.write(9, floor(limit/256));
      EEPROM.write(10, limit%256);
    }
    screen=INTERVAL;
    screen_select();
    return;
  }

  if(screen==INTERVAL && GAMEON==false){
    if(EEPROM.read(11) != timer) EEPROM.write(11, timer);
    if (mode == 4) screen=LNCH_TIMER;
    else screen=PENDING;
    screen_select();
    return;
  }

  if(screen==LNCH_TIMER && GAMEON==false){
    if(EEPROM2BytesRead(12,13) != lnchtimer){
      EEPROM2BytesWrite(12, 13, lnchtimer);
    }
    screen=PENDING;
    screen_select();
    return;
  }

  if(screen==PENDING && GAMEON==false){
    screen=GAME;
    GAMEON=true;
    capture_time -= 2;
    starttime=millis();
    screen_select();
    return;
  }
}

void onClickBtnReset(){
  if(screen == TRX){
    mode = EEPROM.read(0);
    screen = MODE;
    screen_select();
  }
  else{
    numbox = EEPROM.read(1)*256+EEPROM.read(2);
    winbox = EEPROM.read(3)*256+EEPROM.read(4);
    boxid = EEPROM.read(5)*256+EEPROM.read(6);
    captime = EEPROM.read(7)*256+EEPROM.read(8);
    limit = EEPROM.read(9)*256+EEPROM.read(10);
    timer = EEPROM.read(11);
    lnchtimer = EEPROM.read(12)*256+EEPROM.read(13);
    digitalWrite(BUZZER,LOW);
    digitalWrite(BUZZER_BECON,LOW);
    GAMEON=false;
    A=false;
    B=false;
    digitalWrite(LED_A,HIGH);
    digitalWrite(LED_B,HIGH);
    capture_time=captime;
    //box_id = 0;
    if(screen == END && mode == 4) {
      uncapture = true;
      if(boxes_A > 0) boxes_A --;
      if(boxes_B > 0) boxes_B --;
      score_A = boxes_A;
      score_B = boxes_B;
      if(score_A == 0) deact_A = 0;
      if(score_B == 0) deact_B = 0;
    }
    else{
      boxes_A = 0;
      boxes_B = 0;
      score_A = 0;
      score_B = 0;
    }
    points_A = 0;
    points_B = 0;
    for(uint8_t i = 0; i < sizeof(xbeeStr); i++){
      xbeeStr[i] = 0x00;
    }

    screen = PENDING;
    screen_select();
  }
}

void onClickBtnCfg(){
  if(screen != GAME && screen != END && GAMEON == false){
    screen = MODE;
    screen_select();
  }
}

void onClickBtnBuzz(){
  xbee.print("BZ");
  digitalWrite(BUZZER_BECON,HIGH);
}

void duringLongPressBtnRed(){
  redReleased = false;
  if(mode == 6) A = true;
}

void duringLongPressBtnBlue(){
  blueReleased = false;
  if(mode == 6) B = true;
}



































