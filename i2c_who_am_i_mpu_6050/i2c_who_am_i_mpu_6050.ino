#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(); // Join I2C bus (SDA=21, SCL=22)
  
  Serial.println("Checking MPU6050 WHO_AM_I register...");
  
  Wire.beginTransmission(0x68); // Address of MPU6050
  Wire.write(0x75);             // Point to the WHO_AM_I register
  Wire.endTransmission(false);  // Keep bus active
  
  Wire.requestFrom(0x68, 1);    // Request 1 byte
  
  if (Wire.available()) {
    byte whoAmI = Wire.read();
    Serial.print("WHO_AM_I value is: 0x");
    Serial.println(whoAmI, HEX);
    
    if (whoAmI == 0x68) {
      Serial.println("SUCCESS: Sensor logic is alive!");
    } else {
      Serial.println("FAILURE: Read wrong value.");
    }
  } else {
    Serial.println("FAILURE: Device did not respond.");
  }
}

void loop() {
  // Do nothing
}