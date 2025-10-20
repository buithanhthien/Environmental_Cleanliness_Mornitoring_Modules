#include "DFRobot_PH.h"
#include <EEPROM.h>
#include <DS18B20.h>
#include <Wire.h>
#include <SD.h>
#include <RTClib.h>

#define PH_PIN A1           // Analog pin for pH sensor
#define DS18B20_PIN 3       // Digital pin for DS18B20 temperature sensor
#define CHIP_SELECT 53      // SD card CS pin

File dataFile;
DFRobot_PH ph;
DS18B20 ds(DS18B20_PIN);
RTC_DS3231 rtc;

float voltage = 0.0, phValue = -1.0, temperature = 25.0;  // Initial values

// Flags to indicate whether sensors and modules are available
bool sdAvailable = false;
bool phSensorAvailable = false;
bool rtcAvailable = false;
bool tempSensorAvailable = false;
bool headerWritten = false;  // To ensure we only write CSV header once

unsigned long lastLoopTime = 0;

// Check if pH sensor is connected by reading analog value twice
void checkPHSensor() {
  int rawPH1 = analogRead(PH_PIN);
  delay(100);
  int rawPH2 = analogRead(PH_PIN);
  phSensorAvailable = (abs(rawPH1 - rawPH2) >= 3 && rawPH1 >= 50 && rawPH1 <= 1000);
  Serial.println(phSensorAvailable ? "pH sensor connected." : "pH sensor not detected.");
}

// Check if temperature sensor is available and return valid readings
void checkTemperatureSensor() {
  if (ds.selectNext()) {
    float temp = ds.getTempC();
    tempSensorAvailable = (temp != -127.0 && temp != 85.0);
    if (tempSensorAvailable) temperature = temp;  // Save valid temperature
  }
  Serial.println(tempSensorAvailable ? "DS18B20 sensor connected." : "DS18B20 sensor not detected.");
}

// Check SD card connection
void checkSDCard() {
  sdAvailable = SD.begin(CHIP_SELECT);
  Serial.println(sdAvailable ? "SD card connected." : "Failed to connect to SD card.");
}

// Initialize RTC, but do not set time if power is lost
void checkRTC() {
  rtcAvailable = rtc.begin();
  if (rtcAvailable) {
    if (rtc.lostPower()) {
      Serial.println("RTC lost power. Time not set (external program required).");
    }
    Serial.println("RTC connected.");
  } else {
    Serial.println("Failed to connect to RTC.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Give time for serial and sensors to initialize

  ph.begin();  // Initialize pH library

  Serial.println("Initializing sensors...");
  checkPHSensor();         // Check pH sensor presence
  checkTemperatureSensor();// Check temperature sensor presence
  checkSDCard();           // Try to initialize SD card
  checkRTC();              // Check if RTC is working
}

void loop() {
  // Run the loop once every 1000ms
  if (millis() - lastLoopTime >= 1000) {
    lastLoopTime = millis();

    // Update temperature from DS18B20 if available
    if (tempSensorAvailable && ds.selectNext()) {
      float temp = ds.getTempC();
      if (temp != -127.0 && temp != 85.0) temperature = temp;
    }

    // Read voltage from pH sensor and calculate pH
    if (phSensorAvailable) {
      voltage = analogRead(PH_PIN) * 5.0 / 1024.0;  // Convert to voltage
      phValue = ph.readPH(voltage, temperature);    // Calculate pH using library
    } else {
      phValue = -1.0;  // Invalid pH
    }

    // --- Serial Monitor Output ---
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print(" °C | pH: ");
    Serial.print((phValue >= 0.0) ? String(phValue, 2) : "N/A");
    Serial.print(" | Time: ");
    Serial.println(rtcAvailable ? rtc.now().timestamp(DateTime::TIMESTAMP_FULL) : "N/A");

    // --- Save to SD Card ---
    if (sdAvailable) {
      dataFile = SD.open("M3.CSV", FILE_WRITE);
      if (dataFile) {
        if (!headerWritten && dataFile.size() == 0) {
          dataFile.println("Timestamp,Temperature,pH");  // Write CSV header once
          headerWritten = true;
        }

        if (rtcAvailable) {
          dataFile.print(rtc.now().timestamp(DateTime::TIMESTAMP_FULL));  // Timestamp from RTC
        } else {
          dataFile.print("No RTC");
        }

        dataFile.print(",");
        dataFile.print(temperature, 1);
        dataFile.print(",");
        dataFile.println((phValue >= 0.0) ? String(phValue, 2) : "N/A");

        dataFile.close();  // Always close file after writing
        Serial.println("Data saved to SD card.");
      } else {
        Serial.println("Failed to open SD file for writing.");
      }
    }

    Serial.println("---------------------------------------------------");

    // Run pH calibration routine (passive unless calibration command is sent)
    if (phSensorAvailable) {
      ph.calibration(voltage, temperature);
    }
  }
}
