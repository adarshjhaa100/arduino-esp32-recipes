void setup() {
  // put your setup code here, to run once:
  /*
    In this scenario, we'll send a signal from arduino's PIN_NO 9
    This signal will be the +ve end of voltage
    For ground/-ve end, there is a special GND pin above 13.
    We'll attach it to the longer -ve lines on either end of board
  */
  pinMode(9, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(9, HIGH); // Send current/signal through pin 9 +ve volt 
  delay(500);
  digitalWrite(9, LOW); // send 0 V
  delay(500);
}
