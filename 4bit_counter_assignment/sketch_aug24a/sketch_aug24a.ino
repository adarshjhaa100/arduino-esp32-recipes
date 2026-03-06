int pins[] = {9, 10, 11, 12}; // pins[0] = LSB for this counter
bool signal[] = {LOW, LOW, LOW, LOW};
int iteration = 0;
#define WORD_SIZE 4

void setup() {
  // put your setup code here, to run once:
  for(int i = 0; i<WORD_SIZE; i++){
     pinMode(pins[i], OUTPUT);   
  }
}

void writeBinary(int pins[], bool signal[]){
  for(int i = 0; i<WORD_SIZE; i++){
     digitalWrite(pins[i], signal[i]);   
  }
}

void loop() {
  for(int i = 1; i <= WORD_SIZE; i++) {
    writeBinary(pins, signal);
    delay(1000);
    iteration++;
    signal[0] = !signal[0];
    if(iteration%2 == 0) signal[1] = !signal[1];
    if(iteration%WORD_SIZE == 0) signal[2] = !signal[2];
    if(iteration%(2*WORD_SIZE) == 0) signal[3] = !signal[3];
  }
}
