#include <ESP32Servo.h>

#define GATE_SERVO_PIN 26

Servo gateservo;

void setup() {
    Serial.begin(115200);
    gateservo.attach(GATE_SERVO_PIN, 500, 2400); // ตั้งค่าช่วงพัลส์ PWM
}

void loop() {
    Serial.println("Moving to 0°...");
    gateservo.write(90);  // เปิดประตู
    delay(1000);

    Serial.println("Moving to 15°...");
    gateservo.write(105);  // ปิดประตู
    delay(1000);
}
