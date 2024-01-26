#include <RTClib.h>

// NTP config
const char* NtpServer = "pool.ntp.org";
const long GmtOffsetSecs = -28800;
const int DstOffsetSecs = 3600;

RTC_DS3231 rtc;

void rtcSetup() {
  Serial.println("rtcSetup()");

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    return;
  }

  configTime(GmtOffsetSecs, DstOffsetSecs, NtpServer);

  char format[] = "hh:mm:ss";
  Serial.printf("DS3231: %s\n", rtc.now().toString(format));

  struct tm timeinfo;
  getLocalTime(&timeinfo); // this adjusts ESP32 RTC

  int yr = timeinfo.tm_year + 1900;
  int mt = timeinfo.tm_mon + 1;
  int dy = timeinfo.tm_mday;
  int hr = timeinfo.tm_hour;
  int mi = timeinfo.tm_min;
  int se = timeinfo.tm_sec;

  Serial.printf("   NTP: %02u:%02u:%02u\n", hr, mi, se);
  Serial.println("Adjusting DS3231 with NTP time");
  rtc.adjust(DateTime(yr, mt, dy, hr, mi, se));
}