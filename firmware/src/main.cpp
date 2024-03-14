#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "animation.h"
#include "nixieDriver.h"
#include "ota.h"
#include "rtc.h"
#include "secrets.h"
#include "tubeConfiguration.h"
#include "tubes.h"

enum TransitionBehavior { Sequential, Random };

TransitionBehavior transitionBehavior = TransitionBehavior::Sequential;
bool animationsEnabled[NUM_ANIMATIONS];
Animation* animations[NUM_ANIMATIONS];
Animation* curAnimation;
int brightness = 255;  // MAX_BRIGHTNESS;

hw_timer_t* refreshTimer;
volatile bool refreshTick = false;

int wifiDisconnects = 0;
WebServer server(80);

uint8_t randomDigit(TubeType type) {
  return TubeCathodes[type][random(TubeCathodeCount[type])];
}

void IRAM_ATTR refreshTimerCallback() {
  refreshTick = true;
}

int i = 0;
int curAnimationIndex = 0;
int newAnimationIndex = 0;
void handleRefresh() {
  if (refreshTick) {
    refreshTick = false;

    // if animation is complete, initialize new animation
    if (curAnimation->isComplete()) {
      if (curAnimationIndex == AnimationType::Name) {
        do {
          if (transitionBehavior == TransitionBehavior::Sequential)
            newAnimationIndex = 1 + (i++) % (NUM_ANIMATIONS - 1);
          else if (transitionBehavior == TransitionBehavior::Random)
            newAnimationIndex = random(1, NUM_ANIMATIONS);
        } while (!animationsEnabled[newAnimationIndex] ||
                 newAnimationIndex == curAnimationIndex);
      } else {
        newAnimationIndex = AnimationType::Name;
      }

      curAnimationIndex = newAnimationIndex;
      curAnimation = animations[newAnimationIndex];
      curAnimation->initialize(Tubes, brightness);
    }

    TickResult result = curAnimation->handleTick(Tubes);

    if (result.CathodeUpdate)
      nixieDisplay(Tubes);
    if (result.BrightnessUpdate)
      nixieBrightness(Tubes);
    // if (refreshTick)
    //   log_w("Refresh took too long");
  }
}

/// REST FUNCTIONS ///
void handleRoot() {
  String html =
      "<!DOCTYPE html><html><body><h2>Stemmett Nixie Device</h2>"
      "<p>Adjust Brightness:</p>"
      "<input type='range' min='0' max='100' value='70' "
      "onchange='updateBrightness(this.value)' id='brightnessSlider'>"
      "<script>"
      "function updateBrightness(brightnessPct) {"
      "  fetch(`/setBrightness?value=${brightnessPct}`).then(response => "
      "response.text()).then(data => console.log(data));"
      "}"
      "</script>"
      "</body></html>";
  server.send(200, "text/html", html);
}

void handleEnable(bool isEnabled) {
  if (!server.hasArg("name")) {
    server.send(400, "text/plain", "No animation provided");
    return;
  }

  // try to parse value of arg as enum AnimationType
  String animationStr = server.arg("name");
  AnimationType animationType = parseAnimationType(animationStr);
  if (animationType == AnimationType::Invalid) {
    server.send(400, "text/plain", "Invalid animation name: " + animationStr);
    return;
  }

  animationsEnabled[animationType] = isEnabled;
  server.send(
      200, "text/plain",
      "Animation " + animationStr + " " + (isEnabled ? "enabled" : "disabled"));
}

void handleRestart() {
  server.send(200, "text/plain", "Restarting...");
  ESP.restart();
}

void handleBrightness() {
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "No brightness value provided");
    return;
  }

  int brightnessPct = -1;
  try {
    brightnessPct = server.arg("value").toInt();
  } catch (const std::exception& e) {
    server.send(400, "text/plain", "Invalid brightness value" + brightnessPct);
    return;
  }

  if (brightnessPct < 0 || brightnessPct > 100) {
    server.send(400, "text/plain", "Brightness out of range:" + brightnessPct);
    return;
  }

  brightness = (255.0 - 2.55 * (100 - brightnessPct));
  server.send(200, "text/plain",
              "Brightness set to " + String(brightnessPct) + "%");
  curAnimation->setBrightness(brightness);
  log_w("Brightness set to %d%%", brightnessPct);
}

void handleTransition() {
  if (!server.hasArg("type")) {
    server.send(400, "text/plain", "No transition type provided");
    return;
  }

  String transitionStr = server.arg("type");
  if (transitionStr == "sequential") {
    transitionBehavior = TransitionBehavior::Sequential;
    server.send(200, "text/plain", "Transition type set to sequential");
  } else if (transitionStr == "random") {
    transitionBehavior = TransitionBehavior::Random;
    server.send(200, "text/plain", "Transition type set to random");
  } else {
    server.send(400, "text/plain", "Invalid transition type: " + transitionStr);
    return;
  }
}

void restSetup() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/enableAnimation", HTTP_GET, []() { handleEnable(true); });
  server.on("/disableAnimation", HTTP_GET, []() { handleEnable(false); });
  server.on("/restart", HTTP_GET, handleRestart);
  server.on("/setBrightness", HTTP_GET, handleBrightness);
  server.on("/setTransitionType", HTTP_GET, handleTransition);
  server.begin();
}

void wifiSetup() {
  log_i("->wifiSetup()");

  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    uint8_t wifiAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
      Serial.print(".");
      delay(1000);
      if (wifiAttempts == 10) {
        WiFi.disconnect(true, true);
        WiFi.begin(WIFI_SSID, WIFI_PASS);
      }
      wifiAttempts++;
    }

    log_w("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  log_i("IP: %s\n", WiFi.localIP().toString().c_str());
  log_i("<-wifiSetup()");
}

int wifiStatusDelayMs = 0;
void checkWifiStatus() {
  wifiStatusDelayMs--;
  if (wifiStatusDelayMs < 0) {
    try {
      if (WiFi.status() != WL_CONNECTED) {
        log_w("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        wifiDisconnects++;
        log_w("Reconnected to WiFi");
      }
    } catch (const std::exception& e) {
      log_e("Wifi error: %s", e.what());
      wifiStatusDelayMs = 10 * 60 * 1000;  // 10   minutes
    }

    wifiStatusDelayMs = 60 * 1000;  // 1 minute
  }
}

void setup() {
  delay(2000);
  Serial.begin(115200);
  log_w("setup()");

  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 0; i < NUM_TUBES; i++) {
    pinMode(Tubes[i].AnodePin, OUTPUT);
    digitalWrite(Tubes[i].AnodePin, LOW);
    // pwm.write(Tubes[i].AnodePin, PWM_MIN, PWM_FREQUENCY);
  }

  SPI.begin();

  wifiSetup();
  otaSetup();
  restSetup();

  // rtcSetup();

  refreshTimer = timerBegin(0, 80, true);  // 80Mhz / 80 = 1Mhz
  timerAttachInterrupt(refreshTimer, &refreshTimerCallback, true);
  timerAlarmWrite(refreshTimer, REFRESH_RATE_US, true);
  timerAlarmEnable(refreshTimer);

  // calculate PWM phase offset for each tube to smooth out power consumption
  for (int i = 0; i < NUM_TUBES; i++) {
    int phaseDeg = (360 / NUM_TUBES) * i;
    Tubes[i].PwmPhase = (phaseDeg / 360.0) * (pow(2, PWM_RESOLUTION));
    pwm.write(Tubes[i].AnodePin, 128, PWM_FREQUENCY, PWM_RESOLUTION,
              Tubes[i].PwmPhase);
  }

  // initialize animations
  animations[AnimationType::BasicFade] = new BasicFadeAnimation();
  animations[AnimationType::Bubble] = new BubbleAnimation();
  animations[AnimationType::CylonScan] = new CylonScanAnimation();
  animations[AnimationType::Ghost] = new GhostAnimation();
  animations[AnimationType::Name] = new NameAnimation();
  animations[AnimationType::NameGlitch] = new NameGlitchAnimation();
  animations[AnimationType::Pulsar] = new PulsarAnimation();
  animations[AnimationType::Rain] = new RainAnimation();
  animations[AnimationType::Scan] = new ScanAnimation();
  animations[AnimationType::SlotMachine] = new SlotMachineAnimation();

  // enable animations
  for (int i = 0; i < NUM_ANIMATIONS; i++) {
    animationsEnabled[i] = true;
  }

  // start first animation
  curAnimation = animations[AnimationType::Name];
  curAnimation->initialize(Tubes, 152);

  log_w("Setup complete");
}

void loop() {
  handleRefresh();
  server.handleClient();
  // ArduinoOTA.handle();
}
