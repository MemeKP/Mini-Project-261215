// กำหนดพิน
#define IN1 22        // ขา Input1 ของ L293D
#define IN2 19        // ขา Input2 ของ L293D
#define ENABLE1 23    // ขา Enable1 ของ L293D
#define LED_CW 18     // LED สำหรับแสดงหมุนตามเข็ม
#define LED_CCW 5     // LED สำหรับแสดงหมุนทวนเข็ม

unsigned long lastActionTime = 0; // เก็บเวลาสำหรับเปลี่ยนแอ็คชัน
const int rotationInterval = 2000;  // เวลา 2 วินาทีสำหรับหมุนแต่ละทิศทาง
const int pauseInterval = 500;      // เวลา 0.5 วินาทีสำหรับหยุดพัก
bool clockwise = true;              // ทิศทางเริ่มต้น: ตามเข็ม
bool isPaused = false;              // สถานะหยุดพัก
bool isRunning = false;             // สถานะการทำงานของมอเตอร์

void setup() {
  // ตั้งค่าพิน
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENABLE1, OUTPUT);
  pinMode(LED_CW, OUTPUT);
  pinMode(LED_CCW, OUTPUT);

  // เปิดใช้งาน L293D
  digitalWrite(ENABLE1, HIGH);

  // เริ่มการสื่อสารผ่าน Serial
  Serial.begin(9600);
  Serial.println("Press 't' to toggle motor state (on/off).");
}

void loop() {
  // ตรวจสอบคำสั่งจาก Serial
  if (Serial.available() > 0) {
    char command = Serial.read(); // อ่านคำสั่งจาก Serial

    if (command == 't') { // 't' สำหรับสลับสถานะการทำงาน
      isRunning = !isRunning; // เปิดหรือปิดการทำงาน
      if (isRunning) {
        Serial.println("Motor started.");
      } else {
        Serial.println("Motor stopped.");
      }
    }
  }

  // หากมอเตอร์หยุด ให้ปิดทุกพิน
  if (!isRunning) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(LED_CW, LOW);
    digitalWrite(LED_CCW, LOW);
    return;
  }

  // ทำงานเมื่อมอเตอร์กำลังทำงาน
  unsigned long currentTime = millis();

  if (isPaused) {
    // หยุดมอเตอร์ชั่วคราว
    if (currentTime - lastActionTime >= pauseInterval) {
      isPaused = false;           // สิ้นสุดช่วงหยุดพัก
      clockwise = !clockwise;     // สลับทิศทาง
      lastActionTime = currentTime; // อัปเดตเวลา
    }
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(LED_CW, LOW);
    digitalWrite(LED_CCW, LOW);
  } else {
    // หมุนมอเตอร์
    if (currentTime - lastActionTime >= rotationInterval) {
      isPaused = true;            // เริ่มช่วงหยุดพัก
      lastActionTime = currentTime; // อัปเดตเวลา
    } else {
      if (clockwise) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(LED_CW, HIGH);  // เปิด LED หมุนตามเข็ม
        digitalWrite(LED_CCW, LOW); // ปิด LED หมุนทวนเข็ม
      } else {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(LED_CW, LOW);  // ปิด LED หมุนตามเข็ม
        digitalWrite(LED_CCW, HIGH); // เปิด LED หมุนทวนเข็ม
      }
    }
  }
}
