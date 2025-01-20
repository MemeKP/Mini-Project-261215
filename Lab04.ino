#include <WiFi.h>
#include <MQTT.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// กำหนดพินสำหรับมอเตอร์
#define IN1 17        // ขา Input1 ของ L293D
#define IN2 16        // ขา Input2 ของ L293D
#define ENABLE1 19    // ขา Enable1 ของ L293D

unsigned long lastActionTime = 0;  // เก็บเวลาสำหรับเปลี่ยนแอ็คชัน
const int rotationInterval = 2000; // เวลา 2 วินาทีสำหรับหมุนแต่ละทิศทาง
const int pauseInterval = 500;     // เวลา 0.5 วินาทีสำหรับหยุดพัก
bool clockwise = true;             // ทิศทางเริ่มต้น: ตามเข็ม
bool isPaused = false;             // สถานะหยุดพัก
bool isRunning = false;            // สถานะการทำงานของมอเตอร์
unsigned long lastMillis = 0;      // ตัวแปรสำหรับเก็บเวลาในการ Publish

// WiFi และ MQTT Config
const char ssid[] = "nutty";
const char pass[] = "12345678";
const char mqtt_broker[] = "test.mosquitto.org";
const char mqtt_topic[] = "group2.21/command";
const char mqtt_client_id[] = "mini_project_group_2.21";
int MQTT_PORT = 1883;

// ตัวแปร MQTT
WiFiClient net;
MQTTClient client;

// ตัวแปร LCD
LiquidCrystal_PCF8574 lcd(0x27);

// Timer variables
unsigned long startTime;
unsigned long countdownDuration = 30000; // Default to 10 minutes 10 * 60 * 1000
bool timerRunning = false;

// ฟังก์ชันเชื่อมต่อ WiFi และ MQTT
void connect() {
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi connected!");

  Serial.print("Connecting to MQTT...");
  while (!client.connect(mqtt_client_id)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nMQTT connected!");
  client.subscribe(mqtt_topic);
}

// ฟังก์ชันจัดการข้อความที่ได้รับจาก MQTT
void messageReceived(String &topic, String &payload) {
  Serial.println("Received: " + topic + " - " + payload);

  if (payload == "on") {
    isRunning = true;
    startTime = millis(); // เริ่มต้นจับเวลา
    timerRunning = true;  // เริ่มต้นนับถอยหลัง
    Serial.println("The washing machine is running..");
  } else if (payload == "off") {
    isRunning = false;
    timerRunning = false; // หยุดนับถอยหลัง
    Serial.println("The washing machine stopped.");
  }
}

void setup() {
  // ตั้งค่าพิน
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENABLE1, OUTPUT);

  // เปิดใช้งาน L293D
  digitalWrite(ENABLE1, HIGH);

  // เริ่มต้น Serial และ WiFi
  Serial.begin(115200); // Set baud rate to 115200
  WiFi.begin(ssid, pass);

  // เริ่มต้น MQTT
  client.begin(mqtt_broker, MQTT_PORT, net);
  client.onMessage(messageReceived);

  connect();

  // เริ่มต้น I2C และ LCD
  Wire.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(255);

  // แสดงข้อความเริ่มต้น
  lcd.setCursor(0, 0);
  lcd.print("Washing Machine");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready");
}

void loop() {
  client.loop();
  delay(10); // เสถียรภาพของ WiFi และ MQTT

  if (!client.connected()) {
    connect();
  }

  // การทำงานของมอเตอร์
  if (!isRunning) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  } else {
    unsigned long currentTime = millis();

    if (isPaused) {
      if (currentTime - lastActionTime >= pauseInterval) {
        isPaused = false;
        clockwise = !clockwise;
        lastActionTime = currentTime;
      }
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
    } else {
      if (currentTime - lastActionTime >= rotationInterval) {
        isPaused = true;
        lastActionTime = currentTime;
      } else {
        if (clockwise) {
          digitalWrite(IN1, HIGH);
          digitalWrite(IN2, LOW);
        } else {
          digitalWrite(IN1, LOW);
          digitalWrite(IN2, HIGH);
        }
      }
    }
  }

  // การแสดงผลบนจอ LCD
  if (timerRunning) {
    unsigned long elapsedTime = millis() - startTime;
    if (elapsedTime < countdownDuration) {
      unsigned long remainingTime = countdownDuration - elapsedTime;
      int minutes = remainingTime / 60000;
      int seconds = (remainingTime % 60000) / 1000;

      lcd.setCursor(0, 0);
      lcd.print("Time Left:      ");
      lcd.setCursor(0, 1);
      lcd.print(minutes);
      lcd.print(" min ");
      lcd.print(seconds);
      lcd.print(" sec ");
    } else {
      timerRunning = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Finish Washing!");
    }
  }

  // ส่งสถานะไปยัง MQTT ทุก 2 วินาที
  if (millis() - lastMillis > 2000) {
    lastMillis = millis();
    if (isRunning) {
      client.publish(mqtt_topic, "The washing machine is running...");
      Serial.println("Status: The washing machine is running...");
    } else {
      client.publish(mqtt_topic, "The washing machine stopped.");
      Serial.println("Status: The washing machine stopped.");
    }
  }

  delay(1000); // Update every second
}
