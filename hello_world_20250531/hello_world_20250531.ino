void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT); // connect to LED in output mode ( host -> device )
}

void loop() {
  // put your main code here, to run repeatedly:
  int a[6] = {1, 3, 5, 7, 9, 12};
  for(int i = 0; i < 6; i++){
    digitalWrite(13, HIGH); // write high volt to pin 13
    delay(a[i]*200); // delay
    digitalWrite(13, LOW);
    delay(a[i]*100); // delay 
  }
  
}
