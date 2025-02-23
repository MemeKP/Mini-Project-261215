#define MOTOR  18
#define LED    22

void setup()
{
  pinMode(MOTOR, OUTPUT);
  pinMode(LED, OUTPUT);
}

void loop()
{
  digitalWrite(MOTOR, HIGH);
  digitalWrite(LED, HIGH);
  delay(5000); // Wait for 5000 millisecond(s)
  digitalWrite(MOTOR, LOW);
  digitalWrite(LED,LOW);
  delay(5000); // Wait for 5000 millisecond(s)
}
