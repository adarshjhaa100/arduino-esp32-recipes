#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define IR_SENSOR_PIN 34

#define DHT11_PIN 4
#define DHTTYPE DHT11


// for SR04 ultrasonic sensor
const int trigPin = 5; // emits ultrasound - output
const int echoPin = 18; // receives reflected wave - input

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The parameter -1 means the display doesn't have a dedicated reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHT11_PIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  // Address 0x3C is common for 128x64 OLEDs
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  delay(2000);
  display.clearDisplay();
}

int counter = 0;

float soundSensorAction() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds - emit sound
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin after reflection, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2; // divided by 2 since sound travelled back+forth
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);
  return distanceCm;
}

void readDht(float dhtReadings[]) {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  dhtReadings[0] = h;
  dhtReadings[1] = t;
}


void loop() {
  display.clearDisplay();
  // Nothing needed here for static text
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE, SSD1306_BLACK); // Draw white text
  display.setCursor(0, 5);
  
  
  display.drawFastHLine(0, 0, SCREEN_WIDTH, SSD1306_INVERSE);
  display.printf("IR Sensor: %d\n", analogRead(IR_SENSOR_PIN));
  display.printf("US Dat: %0.2f cm\n", soundSensorAction());
  
  float dhtReadings[2];
  readDht(dhtReadings);


  display.printf("DHT, H:%0.1f; T:%0.1f\n", dhtReadings[0], dhtReadings[1]);
  
  display.drawChar()

  display.display(); 
  delay(1000);
}