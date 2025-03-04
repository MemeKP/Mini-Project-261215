#include <LCD_I2C.h>
#include <algorithm>
#include <ESP32Servo.h>

#define LED 22
#define S2 4  
#define S3 2  
#define sensorOut 16  

/* ขอบเขตค่าขั้นต่ำที่ยอมรับว่าเป็นสีจริง ๆ */
#define COLOR_THRESHOLD 100  

/* ขอบเขตของสีจากการ Calibration */
int R_Min = 5,  R_Max = 380;  
int G_Min = 4,  G_Max = 420;  
int B_Min = 4,  B_Max = 350;  

volatile unsigned long pulseStart = 0;
volatile unsigned long pulseDuration = 0;
volatile bool pulseCaptured = false;
int Red, Green, Blue;
int redValue, greenValue, blueValue;

LCD_I2C lcd(0x27, 16, 2);
Servo wiperservo;
Servo sorterservo;
Servo gateservo;

int wiperpin = 19;
int sorterpin = 32;
int sorterpos = 90;
int wiperpos = 85;
int gatepin = 26;
int gate_open = 120;  // มุมที่ gate เปิด
int gate_close = 105;   // มุมที่ gate ปิด

int sorterPositions[3] = {10, 45, 90};  // มุมของ sorter สำหรับสีต่าง ๆ

void IRAM_ATTR pulseISR() {
    if (digitalRead(sensorOut) == LOW) {
        pulseStart = micros();
    } else {
        pulseDuration = micros() - pulseStart;
        pulseCaptured = true;
    }
}

void setup() {
    pinMode(LED, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    pinMode(sensorOut, INPUT);

    attachInterrupt(digitalPinToInterrupt(sensorOut), pulseISR, CHANGE);

    //servo connected
    wiperservo.attach(wiperpin);
    sorterservo.attach(sorterpin);
    gateservo.setPeriodHertz(50);
    gateservo.attach(gatepin); 

    lcd.begin();
    lcd.backlight();

    Serial.begin(115200);
    delay(1000);

    gateservo.write(gate_close); //start with closed gate
}
/*
redValue = map(constrain(Red, R_Min, R_Max), R_Min, R_Max, 255, 0);
greenValue = map(constrain(Green, G_Min, G_Max), G_Min, G_Max, 255, 0);
blueValue = map(constrain(Blue, B_Min, B_Max), B_Min, B_Max, 255, 0);

*/
void loop() {
    Red = getStableColor(getRed);
    //redValue = constrain(Red, R_Min, R_Max);
    redValue = map(constrain(Red, R_Min, R_Max), R_Min, R_Max, 255, 0);
    delay(50);
    
    Green = getStableColor(getGreen);
    //greenValue = constrain(Green, G_Min, G_Max);
    greenValue = map(constrain(Green, G_Min, G_Max), G_Min, G_Max, 255, 0);
    delay(50);
    
    Blue = getStableColor(getBlue);
    //blueValue = constrain(Blue, B_Min, B_Max);
    blueValue = map(constrain(Blue, B_Min, B_Max), B_Min, B_Max, 255, 0);
    delay(50);

    lcd.clear();
    delay(50);
    Serial.print("Red: "); Serial.print(redValue);
    Serial.print("  Green: "); Serial.print(greenValue);
    Serial.print("  Blue: "); Serial.println(blueValue);

    String detectedColor = "waiting..";
    int sorterTarget = sorterpos;  // ค่าเริ่มต้น คือ ไม่ขยับ

    // ตรวจสอบว่าสีที่อ่านได้มากพอหรือไม่
    if (redValue >= COLOR_THRESHOLD || greenValue >= COLOR_THRESHOLD || blueValue >= COLOR_THRESHOLD) {
        if (redValue > greenValue && redValue > blueValue) {
            detectedColor = "RED";
            sorterTarget = sorterPositions[0]; // สีแดงไปตำแหน่ง 0°
        } else if (greenValue > redValue && greenValue > blueValue) {
            detectedColor = "GREEN";
            sorterTarget = sorterPositions[1]; // สีเขียวไปตำแหน่ง 64°
        } else if (blueValue > redValue && blueValue > greenValue) {
            detectedColor = "BLUE";
            sorterTarget = sorterPositions[2]; // สีน้ำเงินไปตำแหน่ง 140°
        } else {
            detectedColor = "waiting..";  // ถ้าสีไม่ชัดเจน ให้รอ
        }
    } else {
        detectedColor = "waiting..";  // ถ้าค่าสีต่ำไป ให้รอ
    }

    lcd.print("     " + detectedColor);

    // ถ้าเจอสีที่แน่ชัด ค่อยขยับเซอร์โว
    if (detectedColor != "waiting..") {
        moveServos(sorterTarget);
    }
    delay(500);
}

/* ฟังก์ชันควบคุมการเคลื่อนที่ของเซอร์โว */
/*
void moveServos(int sorterTarget){
    // Wiper Servo Action - ปัดวัตถุ
    Serial.println("Moving wiper servo...");
    wiperservo.write(140);  // ปัดไปด้านหน้า
    delay(450);
    wiperservo.write(85);   // กลับตำแหน่งเดิม
    delay(450);

    // Sorter Servo Action - แยกวัตถุ
    Serial.println("Moving sorter servo...");
    sorterservo.write(sorterTarget);  // หมุนไปตำแหน่งแยกวัตถุ
    delay(500);
    sorterservo.write(sorterpos); // กลับตำแหน่งเริ่มต้น
    delay(500);
}
*/
void smoothServoMove(Servo &servo, int startPos, int endPos, int stepDelay = 5) {
    int step = (startPos < endPos) ? 1 : -1;
    for (int pos = startPos; pos != endPos; pos += step) {
        servo.write(pos);
        delay(stepDelay);
    }
    servo.write(endPos);
}

void moveServos(int sorterTarget){
  // ขยับ sorter servo
  smoothServoMove(sorterservo, sorterpos, sorterTarget, 5);
  delay(500);

  smoothServoMove(wiperservo, 85, 140, 5);
  delay(300);
  smoothServoMove(wiperservo, 140, 85, 5);
  delay(300);
  smoothServoMove(sorterservo, sorterTarget, sorterpos, 5);
  delay(300);
  
  // // ขยับ wiper servo
  // smoothServoMove(wiperservo, 85, 140, 5);
  // delay(300);
  // smoothServoMove(wiperservo, 140, 85, 5);
  // delay(300);

  // เปิด Gate ให้วัตถุหล่น
  Serial.println("Opening gate...");
  gateservo.write(gate_open);  
  delay(500);  // รอ

  // ปิด Gate กลับตำแหน่งเดิม
  Serial.println("Closing gate...");
  gateservo.write(gate_close);  
  delay(500);
}


/* ฟังก์ชันอ่านค่าของสีแบบเสถียร */
/*
int getStableColor(int (*getColor)()) {
    int readings[5];
    for (int i = 0; i < 5; i++) {
        readings[i] = getColor();
        delay(10);
    }
    std::sort(readings, readings + 5);
    return readings[2];  
}
*/
int getStableColor(int (*getColor)()) {
    const int numReadings = 7;  // เพิ่มจำนวนการอ่านให้มากขึ้น
    int readings[numReadings];

    for (int i = 0; i < numReadings; i++) {
        readings[i] = getColor();
        delay(5); // ลด delay เพื่อให้เร็วขึ้น
    }

    std::sort(readings, readings + numReadings);
    return readings[numReadings / 2];  // ใช้median
}


/* ฟังก์ชันอ่านค่าพัลส์ของสี */
int getRed() {
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    delay(10);
    return readPulse();  
}

int getGreen() {
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    delay(10);
    return readPulse();
}

int getBlue() {
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    delay(10);
    return readPulse();
}

/* อ่านค่าพัลส์จากเซ็นเซอร์ */
int readPulse() {
    pulseCaptured = false;
    while (!pulseCaptured);
    Serial.println(pulseDuration); // ตรวจสอบค่าที่อ่านได้ ถ้าติดลบอาจมีปัญหาเรื่องอินพุตของ sensorOut
    return pulseDuration;
}
