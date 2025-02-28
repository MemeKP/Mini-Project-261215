#include <LCD_I2C.h>
#include <algorithm>
#include <ESP32Servo.h>

#define MOTOR  18
#define LED    22
#define S2 4  
#define S3 2  
#define sensorOut 16  
/* Enter the Minimum and Maximum Values obtained from Calibration */
int R_Min = 5,  R_Max = 38;  /* Red */
int G_Min = 4,  G_Max = 42;  /* Green */
int B_Min = 4,  B_Max = 35;  /* Blue */

volatile unsigned long pulseStart = 0;
volatile unsigned long pulseDuration = 0;
volatile bool pulseCaptured = false;
int Red, Green, Blue;
int redValue, greenValue, blueValue;

LCD_I2C lcd(0x27, 16, 2);
Servo wiperservo;
Servo sorterservo;

int wiperpin = 19;
int sorterpin = 32;
int sorterpos = 90;
int wiperpos = 85;

int beltpin = 18;
//int shakerpin = ...?

//In sorting
// int colorDetected;
// int container[3] = {0, 64, 140};

void IRAM_ATTR pulseISR() {
    if (digitalRead(sensorOut) == LOW) {
        pulseStart = micros();  // บันทึกเวลาเริ่มต้น
    } else {
        pulseDuration = micros() - pulseStart;  // คำนวณระยะเวลาพัลส์
        pulseCaptured = true;  // แจ้งว่าอ่านค่าเสร็จแล้ว
    }
}

void setup() {
    pinMode(MOTOR, OUTPUT);
    pinMode(LED, OUTPUT);
    
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    pinMode(sensorOut, INPUT);

    attachInterrupt(digitalPinToInterrupt(sensorOut), pulseISR, CHANGE); // ใช้ Interrupt

    wiperservo.attach(wiperpin);
    sorterservo.attach(sorterpin);

    wiperservo.detach();
    sorterservo.detach();

    lcd.begin();
    lcd.backlight();

    Serial.begin(115200);
    delay(1000);
}

void loop() {

  /*
  //Sorting process
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Start working...");
    sorting();
  */

  
   // Read color values with averaging for stability
  Red = getStableColor(getRed);
  redValue = map(Red, R_Min, R_Max, 255, 0);
  delay(200);
  
  Green = getStableColor(getGreen);
  greenValue = map(Green, G_Min, G_Max, 255, 0);
  delay(200);
  
  Blue = getStableColor(getBlue);
  blueValue = map(Blue, B_Min, B_Max, 255, 0);
  delay(200);

  lcd.clear();
  delay(50);
  // Print sensor values to Serial Monitor
  Serial.print("Red: "); Serial.print(redValue);
  Serial.print("  Green: "); Serial.print(greenValue);
  Serial.print("  Blue: "); Serial.println(blueValue);

  String detectedColor = "UNKNOWN";

  // Improved color detection logic
  if (redValue > greenValue && redValue > blueValue) {
    detectedColor = "RED";
  } else if (greenValue > redValue && greenValue > blueValue) {
    detectedColor = "GREEN";
  } else if (blueValue > redValue && blueValue > greenValue) {
    detectedColor = "BLUE";
  }
  lcd.print("     " + detectedColor);

  delay(1000);

  activateConveyor();
  
}

void activateConveyor() {
    Serial.println("Conveyor Activated");
    digitalWrite(MOTOR, HIGH);
    digitalWrite(LED, HIGH);
    delay(5000);
    digitalWrite(MOTOR, LOW);
    digitalWrite(LED, LOW);
    Serial.println("Conveyor Stopped");

    testServos();
    delay(1000);
}

void testServos() {
  // Attach เซอร์โวก่อนใช้งาน
  wiperservo.attach(wiperpin);
  sorterservo.attach(sorterpin);
  
  // 1. Wiper Servo Action
  Serial.println("Moving wiper servo...");
  wiperservo.write(140);
  delay(450);
  wiperservo.write(85);
  delay(450);

  // 2. Sorter Servo Action
  Serial.println("Moving sorter servo...");
  sorterservo.write(45);
  delay(500);
  sorterservo.write(sorterpos);
  delay(500);

  Serial.println("Servo test complete!");
  /*/
    sorterservo.attach(sorterpin);
    delay(10);
    sorterservo.write(sorterpos);
    delay(350);

    wiperservo.attach(wiperpin);
    delay(10);
    wiperservo.write(140);
    delay(450);
    wiperservo.write(85);
    delay(450);

    sorterservo.detach();
    delay(10);
    wiperservo.detach();
    delay(10);

    Serial.println("Resuming conveyor...");
    digitalWrite(MOTOR, HIGH);
    */
}

void sorting(){
   // เปิดมอเตอร์และเขย่าให้วัตถุเคลื่อนที่
  digitalWrite(shakerpin, HIGH);
  digitalWrite(beltpin, HIGH);

  sensorValue = analogRead(LDRpin); delay(2);

  if (sensorValue < LEDtrigger){
   sensorFlag = 1;
  }
  
  if ((sensorValue > LEDtrigger) && (sensorFlag == 1)) { 
    // หยุดมอเตอร์เมื่อวัตถุผ่าน
    digitalWrite(shakerpin, LOW);
    digitalWrite(beltpin, LOW); delay(1);
    sensorFlag = 0;  

    // อ่านค่าจากเซ็นเซอร์สี
    colorDetected = getStableColor(); 
    Serial.print("Detected color: "); Serial.println(colorDetected);
    delay(1);

    // คัดแยกตามสี
    switch (colorDetected) {
      case 1:  // สีแดง
        sorterpos = container[0];  
        Serial.println("Sorting Red object...");
        break;
      case 2:  // สีเขียว
        sorterpos = container[1];  
        Serial.println("Sorting Green object...");
        break;
      case 3:  // สีน้ำเงิน
        sorterpos = container[2];  
        Serial.println("Sorting Blue object...");
        break;
      default:
        Serial.println("Unknown color, sending to default container.");
        sorterpos = container[0]; // ส่งไปตำแหน่งเริ่มต้น
    }

    // เคลื่อนที่ sorter ตามตำแหน่งที่ได้
    testServos();

    // รีสตาร์ทมอเตอร์
    digitalWrite(shakerpin, HIGH);
    digitalWrite(beltpin, HIGH);
    delay(10);
  }
}

int getStableColor(int (*getColor)()) {
    int readings[5];
    for (int i = 0; i < 5; i++) {
        readings[i] = getColor();
        delay(10);
    }
    std::sort(readings, readings + 5);
    return readings[2];  
}

int getRed() {
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    delay(10);  // รอให้เซ็นเซอร์เปลี่ยนโหมด
    return readPulse();  
}

int getGreen() {
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    delay(10);  // รอให้เซ็นเซอร์เปลี่ยนโหมด
    return readPulse();
}

int getBlue() {
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    delay(10);  // รอให้เซ็นเซอร์เปลี่ยนโหมด
    return readPulse();
}

int readPulse() {
    pulseCaptured = false;
    while (!pulseCaptured);  // รอให้อ่านค่าเสร็จ
    return pulseDuration;
}
