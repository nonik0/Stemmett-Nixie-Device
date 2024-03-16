#pragma once

#include <ESP32Time.h>
#include <RTClib.h>

// NTP config
const char* NtpServer = "pool.ntp.org";
const long GmtOffsetSecs = -28800;
const int DstOffsetSecs = 3600;

ESP32Time espRtc;
RTC_DS3231 rtc;

bool rtcInit = false;

void rtcSyncTime() {
  char format[] = "hh:mm:ss";
  if (rtcInit)
    log_i("DS3231: %s", rtc.now().toString(format));
  log_i("ESP: %s", espRtc.getTime());

  struct tm timeinfo;
  getLocalTime(&timeinfo);

  int yr = timeinfo.tm_year + 1900;
  int mt = timeinfo.tm_mon + 1;
  int dy = timeinfo.tm_mday;
  int hr = timeinfo.tm_hour;
  int mi = timeinfo.tm_min;
  int se = timeinfo.tm_sec;

  log_i("NTP: %02u:%02u:%02u", hr, mi, se);

  log_i("Adjusting ESP32 RTC with NTP time");
  espRtc.setTimeStruct(timeinfo);

  if (rtcInit) {
    log_i("Adjusting DS3231 with NTP time");
    rtc.adjust((yr, mt, dy, hr, mi, se));
  }
}

void rtcSetup() {
  log_i("rtcSetup()");

  if (!(rtcInit = rtc.begin()))
    log_w("Couldn't find RTC");

  configTime(GmtOffsetSecs, DstOffsetSecs, NtpServer);
  rtcSyncTime();
}
