#include <WiFi.h> 
#include <WiFiUdp.h> 
#include <aREST.h>

#include "animation.h"
#include "ota.h"
#include "nixieDriver.h"
#include "rtc.h"
#include "secrets.h"
#include "tubeConfiguration.h"
#include "tubes.h"

#define NUM_ANIMATIONS 10
Animation* animations[NUM_ANIMATIONS] = {
  new NameAnimation(),
  new NameGlitchAnimation(),
  new BasicFadeAnimation(),
  new GhostAnimation(),
  new SlotMachineAnimation(),
  new CylonScanAnimation(),
  new PulsarAnimation(),
  new RainAnimation(),
  new BubbleAnimation(),
  new ScanAnimation(),
  //new RandomScanAnimation(),
};
Animation *curAnimation;
int brightness = 255; // MAX_BRIGHTNESS;

hw_timer_t *refreshTimer;
volatile bool refreshTick = false;

aREST rest = aREST();
WiFiServer server(80);

uint8_t randomDigit(TubeType type) {
  return TubeCathodes[type][random(TubeCathodeCount[type])];
}

void IRAM_ATTR refreshTimerCallback() {
  refreshTick = true;
}

int i = 0;
void handleRefresh() {
  if (refreshTick) {
    refreshTick = false;

    // if animation is complete, initialize new animation
    if (curAnimation->isComplete()) {
      curAnimation = (curAnimation == animations[0])
        ? //animations[random(1, NUM_ANIMATIONS)]
          animations[1 + (i++)%(NUM_ANIMATIONS-1)]
        : animations[0];
      curAnimation->initialize(Tubes, brightness);
    }

    TickResult result = curAnimation->handleTick(Tubes);

    if (result.CathodeUpdate) nixieDisplay(Tubes);
    if (result.BrightnessUpdate) nixieBrightness(Tubes);
    if (refreshTick) Serial.println("Refresh took too long");
  }
}

/// REST FUNCTIONS ///
void log(String message) {
  Serial.println(message);
  lastLog3 = lastLog2;
  lastLog2 = lastLog1;
  lastLog1 = "[" + ds3231Rtc.now().timestamp(DateTime::TIMESTAMP_FULL) + "] " + message;
}

int restart(String notUsed) {
  ESP.restart();
  return 0;
}

int setBrightness(String brightnessPctStr) {
  int brightnessPct = -1;
  try {
    brightnessPct = brightnessPctStr.toInt();
  }
  catch (const std::exception& e) {
    log("Invalid brightness value: " + brightnessPctStr);
    return 1;
  }

  brightnessPctStr = String(brightnessPct);
  if (brightnessPct < 0|| brightnessPct > 100) {
    log("Brightness out of range:" + brightnessPctStr);
    return 1;
  }

  brightness = (255.0 - 2.55 * brightnessPct);

  log("Setting brightness to " + brightnessPctStr);
  brightnessDelayMs = 0;
  displayOn = brightness > 0;
  return 0;
}


void restSetup() {
  // aREST config
  //rest.function((char*)"restart", restart);
  //rest.function((char*)"runTimeSync", runTimeSync);
  //rest.function((char*)"setBrightness", setBrightness);
  //rest.variable("brightness", &brightness);
  //rest.variable("brightnessDay", &brightnessDay);
  //rest.variable("brightnessNight", &brightnessNight);
  //rest.variable("displayOn", &displayOn);
  //rest.variable("isNightMode", &isNightMode);
  //rest.variable("lastLog1", &lastLog1);
  //rest.variable("lastLog2", &lastLog2);
  //rest.variable("lastLog3", &lastLog3);
  //rest.variable("wifiDisconnects", &wifiDisconnects);
  rest.set_id("stemmett-nixie-device");
  rest.set_name((char*)"Stemetts Nixie Device");

  // start rest server
  server.begin();
}

int wifiDisconnects = 0;

void wifiSetup() {
  Serial.println("->wifiSetup()");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.printf("IP: %s\n", WiFi.localIP());
  Serial.println("<-wifiSetup()");
}

int wifiStatusDelayMs = 0;
void checkWifiStatus() {
  wifiStatusDelayMs--;
  if (wifiStatusDelayMs < 0) {
    try {
      if (WiFi.status() != WL_CONNECTED) {
        //log("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        wifiDisconnects++;
        //log("Reconnected to WiFi");
      }
    }
    catch (const std::exception& e) {
      //log("Wifi error:" + String(e.what()));
      wifiStatusDelayMs = 10 * 60 * 1000; // 10   minutes
    }

    wifiStatusDelayMs = 60 * 1000; // 1 minute
  }
}

void setup() {
  delay(3000);

  Serial.begin(115200);
  Serial.println("setup()");

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

  wifiSetup();
  otaSetup();
  restSetup();

  //rtcSetup();

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

  // start first animation
  curAnimation = animations[0];
  curAnimation->initialize(Tubes, 152);

  Serial.println("Setup complete");
}

void loop() {
  handleRefresh();
  //ArduinoOTA.handle();
}


