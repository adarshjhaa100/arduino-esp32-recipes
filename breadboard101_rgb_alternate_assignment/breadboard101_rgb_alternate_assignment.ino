void setup() {
  // put your setup code here, to run once:
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  // pinMode(10, OUTPUT);
   
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(12, HIGH);
  digitalWrite(11, HIGH);
  // digitalWrite(10, HIGH);
  delay(1000); // 12 - H, 11 - H, from 0 - 1000
  digitalWrite(11, LOW);
  delay(1000); // 11 - L, 12 - H, from 1000-2000
  digitalWrite(11, HIGH);
  digitalWrite(12, LOW);
  delay(1000);
  digitalWrite(11, LOW);
  delay(1000);
}
