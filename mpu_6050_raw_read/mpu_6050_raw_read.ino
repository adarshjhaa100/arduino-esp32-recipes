#include <Wire.h>

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050 (when AD0 is grounded)

// Variables to hold the raw 16-bit data
int16_t rawAccelX, rawAccelY, rawAccelZ;
int16_t rawTemp;
int16_t rawGyroX, rawGyroY, rawGyroZ;

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C. On ESP32, default SDA=21, SCL=22
  Wire.begin(); 
  
  // 1. WAKE UP THE SENSOR
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // Point to PWR_MGMT_1 register
  Wire.write(0x00);  // Write 0 to wake up the MPU6050
  Wire.endTransmission(true);
  
  Serial.println("MPU6050 Woken Up. Starting data read...");
  delay(1000);
}

void loop() {
  // 2. POINT TO THE START OF THE DATA
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // Register 0x3B (ACCEL_XOUT_H) is the first data register
  Wire.endTransmission(false); // Keep the connection active
  
  // 3. REQUEST 14 BYTES OF DATA
  // 3 axes Accel (6 bytes) + Temp (2 bytes) + 3 axes Gyro (6 bytes) = 14 bytes total
  Wire.requestFrom(MPU_ADDR, 14, true); 
  
  // 4. READ AND STITCH THE BYTES TOGETHER
  // Each reading is 2 bytes: High Byte first, then Low Byte
  rawAccelX = (Wire.read() << 8 | Wire.read()); 
  rawAccelY = (Wire.read() << 8 | Wire.read());
  rawAccelZ = (Wire.read() << 8 | Wire.read());
  
  rawTemp   = (Wire.read() << 8 | Wire.read());
  
  rawGyroX  = (Wire.read() << 8 | Wire.read());
  rawGyroY  = (Wire.read() << 8 | Wire.read());
  rawGyroZ  = (Wire.read() << 8 | Wire.read());
  
  // 5. CONVERT RAW DATA TO REAL-WORLD UNITS
  // Default Accel scale is +/- 2g. Sensitivity factor is 16384 LSB/g.
  float accelX = rawAccelX / 16384.0;
  float accelY = rawAccelY / 16384.0;
  float accelZ = rawAccelZ / 16384.0;
  
  // Default Gyro scale is +/- 250 deg/s. Sensitivity factor is 131 LSB/deg/s.
  float gyroX = rawGyroX / 131.0;
  float gyroY = rawGyroY / 131.0;
  float gyroZ = rawGyroZ / 131.0;
  
  // Factory temperature conversion formula from the datasheet
  float temperature = (rawTemp / 340.0) + 36.53;
  
  // 6. PRINT THE RESULTS
  Serial.print("Accel (g): X="); Serial.print(accelX, 2);
  Serial.print(" Y="); Serial.print(accelY, 2);
  Serial.print(" Z="); Serial.print(accelZ, 2);
  
  Serial.print(" | Gyro (deg/s): X="); Serial.print(gyroX, 2);
  Serial.print(" Y="); Serial.print(gyroY, 2);
  Serial.print(" Z="); Serial.print(gyroZ, 2);
  
  Serial.print(" | Temp (C): "); Serial.println(temperature, 1);
  
  delay(1000); // Wait a bit before reading again
}