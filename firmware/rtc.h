#pragma once

#include <ESP32Time.h>
#include <RTClib.h>

// NTP config
const char *NtpServer = "pool.ntp.org";
const long GmtOffsetSecs = -28800;
const int DstOffsetSecs = 3600;

ESP32Time espRtc;
bool isNtpSynced = false;
#ifdef USE_DS3231_RTC
TwoWire twoWire(0);
RTC_DS3231 ds3231Rtc;
bool ds3231RtcInit = false;
#endif

bool rtcSyncTime();

void rtcGetTime(int &hour, int &minute, int &second)
{
  if (!isNtpSynced)
  {
    log_d("NTP not synced and Wifi connected, attempting to sync again");
    rtcSyncTime();
  }

#ifdef USE_DS3231_RTC
  if (ds3231RtcInit)
  {
    DateTime now = ds3231Rtc.now();
    hour = now.hour();
    minute = now.minute();
    second = now.second();
    log_i("Reading DS3231 RTC: %02u:%02u:%02u", hour, minute, second);
  }
  else
#endif
  {
    hour = espRtc.getHour(true); // 24 hour
    minute = espRtc.getMinute();
    second = espRtc.getSecond();
    log_i("Reading ESP RTC: %02u:%02u:%02u", hour, minute, second);
  }
}

bool rtcSyncTime()
{
  char format[] = "hh:mm:ss";
#ifdef USE_DS3231_RTC
  if (ds3231RtcInit)
  {
    log_i("DS3231: %s", ds3231Rtc.now().toString(format));
  }
#endif
  log_i("ESP: %s", espRtc.getTime());

  struct tm timeinfo;
  isNtpSynced = getLocalTime(&timeinfo);
  if (!isNtpSynced)
  {
    log_w("Failed to obtain NTP time");
    return false;
  }

  int yr = timeinfo.tm_year + 1900;
  int mt = timeinfo.tm_mon + 1;
  int dy = timeinfo.tm_mday;
  int hr = timeinfo.tm_hour;
  int mi = timeinfo.tm_min;
  int se = timeinfo.tm_sec;

  log_i("NTP: %02u:%02u:%02u", hr, mi, se);

  log_i("Adjusting ESP32 RTC with NTP time");
  espRtc.setTimeStruct(timeinfo);

#ifdef USE_DS3231_RTC
  if (ds3231RtcInit)
  {
    log_i("Adjusting DS3231 RTC with NTP time");
    ds3231Rtc.adjust((yr, mt, dy, hr, mi, se));
  }
#endif

  return true;
}

void rtcSetup()
{
  log_d("Setting up RTC");

#ifdef USE_DS3231_RTC
  // swap I2C clock and data because the board is dun goofed for the custom 4-pin RTC port
  if (!twoWire.begin(9, 8))
  {
    log_w("Couldn't find I2C bus");
    return;
  }

  if (!(ds3231RtcInit = ds3231Rtc.begin(&twoWire)))
  {
    log_w("Couldn't find RTC");
  }
#endif

  configTime(GmtOffsetSecs, DstOffsetSecs, NtpServer);
  rtcSyncTime();

  log_d("RTC setup complete");
}
