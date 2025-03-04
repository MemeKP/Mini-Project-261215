#include <ESP32Servo.h>
#include <LCD_I2C.h>
//#include <Wifi.h>

#define LED 22
#define S2 4  
#define S3 2  
#define sensorOut 16  
#define WIPER_SERVO_PIN 19
#define SORTER_SERVO_PIN 32
#define GATE_SERVO_PIN 26

/* ขอบเขตค่าขั้นต่ำที่ยอมรับว่าเป็นสีจริง ๆ */
#define COLOR_THRESHOLD 70  

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

int sorterpos = 90;
int wiperpos = 85;
int gate_open = 25;  
int gate_close = 0;   

//Connect MQTT
//const int mqttButtonPin = 25;

// WiFi and MQTT Config
/*
const char ssid[] = "@JumboPlusIoT";   // Your WiFi SSID
const char pass[] = "thw02dbk";        // Your WiFi Password
const char mqtt_broker[] = "test.mosquitto.org"; // MQTT broker
const char mqtt_topic[] = "group2.21/command";
const char mqtt_client_id[] = "mini_project_group_2.21";*/

int sorterPositions[3] = {10, 45, 90};  

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

    Serial.begin(115200);

    wiperservo.attach(WIPER_SERVO_PIN, 500, 2400);
    sorterservo.attach(SORTER_SERVO_PIN, 500, 2400);
    gateservo.attach(GATE_SERVO_PIN, 500, 2400);

    moveServo(gateservo, gate_close);

    lcd.begin();
    lcd.backlight();

    delay(1000);
}

void smoothServoMove(Servo &servo, int startPos, int endPos, int stepDelay = 3) {
    int step = (startPos < endPos) ? 1 : -1;
    for (int pos = startPos; pos != endPos; pos += step) {
        servo.write(pos);
        delay(stepDelay);
    }
    servo.write(endPos);
}

void loop() {
    Red = getStableColor(getRed);
    redValue = map(constrain(Red, R_Min, R_Max), R_Min, R_Max, 255, 0);
    delay(50);

    Green = getStableColor(getGreen);
    greenValue = map(constrain(Green, G_Min, G_Max), G_Min, G_Max, 255, 0);
    delay(50);

    Blue = getStableColor(getBlue);
    blueValue = map(constrain(Blue, B_Min, B_Max), B_Min, B_Max, 255, 0);
    delay(50);

    lcd.clear();
    delay(50);
    Serial.print("Red: "); Serial.print(redValue);
    Serial.print("  Green: "); Serial.print(greenValue);
    Serial.print("  Blue: "); Serial.println(blueValue);

    String detectedColor = "waiting..";
    int sorterTarget = sorterpos;

    if (redValue >= COLOR_THRESHOLD || greenValue >= COLOR_THRESHOLD || blueValue >= COLOR_THRESHOLD) {
        if (redValue > greenValue && redValue > blueValue) {
            detectedColor = "RED";
            sorterTarget = sorterPositions[0];
        } else if (greenValue > redValue && greenValue > blueValue) {
            detectedColor = "GREEN";
            sorterTarget = sorterPositions[1];
        } else if (blueValue > redValue && blueValue > greenValue) {
            detectedColor = "BLUE";
            sorterTarget = sorterPositions[2];
        }
    }

    lcd.print("     " + detectedColor);

    if (detectedColor != "waiting..") {
        Serial.println("Moving sorter servo...");
        smoothServoMove(sorterservo, sorterpos, sorterTarget);  // ใช้ smoothServoMove แทน moveServo
        sorterpos = sorterTarget;  // อัปเดตตำแหน่งของเซอร์โว

        delay(500);

        Serial.println("Moving wiper servo...");
        smoothServoMove(wiperservo, wiperpos, 140);
        delay(450);
        smoothServoMove(wiperservo, 140, wiperpos);
        delay(450);

        Serial.println("Moving sorter servo back...");
        smoothServoMove(sorterservo, sorterTarget, 90);
        delay(500);

        Serial.println("Opening gate...");
        moveServo(gateservo, gate_open);
        delay(200);

        Serial.println("Closing gate...");
        moveServo(gateservo, gate_close);
        delay(200);

/*
        Serial.println("Opening gate...");
        smoothServoMove(gateservo, gate_close, gate_open);  // ใช้ smoothServoMove แทน moveServo
        delay(500);

        Serial.println("Closing gate...");
        smoothServoMove(gateservo, gate_open, gate_close);  // ใช้ smoothServoMove แทน moveServo
        delay(500);*/
    }
    delay(500);
}

/*
void loop() {
    Red = getStableColor(getRed);
    redValue = map(constrain(Red, R_Min, R_Max), R_Min, R_Max, 255, 0);
    delay(50);
    
    Green = getStableColor(getGreen);
    greenValue = map(constrain(Green, G_Min, G_Max), G_Min, G_Max, 255, 0);
    delay(50);
    
    Blue = getStableColor(getBlue);
    blueValue = map(constrain(Blue, B_Min, B_Max), B_Min, B_Max, 255, 0);
    delay(50);

    lcd.clear();
    delay(50);
    Serial.print("Red: "); Serial.print(redValue);
    Serial.print("  Green: "); Serial.print(greenValue);
    Serial.print("  Blue: "); Serial.println(blueValue);

    String detectedColor = "waiting..";
    int sorterTarget = sorterpos;

    if (redValue >= COLOR_THRESHOLD || greenValue >= COLOR_THRESHOLD || blueValue >= COLOR_THRESHOLD) {
        if (redValue > greenValue && redValue > blueValue) {
            detectedColor = "RED";
            sorterTarget = sorterPositions[0]; 
        } else if (greenValue > redValue && greenValue > blueValue) {
            detectedColor = "GREEN";
            sorterTarget = sorterPositions[1]; 
        } else if (blueValue > redValue && blueValue > greenValue) {
            detectedColor = "BLUE";
            sorterTarget = sorterPositions[2]; 
        }
    }

    lcd.print("     " + detectedColor);

    if (detectedColor != "waiting..") {
        Serial.println("Moving sorter servo...");
        moveServo(sorterservo, sorterTarget);
        delay(500);

        Serial.println("Moving wiper servo...");
        moveServo(wiperservo, 140);
        delay(450);
        moveServo(wiperservo, 85);
        delay(450);

        moveServo(sorterservo, 90);
        delay(500);

        Serial.println("Opening gate...");
        moveServo(gateservo, gate_open);
        delay(500);

        Serial.println("Closing gate...");
        moveServo(gateservo, gate_close);
        delay(500);
    }
    delay(500);
}*/

void moveServo(Servo &servo, int angle) {
    servo.write(angle);
}

int getStableColor(int (*getColor)()) {
    const int numReadings = 8;
    int readings[numReadings];

    for (int i = 0; i < numReadings; i++) {
        readings[i] = getColor();
        delay(5);
    }

    std::sort(readings, readings + numReadings);
    return readings[numReadings / 2];
}

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

int readPulse() {
    pulseCaptured = false;
    while (!pulseCaptured);
    return pulseDuration;
}
