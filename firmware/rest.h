#include <WebServer.h>

#include "Animation/animation.h"
#include "index.html.h"
#include "rtc.h"

// animation settings
bool animationsEnabled[NUM_ANIMATIONS];
TransitionBehavior transitionBehavior = TransitionBehavior::Sequential;

// brightness settings
int brightness;
struct tm dayTransitionTime;
int dayBrightness;
struct tm nightTransitionTime;
int nightBrightness;

WebServer server(80);

void restIndex() {
  server.send(200, "text/html", indexHtml);
  log_i("Served index.html");
}

void restGetState() {
  // lighterweight than using a json library
  String response = "{";
  response += "\"animations\": [";
  for (int i = 1; i < NUM_ANIMATIONS; i++) { // skip name animation
    response += "{\"name\": \"" + String(AnimationTypeStrings[i]) + "\", \"enabled\": " +
                (animationsEnabled[i] ? "true" : "false") + "}";
    if (i < NUM_ANIMATIONS - 1) response += ",";
  }
  response += "],";
  response += "\"transitionBehavior\": \"" + String(transitionBehavior == TransitionBehavior::Sequential ? "sequential" : "random") + "\",";
  response += "\"brightness\": " + String(100 - (brightness / 2.55)) + ",";
  response += "\"dayBrightness\": " + String(100 - (dayBrightness / 2.55)) + ",";
  // Format dayTransitionTime as HH:mm
  String dayHour = String(dayTransitionTime.tm_hour);
  String dayMinute = String(dayTransitionTime.tm_min);
  if(dayTransitionTime.tm_hour < 10) dayHour = "0" + dayHour;
  if(dayTransitionTime.tm_min < 10) dayMinute = "0" + dayMinute;
  response += "\"dayTransitionTime\": \"" + dayHour + ":" + dayMinute + "\",";
  response += "\"nightBrightness\": " + String(100 - (nightBrightness / 2.55)) + ",";
  // Format nightTransitionTime as HH:mm
  String nightHour = String(nightTransitionTime.tm_hour);
  String nightMinute = String(nightTransitionTime.tm_min);
  if(nightTransitionTime.tm_hour < 10) nightHour = "0" + nightHour;
  if(nightTransitionTime.tm_min < 10) nightMinute = "0" + nightMinute;
  response += "\"nightTransitionTime\": \"" + nightHour + ":" + nightMinute + "\"";
  response += "}";
  server.send(200, "application/json", response);
  log_i("Sent device state");
}

void restRestart() {
  server.send(200, "text/plain", "Restarting...");
  log_w("Restarting...");
  ESP.restart();
}

void restSyncTime() {
  rtcSyncTime();
  server.send(200, "text/plain", "Time synced");
  log_i("Time synced");
}

void restSetAnimationState(bool isEnabled) {
  if (!server.hasArg("name")) {
    server.send(400, "text/plain", "No animation provided");
    log_w("No animation provided");
    return;
  }

  // try to parse value of arg as enum AnimationType
  String animationStr = server.arg("name");
  AnimationType animationType = parseAnimationType(animationStr);
  if (animationType == AnimationType::Invalid) {
    server.send(400, "text/plain", "Invalid animation name: " + animationStr);
    log_w("Invalid animation name: %s", animationStr.c_str());
    return;
  }

  animationsEnabled[animationType] = isEnabled;
  server.send(200, "text/plain", "Animation " + animationStr + " " + (isEnabled ? "enabled" : "disabled"));
  log_i("Animation %s %s", animationStr.c_str(), (isEnabled ? "enabled" : "disabled"));
}

void restSetTransitionType() {
  if (!server.hasArg("type")) {
    server.send(400, "text/plain", "No transition type provided");
    log_w("No transition type provided");
    return;
  }

  String transitionStr = server.arg("type");
  transitionStr.toLowerCase();
  if (transitionStr == "sequential") {
    transitionBehavior = TransitionBehavior::Sequential;
    server.send(200, "text/plain", "Transition type set to sequential");
    log_i("Transition type set to sequential");
  } else if (transitionStr == "random") {
    transitionBehavior = TransitionBehavior::Random;
    server.send(200, "text/plain", "Transition type set to random");
    log_i("Transition type set to random");
  } else {
    server.send(400, "text/plain", "Invalid transition type: " + transitionStr);
    log_w("Invalid transition type: %s", transitionStr.c_str());
    return;
  }
}

void restSetBrightness() {
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "No brightness value provided");
    log_w("No brightness value provided");
    return;
  }

  int brightnessPct = -1;
  try {
    brightnessPct = server.arg("value").toInt();
  } catch (const std::exception& e) {
    server.send(400, "text/plain", "Invalid brightness value" + brightnessPct);
    log_w("Invalid brightness value: %s", e.what());
    return;
  }

  if (brightnessPct < 0 || brightnessPct > 100) {
    server.send(400, "text/plain", "Brightness out of range:" + brightnessPct);
    log_w("Brightness out of range: %d", brightnessPct);
    return;
  }

  brightness = (255.0 - 2.55 * (100 - brightnessPct));
  server.send(200, "text/plain",
              "Brightness set to " + String(brightnessPct) + "%");
  log_i("Brightness set to %d%%", brightnessPct);
}

void restSetup() {
  server.on("/", HTTP_GET, restIndex);
  server.on("/index.html", HTTP_GET, restIndex);
  server.on("/getState", HTTP_GET, restGetState);
  server.on("/restart", HTTP_GET, restRestart);
  server.on("/syncTime", HTTP_GET, restSyncTime);

  server.on("/enableAnimation", HTTP_GET, []() { restSetAnimationState(true); });
  server.on("/disableAnimation", HTTP_GET, []() { restSetAnimationState(false); });
  server.on("/setTransitionType", HTTP_GET, restSetTransitionType);

  server.on("/setBrightness", HTTP_GET, restSetBrightness);
  // server.on("/setDayBrightness", HTTP_GET, handleBrightness);
  // server.on("/setDayStart", HTTP_GET, handleBrightness);
  // server.on("/setNightBrightness", HTTP_GET, handleBrightness);
  // server.on("/setNightStart", HTTP_GET, handleBrightness);

  //server.on("/recentLogs", HTTP_GET, restRecentLogs);
  server.begin();

  log_i("REST server running");
}