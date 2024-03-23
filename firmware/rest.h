#include <WebServer.h>

#include "Animation/animation.h"
#include "index.html.h"
#include "rtc.h"

extern bool animationsEnabled[NUM_ANIMATIONS];
extern int brightness;
extern TransitionBehavior transitionBehavior;

WebServer server(80);

void restIndex() {
  server.send(200, "text/html", indexHtml);
}

void restSetAnimation(bool isEnabled) {
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

void restRestart() {
  server.send(200, "text/plain", "Restarting...");
  ESP.restart();
}

void restBrightness() {
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
  log_w("Brightness set to %d%%", brightnessPct);
}

void restSetAnimationTransition() {
  if (!server.hasArg("type")) {
    server.send(400, "text/plain", "No transition type provided");
    return;
  }

  String transitionStr = server.arg("type");
  transitionStr.toLowerCase();
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

void restSyncTime() {
  rtcSyncTime();
  server.send(200, "text/plain", "Time synced");
}

void restSetup() {
  server.on("/", HTTP_GET, restIndex);
  server.on("/enableAnimation", HTTP_GET, []() { restSetAnimation(true); });
  server.on("/disableAnimation", HTTP_GET, []() { restSetAnimation(false); });
  //server.on("/recentLogs", HTTP_GET, restRecentLogs);
  server.on("/restart", HTTP_GET, restRestart);
  server.on("/runTimeSync", HTTP_GET, restSyncTime);
  server.on("/setAnimationTransitionType", HTTP_GET, restSetAnimationTransition);
  server.on("/setBrightness", HTTP_GET, restBrightness);
  // server.on("/setDayBrightness", HTTP_GET, handleBrightness);
  // server.on("/setDayStart", HTTP_GET, handleBrightness);
  // server.on("/setNightBrightness", HTTP_GET, handleBrightness);
  // server.on("/setNightStart", HTTP_GET, handleBrightness);
  server.begin();
}