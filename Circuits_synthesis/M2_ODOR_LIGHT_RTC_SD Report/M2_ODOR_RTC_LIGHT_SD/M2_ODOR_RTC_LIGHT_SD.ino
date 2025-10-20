#include <Wire.h>       // I2C communication library
#include <BH1750.h>     // Library for BH1750 light sensor
#include <SD.h>         // Library for working with SD card
#include <RTClib.h>     // Library for RTC module DS1307

// Define pin for odor sensor
int odorPin = A0;            // Odor sensor connected to analog pin A0
int odorSensorValue = 0;     // Variable to store value read from the odor sensor

// Measurement interval (in seconds)
#define measureTime 1

// Create BH1750 light sensor object
BH1750 lightMeter;

// Create RTC object for DS1307
RTC_DS1307 rtc;

// SD card
#define chipSelect 53  // Chip select pin for SD card on Arduino Mega

File dataFile; // File object for SD card operations

void setup() {
  Serial.begin(9600); // Start Serial Monitor with baud rate 9600
  Wire.begin();       // Initialize I2C communication

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, setting the time to compile time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize light sensor
  if (lightMeter.begin()) {
    Serial.println("Light sensor connected.");
  } else {
    Serial.println("Failed to initialize light sensor");
  }

  // Initialize SD card
  if (SD.begin(chipSelect)) {
    Serial.println("SD card connected.");
  } else {
    Serial.println("SD card initialization failed!");
    while (1);
  }
}

void loop() {
  delay(measureTime * 1000); // Wait before next measurement

  // Read odor sensor value
  odorSensorValue = analogRead(odorPin);

  // Read time from RTC
  DateTime now = rtc.now();

  // Read light level from BH1750
  float lux = lightMeter.readLightLevel();

  // Print data to Serial Monitor
  Serial.print("Time: ");
  Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL));
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  Serial.print("Odor: ");
  Serial.println(odorSensorValue);

  // Save data to SD card
  dataFile = SD.open("M2.csv", FILE_WRITE);

  if (dataFile) {
    // If file is empty, write header first
    if (dataFile.size() == 0) {
      dataFile.println("Timestamp,Light (lx),Odor");
    }

    dataFile.print(now.timestamp(DateTime::TIMESTAMP_FULL));
    dataFile.print(", ");
    dataFile.print(lux, 1);
    dataFile.print(", ");
    dataFile.println(odorSensorValue);
    dataFile.close();
    Serial.println("Data saved to SD card.");
  } else {
    Serial.println("Unable to open file for writing.");
  }

  Serial.println("-----------------------------------------------");
}
