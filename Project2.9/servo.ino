#include <ESP32Servo.h>

Servo wiperservo;
Servo sorterservo;

int wiperpin = 19;
int sorterpin = 5;

// ตำแหน่งของเซอร์โว
int sorterpos = 90; // ค่าเริ่มต้น
int wiperpos = 85;

void setup() {
  Serial.begin(9600);
  
  // Attach เซอร์โว
  wiperservo.attach(wiperpin);
  sorterservo.attach(sorterpin);

  Serial.println("Testing Servo Motors...");
}

void loop() {
  testServos();
  delay(2000);  // หน่วงเวลาก่อนทดสอบใหม่
}

void testServos() {
  Serial.println("Moving sorter servo...");
  sorterservo.write(sorterpos);  
  delay(500);

  Serial.println("Moving wiper servo...");
  wiperservo.write(140); 
  delay(450);    
  wiperservo.write(85);  
  delay(450);  

  Serial.println("Servo test complete!");
}
