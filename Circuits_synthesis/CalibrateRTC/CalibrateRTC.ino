#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
    // Set the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  Serial.println("RTC is set.");
}

void loop() {
  DateTime now = rtc.now();

  Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL));

  delay(1000);
}
