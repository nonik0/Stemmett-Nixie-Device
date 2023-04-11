#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <pwmWrite.h>

#include "animation.h"
#include "secrets.h"
#include "tubes.h"

Pwm pwm;
hw_timer_t *refreshTimer;
volatile bool refreshTick = false;
volatile int pwmDelay = 0;
volatile int stellaDelay = 5000;

uint8_t randomDigit(TubeType type) {
  return TubeCathodes[type][random(TubeCathodeCount[type])];
}

void IRAM_ATTR refreshTimerCallback() {
  refreshTick = true;
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

Animation *animation0 = new PrimaryCathodeAnimation();
Animation *animation1 = new SlotMachineAnimation();
Animation *curAnimation = animation1;
void handleRefresh() {
  if (refreshTick) {
    refreshTick = false;

    // if animation is complete, create new animation
    if (curAnimation->isComplete()) {
      curAnimation = (curAnimation == animation1) ? animation0 : animation1;
      curAnimation->initialize();
    }

    TickResult result = curAnimation->handleTick();

    if (result.CathodeUpdate) {
      nixieDisplay(Tubes[5].ActiveCathode, Tubes[4].ActiveCathode, Tubes[3].ActiveCathode, Tubes[2].ActiveCathode, Tubes[1].ActiveCathode, Tubes[0].ActiveCathode);
    }

    if (result.BrightnessUpdate) {
      for (int i = 0; i < NUM_TUBES; i++) {
        //Serial.printf("B:%3d|%3d|%3d|%3d|%3d|%3d\n", Tubes[5].Brightness, Tubes[4].Brightness, Tubes[3].Brightness, Tubes[2].Brightness, Tubes[1].Brightness, Tubes[0].Brightness);
        pwm.write(Tubes[i].AnodePin, Tubes[i].Brightness, PWM_FREQUENCY, PWM_RESOLUTION, Tubes[i].PwmPhase);
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
    pinMode(Tubes[i].AnodePin, OUTPUT);
    digitalWrite(Tubes[i].AnodePin, LOW);
    //pwm.write(Tubes[i].AnodePin, PWM_MIN, PWM_FREQUENCY);
  }

  SPI.begin();

  // draws a lot of current, should do before enabling tubes
  otaSetup();

  refreshTimer = timerBegin(0, 80, true);  // 80Mhz / 80 = 1Mhz
  timerAttachInterrupt(refreshTimer, &refreshTimerCallback, true);
  timerAlarmWrite(refreshTimer, REFRESH_RATE_US, true);
  timerAlarmEnable(refreshTimer);

  // calculate PWM phase offset for each tube to smooth out power consumption
  for (int i = 0; i < NUM_TUBES; i++) {
    int phaseDeg = (360 / NUM_TUBES) * i;
    Tubes[i].PwmPhase = (phaseDeg / 360.0) * (pow(2,PWM_RESOLUTION));
    pwm.write(Tubes[i].AnodePin, 128, PWM_FREQUENCY, PWM_RESOLUTION, Tubes[i].PwmPhase);
  }

  Serial.println("Setup complete");
}

void loop() {
  handleRefresh();
  ArduinoOTA.handle();
}


