/* Include the standard Arduino SPI library */
#include <SPI.h>
/* Include the RFID library */
#include <RFID.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(12,11,5,4,3,2);
const int turnMotor1 = 12;
const int turnMotor2 = 11;
const int upDownMotor1 = 10;
const int upDownMotor2 = 9;
const int SDA1 = 8;
const int RESET = 7;
const int groundFloor = 6;
const int firstFloor = 5;
const int secondFloor = 4;
const int trigger = 3;
const int pot = A0;
int turnMotorSpeed=100;
int occupied[] = {-1,-1,-1,-1,-1,-1};
int rfidValues[] = {1506,1726,1510,-1,-1,-1};
int potValue;
int angleDifference;
int initialAngleDifference;
int floorNumber;
int parkingFloor;
int parkingSpot;
int openSpot;
int RFIDValue;
char status[] = "Welcome to RoboPark";
RFID RC522(SDA1, RESET); 
void turn(int angle){
  potValue = map(analogRead(pot),0,1023,0,270);
  if (potValue < angle){
    while (potValue < angle){
      analogWrite(turnMotor1,turnMotorSpeed);
      potValue = map(analogRead(pot),0,1023,0,270);
    }
    digitalWrite(turnMotor1,LOW);
  }
  else{
    while (potValue > angle){
      analogWrite(turnMotor2,turnMotorSpeed);
      potValue = map(analogRead(pot),0,1023,0,270);
    }
    digitalWrite(turnMotor2,LOW);
  }
}
void ground(){
  while(digitalRead(groundFloor)==0){
    digitalWrite(upDownMotor1,HIGH);
    Serial.println(digitalRead(groundFloor));
    
  }
  delay(1500);
  digitalWrite(upDownMotor1,LOW);
  floorNumber=0;
}

void first(){
  if (floorNumber==0){
    while(digitalRead(firstFloor)==0){
      digitalWrite(upDownMotor2,HIGH);

    }
    delay(1500);
    digitalWrite(upDownMotor2,LOW);
  }
  else if (floorNumber==2){
    while(digitalRead(firstFloor)==0){
      digitalWrite(upDownMotor1,HIGH);
    }
    delay(1500);
    digitalWrite(upDownMotor1,LOW);
  }
  floorNumber=1;
}
void second(){
  while(digitalRead(secondFloor)==0){
    digitalWrite(upDownMotor2,HIGH);
    
  }
  delay(1500);
  digitalWrite(upDownMotor2,LOW);
  floorNumber=2;
}
void goHome(){
  turn(0);
  Serial.println("turned");
  ground();
}
void survey(){
  goHome();
  first();
  delay(1000);
  turn(90);
  delay(1000);
  turn(180);
  delay(1000);
  turn(270);
  second();
  delay(1000);
  turn(180);
  delay(1000);
  turn(90);
  delay(1000);
  turn(0);
  delay(1000);
  goHome();
}
int findOpenSpot(){
  for (int i=0;i<6;i++){
    if (occupied[i]<0){
      return i;
    }
  }
  return -1;
}
void retrieve(int f,int s){
  if (f==1){
    first();
  }
  else{
    second();
  }
  digitalWrite(upDownMotor1,HIGH);
  delay(4000);
  digitalWrite(upDownMotor1,LOW);
  floorNumber-=1;
  turn(s);
  if (f==1){
    first();
  }
  else{
    second();
  }
  digitalWrite(upDownMotor2,HIGH);
  delay(4000);
  digitalWrite(upDownMotor2,LOW);
  goHome();
}
void park(int f,int s){
  if (f==1){
    first();
  }
  else{
    second();
  }
  turn(s);
  digitalWrite(upDownMotor1,HIGH);
  delay(4000);
  digitalWrite(upDownMotor1,LOW);
  goHome();
}
int findRFID(int r){
  for (int i=0;i<6;i++){
    if (rfidValues[i]==r){
      return i;
    }
  }
  return -1;
}
void lcdUpdate(){
  lcd.setCursor(0,1);
  lcd.print("Level 1: ");
  lcd.setCursor(9,1);
  lcd.print("1:");
  lcd.print(occupied[0]);
  lcd.print(" 2:");
  lcd.print(occupied[1]);
  lcd.print(" 3:");
  lcd.print(occupied[2]);
  
  lcd.setCursor(0,2);
  lcd.print("Level 2: ");
  lcd.setCursor(9,2);
  lcd.print("1:");
  lcd.print(occupied[3]);
  lcd.print(" 2:");
  lcd.print(occupied[4]);
  lcd.print(" 3:");
  lcd.print(occupied[5]);
  lcd.setCursor(0,2);
  
  lcd.setCursor(0,3);
  lcd.print("Status: ");
  lcd.print(status);
}
void setup() {
  // put your setup code here, to run once
  digitalWrite(13,LOW);
  digitalWrite(12,LOW);
  Serial.begin(9600);
  lcd.begin(20,4);
  lcd.setCursor(1,0);
  lcd.print("Parking Garage");
  lcd.setCursor(2,0);
  lcd.print("Setup...");
  pinMode(turnMotor1,OUTPUT);
  pinMode(turnMotor2,OUTPUT);
  pinMode(upDownMotor1,OUTPUT);
  pinMode(upDownMotor2,OUTPUT);
  pinMode(groundFloor,INPUT);
  pinMode(firstFloor,INPUT);
  pinMode(secondFloor,INPUT);
  pinMode(trigger,INPUT);
  pinMode(pot,INPUT);
  digitalWrite(upDownMotor1,LOW);
  digitalWrite(upDownMotor2,LOW);
  digitalWrite(turnMotor1,LOW);
  digitalWrite(turnMotor2,LOW);
  SPI.begin(); 
  /* Initialise the RFID reader */
  RC522.init();
  goHome();
  survey();
}

void loop() {
  // put your main code here, to run repeatedly:
  status = "Setup Done";
  lcdUpdate();
  if (RC522.isCard() || digitalRead(trigger)){
    if (digitalRead(trigger)){
      status = "Please wait...";
      lcdUpdate();
      openSpot = findOpenSpot();
      if (openSpot==-1){
        status="NO OPEN PARKING";
        lcdUpdate();
      }
      else{
        if (openSpot<3){
          parkingFloor = 1;
        }
        else{
          parkingFloor = 2;
        }
        parkingSpot = (openSpot%3+1)*90;
        retrieve(parkingFloor,parkingSpot);
        status = "Take Card/Drive Up";
        lcdUpdate();
        while (!RC522.isCard())
        {
          
        }
        RC522.readCardSerial();
        RFIDValue=0;
        for(int i=0;i<5;i++)
          RFIDValue+=100*i+RC522.serNum[i];
        {
        occupied[openSpot]=1;
        
        lcdUpdate();
        rfidValues[openSpot]=RFIDValue;
        park(parkingFloor,parkingSpot);
        }
    }
    }
    else{
      RC522.readCardSerial();
      RFIDValue=0;
      for(int i=0;i<5;i++)
        RFIDValue+=100*i+RC522.serNum[i];
      {
      
      //Serial.print(RC522.serNum[i],HEX); //to print card detail in Hexa Decimal format
      }
      Serial.println(RFIDValue);
      int openSpot = findRFID(RFIDValue);
      if (openSpot==-1){
        status="ERROR: NO RFID REGISTERED";
        lcdUpdate();
      }
      else if (occupied[openSpot]<1){
        status="ERROR: NO CAR IS PARKED HERE";
        lcdUpdate();
      }
      else{
        if (openSpot<3){
          parkingFloor = 1;
        }
        else{
          parkingFloor = 2;
        }
        parkingSpot = (openSpot%3+1)*90;
        retrieve(parkingFloor,parkingSpot);
        occupied[openSpot]=-1;
        lcdUpdate();
    }
    }
    
  }
}
