#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiManager.h>

WiFiManager wifiManager;

void otaSetup()
{
  ArduinoOTA
      .onStart([]()
               {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else  // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
        // using SPIFFS.end()
        log_i("Start updating %s", type.c_str()); })
      .onEnd([]()
             { log_i("End"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          log_i("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          log_i("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          log_i("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          log_i("Receive Failed");
        else if (error == OTA_END_ERROR)
          log_i("End Failed"); });
  ArduinoOTA.begin();

  log_i("OTA server running");
}

bool wifiSetup()
{
  log_d("Setting up WiFi");
  // WiFi.disconnect(true, true);
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(WIFI_SSID, WIFI_PASS);
  // WiFi.setTxPower(WIFI_POWER_8_5dBm);
  // if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   WiFi.begin(WIFI_SSID, WIFI_PASS);

  //   uint8_t wifiAttempts = 0;
  //   while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
  //     Serial.print(".");
  //     delay(1000);
  //     if (wifiAttempts == 10) {
  //       WiFi.disconnect(true, true);
  //       WiFi.begin(WIFI_SSID, WIFI_PASS);
  //     }
  //     wifiAttempts++;
  //   }

  //   log_w("Connection Failed! Rebooting...");
  //   delay(5000);
  //   ESP.restart();
  // }

  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setConnectTimeout(10);
  wifiManager.setDarkMode(true);
  bool result = wifiManager.autoConnect("Stemmett");

  if (result)
  {
    log_i("IP: %s", WiFi.localIP().toString().c_str());
  }
  else
  {
    log_e("Failed to connect to WiFi");
    return false;
  }

  log_i("WiFi setup complete");
  return true;
}

int wifiDisconnects = 0;
int wifiLastUpdateMillis = 0;
int wifiStatusDelayMs = 0;
void checkWifiStatus()
{
  if (millis() - wifiLastUpdateMillis > wifiStatusDelayMs)
  {
    try
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        log_w("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        wifiDisconnects++;
        log_w("Reconnected to WiFi");
      }
    }
    catch (const std::exception &e)
    {
      log_e("Wifi error: %s", e.what());
      wifiStatusDelayMs = 10 * 60 * 1000; // 10   minutes
    }

    wifiStatusDelayMs = 60 * 1000; // 1 minute
    wifiLastUpdateMillis = millis();
  }
}

void mDnsSetup()
{
  if (!MDNS.begin("stemmett"))
  {
    log_e("Error setting up mDNS");
    return;
  }

  MDNS.addService("http", "tcp", 80);
  log_i("mDNS responder started");
}