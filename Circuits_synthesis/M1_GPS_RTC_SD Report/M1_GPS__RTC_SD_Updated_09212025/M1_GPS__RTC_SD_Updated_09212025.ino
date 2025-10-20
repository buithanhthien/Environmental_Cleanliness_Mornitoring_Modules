#include <Wire.h>
#include <TinyGPS++.h>
#include <SD.h>
#include <RTClib.h>

#define chipSelect 53        // For Arduino Mega: CS pin of SD shield/module
#define LOG_INTERVAL_MS 1000 // Log data every 1 second

File myFile;
TinyGPSPlus gps;
RTC_DS1307 rtc;

unsigned long lastLogMs = 0;

void writeHeader() {
  myFile = SD.open("M1.csv", FILE_WRITE);
  if (myFile) {
    myFile.println("Latitude,Longitude,Satellites,HDOP,Altitude(m),Speed(km/h),Course(°),GPSTime,RTCTime");
    myFile.close();
    Serial.println("Header written to M1.csv.");
  } else {
    Serial.println("Error opening M1.csv.");
  }
}

void setup() {
  Serial.begin(9600);
  // For native USB boards (not Mega), you may need to wait for Serial
  // while (!Serial) {;}

  Serial1.begin(115200);   // GPS (Mega: RX1=19, TX1=18)
  Wire.begin();            // I2C for RTC (Mega: SDA=20, SCL=21)

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  // Adjust RTC to compile time (only once if needed)
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
  }

  // Write header if the file does not exist or is empty
  myFile = SD.open("M1.csv", FILE_READ);
  if (myFile) {
    if (myFile.size() == 0) {
      myFile.close();
      writeHeader();
    } else {
      myFile.close();
    }
  } else {
    writeHeader();
  }

  Serial.println("System ready.");
}

void loop() {
  // 1) Always feed GPS data (non-blocking)
  while (Serial1.available()) {
    gps.encode(Serial1.read());
  }

  // 2) Read RTC time and print to Serial (for monitoring)
  DateTime now = rtc.now();
  Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL)); // Example: 2025-09-21T19:xx:xx

  // 3) Log data according to interval, independent of GPS isUpdated()
  unsigned long ms = millis();
  if (ms - lastLogMs >= LOG_INTERVAL_MS) {
    lastLogMs = ms;

    // Prepare GPS fields (if no fix -> N/A)
    bool hasFix = gps.location.isValid() && gps.location.isUpdated();
    // If you want "last known" instead of strictly updated, use gps.location.isValid() only.

    // GPS time in Vietnam timezone if valid
    char gpsTime[10] = "N/A";
    if (gps.time.isValid()) {
      uint8_t hour_vn = gps.time.hour() + 7;
      if (hour_vn >= 24) hour_vn -= 24;
      sprintf(gpsTime, "%02d:%02d:%02d", hour_vn, gps.time.minute(), gps.time.second());
    }

    // 4) Write to file
    myFile = SD.open("M1.csv", FILE_WRITE);
    if (myFile) {
      if (gps.location.isValid()) {
        myFile.print(gps.location.lat(), 6); myFile.print(",");
        myFile.print(gps.location.lng(), 6); myFile.print(",");
      } else {
        myFile.print("N/A,N/A,");
      }

      if (gps.satellites.isValid()) {
        myFile.print(gps.satellites.value());
      } else {
        myFile.print("N/A");
      }
      myFile.print(",");

      if (gps.hdop.isValid()) {
        myFile.print(gps.hdop.value());
      } else {
        myFile.print("N/A");
      }
      myFile.print(",");

      if (gps.altitude.isValid()) {
        myFile.print(gps.altitude.meters());
      } else {
        myFile.print("N/A");
      }
      myFile.print(",");

      if (gps.speed.isValid()) {
        myFile.print(gps.speed.kmph());
      } else {
        myFile.print("N/A");
      }
      myFile.print(",");

      if (gps.course.isValid()) {
        myFile.print(gps.course.deg());
      } else {
        myFile.print("N/A");
      }
      myFile.print(",");

      myFile.print(gpsTime); myFile.print(",");

      // RTC Time (ISO format from RTClib)
      myFile.println(now.timestamp(DateTime::TIMESTAMP_FULL));

      myFile.close();
      Serial.println("Data written to M1.csv.");
    } else {
      Serial.println("Error opening M1.csv.");
    }
  }

  // Loop delay (small to avoid blocking GPS feed)
  delay(100);
}
