

//LCD display
//SDA --> A4
//SCL --> A5
#include <Wire.h>                 
#include <LiquidCrystal_I2C.h>  
LiquidCrystal_I2C lcd(0x27,16,2);

#include <EEPROM.h>
#define EEPROM_SIZE 3

// rotary encoder
#define button 9  //SW
#define outputA 8 //CLK
#define outputB 7  //DT

//weight HX711
#include <Arduino.h>
#include "HX711.h"
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
HX711 scale;
float weight = 0; float average;

//servos
#include <Servo.h>
Servo wiperservo;
Servo sorterservo;
int wiperpos, sorterpos;

// motor/servo pins
int shakerpin = 11;
int beltpin = 12;
int wiperpin = 4;
int sorterpin = 5;
int LEDpin = 6;
int LDRpin = A0;
int LEDtrigger;

int counter = 0;
int currentStateA;
int lastStateA;
String currentDir ="";
unsigned long lastButtonPress = 0;
unsigned long lastRotation = 0;
int buttonstate;
int minimum0 = 0, maximum0 = 1000;
int flag1 = 0;
int min_options = 0;
int max_options = 2;  //   "0" and "1"  -->2
float min1, max1;
float departments;
float steps;
int sensorValue;
int sensorFlag = 0;
int container[7] = {0, 45, 64, 83, 102, 121, 140}; //array for storing sorter positions



void setup() {

  Serial.begin(9600);
  
  // Set encoder pins as inputs
  pinMode(button,INPUT_PULLUP);
  pinMode(outputA,INPUT);
  pinMode(outputB,INPUT);
  // motor/servo init
  pinMode(LEDpin, OUTPUT);
  pinMode(shakerpin, OUTPUT);   //shaker
  pinMode(beltpin, OUTPUT);     //conveyor belt
  pinMode(wiperpin, OUTPUT);    //wiper servo
  pinMode(sorterpin, OUTPUT);   //sorter servo
  pinMode(LDRpin, INPUT);       //LDR
  delay(10);

  wiperservo.attach(wiperpin);  delay(10);
  wiperservo.write(85); delay(500);
  wiperservo.detach(); delay(10);

  // init LED and LDR
  digitalWrite(LEDpin, HIGH);
  delay(10);
  LEDtrigger = analogRead(LDRpin)*0.8 ;     // minus 20%
  Serial.print("Trigger = ");Serial.println(LEDtrigger);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  delay(10);
    
  // scale init
  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  //scale.set_scale(1200);   //calibration factor
  scale.set_scale(1150);
  delay(10);
  Serial.println("Remove weight!");
  lcd.setCursor(0,0); lcd.print("Remove weight!");
  delay(3000);
  scale.tare();               // reset the scale to 0
  delay(100);
  Serial.println("Starting now"); Serial.println();

  
  Serial.println("started");
  Serial.println("Rotate the knob");


  // Read the initial state of outputA
  lastStateA = digitalRead(outputA);

  lcd.setCursor(0,0); lcd.print("Sorting Machine");    // start text
  lcd.setCursor(2,1); lcd.print("*** menu ***");
  delay(2000);
 
// MENU 1 =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  = 
  lcd.clear(); 
  lcd.setCursor(2,0); lcd.print("continue");    // menu text
  lcd.setCursor(2,1); lcd.print("new settings");
  while (flag1 == 0){
    readbutton();
    encoder();
    if (counter == 0) {
      lcd.setCursor(0,0); lcd.print(">"); lcd.setCursor(0,1); lcd.print(" "); // menu cursor
    }
    if (counter == 1) {
      lcd.setCursor(0,0); lcd.print(" "); lcd.setCursor(0,1); lcd.print(">"); // menu cursor
    }
  delay(5);
  if ((counter == 0) && (flag1 == 1)) {
     lcd.clear(); 
     lcd.setCursor(0,0); lcd.print("start sorting:");
     min1  = EEPROM.read(0);
     max1  = EEPROM.read(1);
     departments  = EEPROM.read(2);
     lcd.setCursor(2,1); lcd.print(min1,0);lcd.print(" - ");lcd.print(max1,0);lcd.print("   / ");lcd.print(departments,0);
     delay(3000);
     //loop();
     break;
  }
  if ((counter == 1) && (flag1 == 1))  {    // goto 2nd menu     
      menu2();
      EEPROM.write(0, min1);
      EEPROM.write(1, max1);
      EEPROM.write(2, departments);
      //loop();
      lcd.clear(); 
      lcd.setCursor(0,0); lcd.print("start sorting");
      lcd.setCursor(0,1); lcd.print("with new values");
      delay(3000);
      lcd.clear();
    //servos
    wiperservo.attach(wiperpin);
    sorterservo.attach(sorterpin);
    delay(10);
      break;
  }
 }
 flag1 = 0;
}    // =  =  =  =  =  =  =  end of setup  =  =  =  =  =  =



void loop() {   // =  =  =  =  =  =  =  loop  =  =  =  =  =  =  =  =  =  =  =  =  =

  // sorting process
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("working ...");
  delay(1);
  sorting();
}


void menu2() {    // MENU 2 =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  = 
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("from(g):");
  flag1 = 0;
  min_options = 0;
  max_options = 49;  // from ...  up to 49 g
  counter = 0;
  while (flag1 == 0) {
    readbutton();
    encoder();
    lcd.setCursor(9,0); lcd.print(counter);  //min1
  }
  if (flag1 == 1) {
    min1 = counter;
    flag1 = 0;
    lcd.setCursor(12,0); lcd.print("-");
  }
  // - - - - - - - - - - - - - - - - - - - - - - - -
  min_options = min1 + 1; // from min1 to...
  max_options = 50;       // ... up to 50 g
  counter = min_options;
  while (flag1 == 0) {
    readbutton();
    encoder();
    lcd.setCursor(14,0); lcd.print(counter);  //max1
  }
  if (flag1 == 1) {
    max1 = counter;
    flag1 = 0;
    delay(1000);
    lcd.clear();    // clear display for next step
  }
  flag1 = 0;
  // - - - - - - - - - - - - - - - - - - - - - - - -
  min_options = 2;    // from 2 to...
  max_options = 7;    // ... up to 6 departments
  counter = 2;
  lcd.setCursor(0,0); lcd.print("departments(2-6) ");
  while (flag1 == 0) {
    readbutton();
    encoder();
    lcd.setCursor(13,1); lcd.print(counter);  //departments
  }
  if (flag1 == 1) {
    departments = counter;
    flag1 = 0;
    delay(1000);
    lcd.clear();    // clear display for next step
  }
  // - - - - - - - - - - - - - - - - - - - - - - - -
  counter = 0; flag1 = 0;
  Serial.print(min1);Serial.print(" - ");Serial.print(max1);Serial.print("  in ");Serial.print(departments);Serial.println(" departments");
  steps = (max1 - min1) / departments;
  Serial.print("step: ");Serial.println(steps,1);
  for (int i = 1; i <= departments; i++){
    Serial.print(i); Serial.print(". department from ");Serial.print(min1 + (steps*(i-1)));Serial.print(" to ");Serial.println(min1 + steps*i);
  }
  delay(500);
}


void readbutton() {  // =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =
  buttonstate = digitalRead(button);
    if (buttonstate == LOW) {
          if (millis() - lastButtonPress > 50) {
             flag1 = 1;
             Serial.print("Button pressed      flag1 = ");Serial.println(flag1);
          }
    lastButtonPress = millis();     // Remember last button press event 
    }
   delay(10);
}

void encoder(){ // =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =
  int currentStateA = digitalRead(outputA); //clk
  int currentStateB = digitalRead(outputB);  //dt
  
  if(lastStateA == HIGH && currentStateA == LOW){
    if(currentStateB == LOW){
      counter--;
    }else{
      counter++;
    }
    if(counter < min_options) counter = max_options - 1;
    if(counter > max_options-1) counter = min_options;
    
  }
  //Serial.println(counter);
  delay(5);
  lastStateA = currentStateA;
}

void sorting(){ // =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =  =
  //LED + motors
  digitalWrite(shakerpin, HIGH);
  digitalWrite(beltpin, HIGH);

  sensorValue = analogRead(LDRpin); delay(2);
  //Serial.print("                  LDR:" ); Serial.println(sensorValue);
  if (sensorValue < LEDtrigger){
   sensorFlag = 1;
  }
  if ((sensorValue > LEDtrigger) && (sensorFlag ==1)) { // all motors stop when object falls from belt 
    digitalWrite(shakerpin, LOW);
    digitalWrite(beltpin, LOW); delay(1);
    sensorFlag = 0;  //reset sensorFlag
    
    //weight = scale.get_units(5),1;
    weight = scale.get_units(10), 5;
    Serial.print(weight);Serial.println(" g");
    delay(1);

  //assign weight to department
  for (int i = 1; i <= departments; i++){
    if ((weight >= min1 + (steps*(i-1))) && (weight < (min1 + steps*i))) { 
      sorterpos = container[i];
      Serial.print("container "); Serial.println(i);
      moveservos();
      break;
    }
    else if ((weight > 0) && (weight < min1)) {
      sorterpos = (container[1]);
      Serial.println("light: container 1 ");
      moveservos();
      break;
    }
    else if (weight > max1) {
      int d = departments;
      sorterpos = (container[d]);   //too heavy objects into the last container
      Serial.print("heavy: container "); Serial.println(d);
      moveservos();
      break;
    }
   }
   //restart motors
   digitalWrite(shakerpin, HIGH);
   digitalWrite(beltpin, HIGH);
   delay(10);
  }
}


void moveservos(){
      //move sorter
      sorterservo.attach(sorterpin);  delay(10);
      sorterservo.write(sorterpos);  delay(350);   //give the servo enough time to finish the move
      
      //move wiper
      wiperservo.attach(wiperpin);  delay(10);
      wiperservo.write(140); delay(450);    
      wiperservo.write(85);  delay(450);  // give the servo enough time to finish the move

      sorterservo.detach(); delay(10);  //deactivate servos to avoid shaking
      wiperservo.detach(); delay(10);
}
