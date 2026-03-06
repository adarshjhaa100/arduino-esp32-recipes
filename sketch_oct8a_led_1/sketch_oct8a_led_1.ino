#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for SSD1306 display connected using I2C
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void helloWorld() {
  // clear buffer
  display.clearDisplay();
  // Display Text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 28);
  display.println("Hello world!");
  display.display();
  delay(2000);
  display.clearDisplay();
}

void drawCircle() {
  // clear buffer
  display.clearDisplay();
  // Display Text
 //draw circle
  display.setCursor(0, 0);
  display.println("Circle");
  display.drawCircle(20, 35, 20, WHITE);
  display.display();
  delay(2000);
  display.setCursor(100, 0);
  display.println("Circle");
  display.fillCircle(120, 35, 10, WHITE);
  display.display();
  delay(2000);
  display.clearDisplay();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // If using I2C protocol
  // initialize the OLED object
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Uncomment this if you are using SPI
  //if(!display.begin(SSD1306_SWITCHCAPVCC)) {
  //  Serial.println(F("SSD1306 allocation failed"));
  //  for(;;); // Don't proceed, loop forever
  //}
  // helloWorld();

  // drawCircle();

  display.setCursor(0, 0);

}

int x_iter = 10, y_iter = 10, radius = 3;

void loop() {
  // put your main code here, to run repeatedly:
  display.println("Circle");
  display.fillCircle(x_iter, y_iter, radius, WHITE);
  display.display();
  x_iter = x_iter * 2; 
  y_iter = y_iter * 2;
  radius = (radius + 1)%10;

  if(x_iter > SCREEN_WIDTH) {
    x_iter = 10;
  }

  if(y_iter > SCREEN_HEIGHT) {
    y_iter = 10;
  }
  delay(500);
  display.clearDisplay();
}
