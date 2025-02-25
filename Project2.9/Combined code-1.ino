#include <LCD_I2C.h>
#include <algorithm>
/*
  Combined code with improvements:
  - Added Serial debugging for color values
  - Improved color detection logic
  - Averaged sensor readings for stability
*/

#define MOTOR  18
#define LED    22
#define S2 4          /* Define S2 Pin Number of ESP32 */
#define S3 2          /* Define S3 Pin Number of ESP32 */
#define sensorOut 16  /* Define Sensor Output Pin Number of ESP32 */

/* Enter the Minimum and Maximum Values obtained from Calibration */
int R_Min = 5,  R_Max = 38;  /* Red */
int G_Min = 4,  G_Max = 42;  /* Green */
int B_Min = 4,  B_Max = 35;  /* Blue */

int Red, Green, Blue;
int redValue, greenValue, blueValue;
int Frequency;
String lastDetectedColor = "";

LCD_I2C lcd(0x27, 16, 2);

void setup() {
  pinMode(MOTOR, OUTPUT);
  pinMode(LED, OUTPUT);
  
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  lcd.begin();
  lcd.backlight();
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  // Read color values with averaging for stability
  Red = getStableColor(getRed);
  redValue = map(Red, R_Min, R_Max, 255, 0);
  
  Green = getStableColor(getGreen);
  greenValue = map(Green, G_Min, G_Max, 255, 0);
  
  Blue = getStableColor(getBlue);
  blueValue = map(Blue, B_Min, B_Max, 255, 0);

  // Print sensor values to Serial Monitor
  Serial.print("Red: "); Serial.print(redValue);
  Serial.print("  Green: "); Serial.print(greenValue);
  Serial.print("  Blue: "); Serial.println(blueValue);

  lcd.clear();
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

  // Activate conveyor only if color changed
  if (detectedColor != lastDetectedColor) {
    activateConveyor();
    lastDetectedColor = detectedColor;
  }

  delay(1000);
}

void activateConveyor() {
  Serial.println("Conveyor Activated");
  digitalWrite(MOTOR, HIGH);
  digitalWrite(LED, HIGH);
  delay(5000); // Run conveyor for 5 seconds
  digitalWrite(MOTOR, LOW);
  digitalWrite(LED, LOW);
  Serial.println("Conveyor Stopped");
  delay(1000); // Small delay before next cycle
}

void bubbleSort(int arr[], int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        // สลับค่าถ้าอันซ้ายมากกว่าอันขวา
        int temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}


/*
int getStableColor(int (*getColor)()) {
  int sum = 0;
  for (int i = 0; i < 10; i++) { // Read 5 times
    sum += getColor();
    delay(100); // Small delay between readings
  }
  return sum / 5; // Return the average
}
*/
int getStableColor(int (*getColor)()) {
  int readings[10];
  for (int i = 0; i < 10; i++) {
    readings[i] = getColor();
    delay(50);
  }
  
  bubbleSort(readings, 10);  // เรียงลำดับจากน้อยไปมาก
  return readings[5];        // คืนค่าตรงกลาง (Median)
}


int getRed() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  return pulseIn(sensorOut, LOW);
}

int getGreen() {
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  return pulseIn(sensorOut, LOW);
}

int getBlue() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  return pulseIn(sensorOut, LOW);
}
