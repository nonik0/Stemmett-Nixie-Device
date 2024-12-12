#pragma once

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <vector>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "rest.h"
#include "secrets.h"

class WifiServices
{
private:
  const int ConnectionTimeoutMs = 10 * 1000;
  const int StatusCheckIntervalMs = 60 * 1000;
  const int TaskPriority = 1; // lowest priority

  const char *_hostname;
  const char *_hostnameLower;
  int _disconnectCount = 0;
  unsigned long _lastStatusCheckMs = 0;
  bool _displayState = true;
  ArduinoOTAClass _ota;
  WiFiManager _wifiManager;

public:
  void setup(const char *hostname);
  void createTask();
  bool isConnected() { return WiFi.status() == WL_CONNECTED; }

private:
  void task();
  void checkWifiStatus();

  bool wifiSetup();
  void otaSetup();
  void mDnsSetup();
};

void WifiServices::setup(const char *hostname)
{
  log_i("Setting up Wifi services for %s", hostname);

  _hostname = hostname;
  _hostnameLower = strdup(hostname);
  char *p = (char *)(_hostnameLower);
  while (*p)
  {
    *p = tolower(*p);
    p++;
  }

  log_i("WiFi services initial setup complete");
}

void WifiServices::createTask()
{
  log_i("Starting WifiServicesTask");

  xTaskCreate(
      [](void *p)
      { ((WifiServices *)p)->task(); },
      "WifiServicesTask",
      8192,
      this,
      TaskPriority,
      NULL);
}

void WifiServices::task()
{
  log_i("WifiServicesTask setup started");
  if (!wifiSetup())
  {
    log_e("Wifi setup failed, will retry at next reboot");
    vTaskDelete(NULL);
    return;
  }

  mDnsSetup();
  otaSetup();
  restSetup();

  log_i("WifiServicesTask setup complete");

  while (1)
  {
    checkWifiStatus();
    _ota.handle();
    server.handleClient();

    delay(10);
  }
}

void WifiServices::checkWifiStatus()
{
  if (millis() - _lastStatusCheckMs > StatusCheckIntervalMs)
  {
    _lastStatusCheckMs = millis();

    try
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        _disconnectCount++;

        log_w("Wifi disconnecting, attempting to reconnect");
        WiFi.disconnect();
        WiFi.reconnect();
        log_w("Reconnected to WiFi");
      }
    }
    catch (const std::exception &e)
    {
      log_e("Wifi error: %s", String(e.what()));
    }
  }
}

bool WifiServices::wifiSetup()
{
  log_i("Wifi setting up...");

  bool connected = false;

  _wifiManager.setConfigPortalTimeout(180);
  _wifiManager.setConnectTimeout(10);
  _wifiManager.setDarkMode(true);

  // try to connect with wifi manager
  if (_wifiManager.autoConnect(DEVICE_NAME))
  {
    log_i("Wifi connected to %s, IP address: %s", _wifiManager.getWiFiSSID().c_str(), WiFi.localIP().toString().c_str());
    return true;
  }

#if defined(WIFI_SSID) && defined(WIFI_PASS)
  // hardcoded wifi 1
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long startMs = millis();
  while (WiFi.waitForConnectResult() != WL_CONNECTED && millis() - startMs < ConnectionTimeoutMs)
    ;
  if (WiFi.status() == WL_CONNECTED)
  {
    log_i("Wifi connected to %s, IP address: %s", WIFI_SSID, WiFi.localIP().toString().c_str());
    return true;
  }
#endif

#if defined(WIFI_SSID2) && defined(WIFI_PASS2)
  // hardcoded wifi 2
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID2, WIFI_PASS2);
  startMs = millis();
  while (WiFi.waitForConnectResult() != WL_CONNECTED && millis() - startMs < ConnectionTimeoutMs)
    ;
  if (WiFi.status() == WL_CONNECTED)
  {
    log_i("Wifi connected to %s, IP address: %s", WIFI_SSID2, WiFi.localIP().toString().c_str());
    return true;
  }
#endif

#if defined(WIFI_SSID3) && defined(WIFI_PASS3)
  // hardcoded wifi 3
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID2, WIFI_PASS2);
  startMs = millis();
  while (WiFi.waitForConnectResult() != WL_CONNECTED && millis() - startMs < ConnectionTimeoutMs)
    ;
  if (WiFi.status() == WL_CONNECTED)
  {
    log_i("Wifi connected to %s, IP address: %s", WIFI_SSID3, WiFi.localIP().toString().c_str());
    return true;
  }
#endif

  log_e("Wifi setup failed");
  return false;
}

void WifiServices::otaSetup()
{
  log_i("OTA setting up...");

  _ota = ArduinoOTA; // use the global instance

  _ota.setHostname(_hostname);
  //_ota.setPasswordHash(OTA_PASS_HASH); TODO: add password hash based off IP?

  _ota
      .onStart([this]()
               { log_i("Start updating %s", _ota.getCommand() == U_FLASH ? "sketch" : "filesystem"); })
      .onEnd([]()
             { log_i("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { log_i("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
        switch(error) {
          case OTA_AUTH_ERROR:
            log_e("Error[%u]: Auth Failed", error);
            break;
          case OTA_BEGIN_ERROR:
            log_e("Error[%u]: Begin Failed", error);
            break;
          case OTA_CONNECT_ERROR:
            log_e("Error[%u]: Connect Failed", error);
            break;
          case OTA_RECEIVE_ERROR:
            log_e("Error[%u]: Receive Failed", error);
            break;
          case OTA_END_ERROR:
            log_e("Error[%u]: End Failed", error);
            break;
        } });

  _ota.begin();

  log_i("OTA setup complete");
}

void WifiServices::mDnsSetup()
{
  if (!MDNS.begin(_hostnameLower))
  {
    log_e("Error setting up mDNS!");
    return;
  }

  log_i("mDNS setup complete");
}