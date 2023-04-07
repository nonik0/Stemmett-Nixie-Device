#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <pwmWrite.h>

#include "main.h"
#include "secrets.h"

// Stella configuration
Tube Tubes[NUM_TUBES] = {
            {IN7,  IN7_A    /*A*/, PWM_PIN_1, 0, 0},
          {IN4,  IN4_7    /*L*/, PWM_PIN_2, 0, 2},
        {IN4,  IN4_7    /*L*/, PWM_PIN_3, 0, 4},
      {IN7A, IN7_m    /*E*/, PWM_PIN_4, 0, 6},
    {IN7A, IN7_Plus /*T*/, PWM_PIN_5, 0, 8},
  {IN7B, IN7B_S   /*S*/, PWM_PIN_6, 0, 0},
};

Pwm pwm;
hw_timer_t *refreshTimer;
volatile bool refreshTick = false;
volatile int stellaDelay = 5000;

uint8_t randomDigit(TubeType type) {
  return TubeCathodes[type][random(TubeCathodeCount[type])];
}

void IRAM_ATTR refreshTimerCallback() {
  refreshTick = true;
  stellaDelay--;
  for (int i = 0; i < NUM_TUBES; i++) {
    Tubes[i].Delay--;
  }
}

void otaSetup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else  // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
        // using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void nixieDisplay(uint8_t tube6, uint8_t tube5, uint8_t tube4, uint8_t tube3, uint8_t tube2, uint8_t tube1) {
  // each tube (at most 10 digits/cathodes) is encoded by 4 bits, so need 3
  // total bytes to shift out all tube data to three 8-bit registers
  digitalWrite(LATCH_PIN, LOW);
  uint8_t byte1 = (0xF0 & (tube6 << 4)) | (0x0F & tube5);
  uint8_t byte2 = (0xF0 & (tube4 << 4)) | (0x0F & tube3);
  uint8_t byte3 = (0xF0 & (tube2 << 4)) | (0x0F & tube1);

  SPI.transfer(byte1);
  SPI.transfer(byte2);
  SPI.transfer(byte3);
  digitalWrite(LATCH_PIN, HIGH);
}

void nixieDisplay() {
  Serial.printf("%d%d%d%d%d%d\n", Tubes[5].ActiveCathode, Tubes[4].ActiveCathode, Tubes[3].ActiveCathode, Tubes[2].ActiveCathode, Tubes[1].ActiveCathode, Tubes[0].ActiveCathode);
  nixieDisplay(
    TubeCathodes[Tubes[5].Type][Tubes[5].ActiveCathode],
    TubeCathodes[Tubes[4].Type][Tubes[4].ActiveCathode],
    TubeCathodes[Tubes[3].Type][Tubes[3].ActiveCathode],
    TubeCathodes[Tubes[2].Type][Tubes[2].ActiveCathode],
    TubeCathodes[Tubes[1].Type][Tubes[1].ActiveCathode],
    TubeCathodes[Tubes[0].Type][Tubes[0].ActiveCathode]);
}

bool stella = false;
void handleRefresh() {
  if (refreshTick) {
    refreshTick = false;

    if (stellaDelay < 0) {
      stella = !stella;
      stellaDelay = stella ? 2000 : 5000;

      for (int i = 0; i < NUM_TUBES; i++) {
        nixieDisplay(
          Tubes[5].PrimaryCathode,
          Tubes[4].PrimaryCathode,
          Tubes[3].PrimaryCathode,
          Tubes[2].PrimaryCathode,
          Tubes[1].PrimaryCathode,
          Tubes[0].PrimaryCathode);
      }
    }

    if (!stella) {
      bool update = false;

      for (int i = 0; i < NUM_TUBES; i++) {
        if (Tubes[i].Delay < 0) {
          Tubes[i].ActiveCathode = (Tubes[i].ActiveCathode + 1) % TubeCathodeCount[Tubes[i].Type];
          Tubes[i].Delay = 45;
          update = true;
        }
      }

      if (update) {      
        nixieDisplay();
      }
    }

    if (refreshTick) {
      Serial.println("Refresh took too long");
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");

  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 0; i < NUM_TUBES; i++) {
    //pwm.write(Tubes[i].AnodePin, PWM_MAX, PWM_FREQUENCY);
    pinMode(Tubes[i].AnodePin, OUTPUT);
    digitalWrite(Tubes[i].AnodePin, LOW);
  }

  SPI.begin();

  // draws a lot of current, should do before enabling tubes
  otaSetup();

  refreshTimer = timerBegin(0, 80, true);  // 80Mhz / 80 = 1Mhz
  timerAttachInterrupt(refreshTimer, &refreshTimerCallback, true);
  timerAlarmWrite(refreshTimer, REFRESH_RATE_US, true);
  timerAlarmEnable(refreshTimer);

  for (int i = 0; i < NUM_TUBES; i++) {
    //pwm.write(Tubes[i].AnodePin, PWM_MAX, PWM_FREQUENCY);
    pinMode(Tubes[i].AnodePin, OUTPUT);
    digitalWrite(Tubes[i].AnodePin, HIGH);
  }

  Serial.println("Setup complete");
}

void loop() {
  handleRefresh();
  ArduinoOTA.handle();
}
