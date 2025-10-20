#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <SD.h>
#include <SPI.h>
#include <RTClib.h>

#define CHIP_SELECT 53  // Chip select pin for SD Card
Adafruit_BNO055 bno = Adafruit_BNO055(55);
RTC_DS1307 rtc;

File myFile;

void setup() {
    Serial.begin(115200);

    // Start I2C (SDA/SCL) for BNO055 and RTC DS1307
    Wire.begin();
    
    if (!bno.begin()) {
        Serial.println("BNO055 not detected!");
    }
    delay(1000);
    bno.setExtCrystalUse(true);

    if (!rtc.begin()) {
        Serial.println("RTC initialization error!");
    }
    Serial.println("RTC connected.");

    // Initialize SD Card
    Serial.print("Initializing SD card...");
    if (!SD.begin(CHIP_SELECT)) {
        Serial.println("Error! Failed to initialize SD card.");
    }
    Serial.println("Success!");

    // Check if CSV file exists; if not, write header
    myFile = SD.open("M4.csv", FILE_READ);
    if (!myFile) {
        writeHeader();
    }
    myFile.close();
}

// Write CSV header
void writeHeader() {
    myFile = SD.open("M4.csv", FILE_WRITE);
    if (myFile) {
        myFile.println("Time,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,MagX,MagY,MagZ");
        myFile.close();
        Serial.println("Header written to M4.csv.");
    } else {
        Serial.println("Error opening M4.csv.");
    }
}

void loop() {
    DateTime now = rtc.now();

    sensors_event_t accel, gyro, mag, temp;
    bno.getEvent(&accel, Adafruit_BNO055::VECTOR_ACCELEROMETER);
    bno.getEvent(&gyro, Adafruit_BNO055::VECTOR_GYROSCOPE);
    bno.getEvent(&mag, Adafruit_BNO055::VECTOR_MAGNETOMETER);

    Serial.print("Time: "); Serial.print(now.timestamp(DateTime::TIMESTAMP_FULL));
    Serial.print(" | Accel: "); Serial.print(accel.acceleration.x); Serial.print(", ");
    Serial.print(accel.acceleration.y); Serial.print(", ");
    Serial.print(accel.acceleration.z);
    Serial.print(" | Gyro: "); Serial.print(gyro.gyro.x); Serial.print(", ");
    Serial.print(gyro.gyro.y); Serial.print(", ");
    Serial.print(gyro.gyro.z);
    Serial.print(" | Mag: "); Serial.print(mag.magnetic.x); Serial.print(", ");
    Serial.print(mag.magnetic.y); Serial.print(", ");
    Serial.println(mag.magnetic.z);

    // Write sensor data to CSV file
    myFile = SD.open("M4.csv", FILE_WRITE);
    if (myFile) {
        myFile.print(now.timestamp(DateTime::TIMESTAMP_FULL)); myFile.print(",");
        myFile.print(accel.acceleration.x); myFile.print(",");
        myFile.print(accel.acceleration.y); myFile.print(",");
        myFile.print(accel.acceleration.z); myFile.print(",");
        myFile.print(gyro.gyro.x); myFile.print(",");
        myFile.print(gyro.gyro.y); myFile.print(",");
        myFile.print(gyro.gyro.z); myFile.print(",");
        myFile.print(mag.magnetic.x); myFile.print(",");
        myFile.print(mag.magnetic.y); myFile.print(",");
        myFile.println(mag.magnetic.z);
        myFile.close();
    } else {
        Serial.println("Error writing to M4.csv.");
    }

    delay(100); // Update every 100 ms
}
