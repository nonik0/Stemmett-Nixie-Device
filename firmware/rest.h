#pragma once

#include <WebServer.h>

#include "index.html.h"
#include "rtc.h"
#include "settings.h"

extern bool displayEnabled;
extern unsigned long dayNightTransitionLastUpdateMillis;
extern unsigned long dayNightTranstionUpdateInteralMs;
extern uint16_t lightSensorAverageReading;

WebServer server(80);

void restIndex()
{
  log_i("Serving index.html");
  server.send(200, "text/html", indexHtml);
  log_i("Served index.html");
}

void restDisplay()
{
  if (server.hasArg("plain") || server.hasArg("value"))
  {

    String cmd = server.hasArg("plain")
      ? server.arg("plain")
      : server.arg("value");
    cmd.toLowerCase();

    bool isValid = false;
    bool state = false;
    if (cmd == "0" || cmd == "off" || cmd == "false")
    {
      state = false;
      isValid = true;
    }
    else if (cmd == "1" || cmd == "on" || cmd == "true")
    {
      state = true;
      isValid = true;
    }

    if (!isValid)
    {
      log_w("Invalid display state value: %s", cmd);
    }
    
    displayEnabled = state;
  }

  server.send(200, "text/plain", displayEnabled ? "on" : "off");
}


void restGetState()
{
  log_i("Sending device state");
  // lighter weight than using a json library
  String response = "{";

  response +=
      "\"transitionBehavior\": \"" +
      String(transitionBehavior == TransitionBehavior::Sequential ? "sequential"
                                                                  : "random") +
      "\",";

  // Format systemTime as HH:mm:ss
  int hour, minute, second;
  rtcGetTime(hour, minute, second);
  String hourStr = String(hour);
  String minuteStr = String(minute);
  String secondStr = String(second);
  if (hour < 10)
    hourStr = "0" + hourStr;
  if (minute < 10)
    minuteStr = "0" + minuteStr;
  if (second < 10)
    secondStr = "0" + secondStr;
  response += "\"systemTime\": \"" + String(hourStr) + ":" + String(minuteStr) +
              ":" + String(secondStr) + "\",";
  response += "\"isNight\": " + String(isNightMode ? "true" : "false") + ",";
  response += "\"isNtpSynced\":" + String(isNtpSynced ? "true" : "false") + ",";

  response += "\"displayEnabled\": " + String(displayEnabled) + ",";
  response += "\"lightSensorReading\": " + String(lightSensorAverageReading) + ",";
  response += "\"lightSensorThreshold\": " + String(lightSensorThreshold) + ",";

  response += "\"animationsDay\": [";
  for (int i = 1; i < NUM_ANIMATIONS; i++)
  { // skip name animation
    response +=
        "{\"name\": \"" + String(AnimationTypeStrings[i]) +
        "\", \"enabled\": " + (animationsEnabledDay[i] ? "true" : "false") + "}";
    if (i < NUM_ANIMATIONS - 1)
      response += ",";
  }
  response += "],";

  int dayBrightnessPct = dayBrightness / 2.55;
  response += "\"dayBrightness\": " + String(dayBrightnessPct) + ",";
  response += "\"dayMaxSpeed\": " + String(animationDaySpeedFactor * 100) + ",";

  // Format dayTransitionTime as HH:mm
  String dayHour = String(dayTransitionTime.tm_hour);
  String dayMinute = String(dayTransitionTime.tm_min);
  if (dayTransitionTime.tm_hour < 10)
    dayHour = "0" + dayHour;
  if (dayTransitionTime.tm_min < 10)
    dayMinute = "0" + dayMinute;
  response += "\"dayTransitionTime\": \"" + dayHour + ":" + dayMinute + "\",";

  response += "\"animationsNight\": [";
  for (int i = 1; i < NUM_ANIMATIONS; i++)
  { // skip name animation
    response +=
        "{\"name\": \"" + String(AnimationTypeStrings[i]) +
        "\", \"enabled\": " + (animationsEnabledNight[i] ? "true" : "false") + "}";
    if (i < NUM_ANIMATIONS - 1)
      response += ",";
  }
  response += "],";

  int nightBrightnessPct = nightBrightness / 2.55;
  response += "\"nightBrightness\": " + String(nightBrightnessPct) + ",";
  response += "\"nightMaxSpeed\": " + String(animationNightSpeedFactor * 100) + ",";

  // Format nightTransitionTime as HH:mm
  String nightHour = String(nightTransitionTime.tm_hour);
  String nightMinute = String(nightTransitionTime.tm_min);
  if (nightTransitionTime.tm_hour < 10)
    nightHour = "0" + nightHour;
  if (nightTransitionTime.tm_min < 10)
    nightMinute = "0" + nightMinute;
  response +=
      "\"nightTransitionTime\": \"" + nightHour + ":" + nightMinute + "\"";

  response += "}";
  server.send(200, "application/json", response);
  log_i("Sent device state");
}

void restGetLightSensorReading()
{
  server.send(200, "text/plain", String(lightSensorAverageReading));
}

void restSetLightSensorThreshold()
{
  if (!server.hasArg("value"))
  {
    server.send(400, "text/plain", "No threshold value provided");
    log_w("No threshold value provided");
    return;
  }

  int threshold = -1;
  try
  {
    threshold = server.arg("value").toInt();
  }
  catch (const std::exception &e)
  {
    server.send(400, "text/plain", "Invalid threshold value" + threshold);
    log_w("Invalid threshold value: %s", e.what());
    return;
  }

  if (threshold < 0 || threshold > 4096)
  {
    server.send(400, "text/plain", "Threshold out of range:" + threshold);
    log_w("Threshold out of range: %d", threshold);
    return;
  }

  lightSensorThreshold = threshold;
  server.send(200, "text/plain",
              "Light sensor threshold set to " + String(threshold));
  log_i("Light sensor threshold set to %d", threshold);
  saveSettings();
}

void restRestart()
{
  server.send(200, "text/plain", "Restarting...");
  log_w("Restarting...");
  ESP.restart();
}

void restSyncTime()
{
  rtcSyncTime();
  server.send(200, "text/plain", "Time synced");
  log_i("Time synced");
}

void restSetAnimationState(bool isEnabled, bool isNight)
{
  if (!server.hasArg("name"))
  {
    server.send(400, "text/plain", "No animation provided");
    log_w("No animation provided");
    return;
  }

  // try to parse value of arg as enum AnimationType
  String animationStr = server.arg("name");
  AnimationType animationType = parseAnimationType(animationStr);
  if (animationType == AnimationType::Invalid)
  {
    server.send(400, "text/plain", "Invalid animation name: " + animationStr);
    log_w("Invalid animation name: %s", animationStr.c_str());
    return;
  }

  if (isNight)
    animationsEnabledNight[animationType] = isEnabled;
  else
    animationsEnabledDay[animationType] = isEnabled;
  server.send(
      200, "text/plain",
      String(isNight ? "Night" : "Day") + " animation " + animationStr + (isEnabled ? " enabled" : " disabled"));
  log_i("%s animation %s %s", animationStr.c_str(),
        (isNight ? "Night" : "Day"),
        (isEnabled ? "enabled" : "disabled"));
  saveSettings();
}

void restSetBrightness(int *brightness, String name)
{
  if (!server.hasArg("value"))
  {
    server.send(400, "text/plain", "No brightness value provided");
    log_w("No brightness value provided");
    return;
  }

  int brightnessPct = -1;
  try
  {
    brightnessPct = server.arg("value").toInt();
  }
  catch (const std::exception &e)
  {
    server.send(400, "text/plain", "Invalid brightness value" + brightnessPct);
    log_w("Invalid brightness value: %s", e.what());
    return;
  }

  if (brightnessPct < 0 || brightnessPct > 100)
  {
    server.send(400, "text/plain", "Brightness out of range:" + brightnessPct);
    log_w("Brightness out of range: %d", brightnessPct);
    return;
  }

  *brightness = (255.0 - 2.55 * (100 - brightnessPct));

  server.send(200, "text/plain",
              name + " brightness set to " + String(brightnessPct) + "%");
  log_i("%s brightness set to %d%%", name.c_str(), brightnessPct);
  saveSettings();

  // update the brightness immediately
  dayNightTranstionUpdateInteralMs = 0;
  dayNightTransitionLastUpdateMillis = 0;
}

void restSetSpeedFactor(float *speedFactor, String name)
{
  if (!server.hasArg("value"))
  {
    server.send(400, "text/plain", "No speed value provided");
    log_w("No speed value provided");
    return;
  }

  int newSpeed = -1;
  try
  {
    newSpeed = server.arg("value").toInt();
  }
  catch (const std::exception &e)
  {
    server.send(400, "text/plain", "Invalid speed value" + newSpeed);
    log_w("Invalid maxSpeed value: %s", e.what());
    return;
  }

  if (newSpeed < 0 || newSpeed > 100)
  {
    server.send(400, "text/plain", "Max speed out of range:" + newSpeed);
    log_w("Max speed out of range: %d", newSpeed);
    return;
  }

  *speedFactor = newSpeed / 100.0;
  server.send(200, "text/plain",
              name + " speed set to " + String(*speedFactor));
  log_i("%s speed set to %.2f", name.c_str(), *speedFactor);
  saveSettings();

  // update the speed immediately
  dayNightTranstionUpdateInteralMs = 0;
  dayNightTransitionLastUpdateMillis = 0;
}

void restSetTransitionTime(tm *transitionTime, String name)
{
  if (!server.hasArg("value"))
  {
    server.send(400, "text/plain", "No transition time provided");
    log_w("No transition time provided");
    return;
  }

  String timeStr = server.arg("value");
  int hour = timeStr.substring(0, 2).toInt();
  int minute = timeStr.substring(3, 5).toInt();
  if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
  {
    server.send(400, "text/plain", "Invalid time format: " + timeStr);
    log_w("Invalid time format: %s", timeStr.c_str());
    return;
  }

  transitionTime->tm_hour = hour;
  transitionTime->tm_min = minute;
  server.send(200, "text/plain", name + " transition time set to " + timeStr);
  log_i("%s transition time set to %s", name, timeStr.c_str());
  saveSettings();

  // update the brightness immediately
  dayNightTranstionUpdateInteralMs = 0;
  dayNightTransitionLastUpdateMillis = 0;
}

void restSetTransitionType()
{
  if (!server.hasArg("value"))
  {
    server.send(400, "text/plain", "No transition type provided");
    log_w("No transition type provided");
    return;
  }

  String transitionStr = server.arg("value");
  transitionStr.toLowerCase();
  if (transitionStr == "sequential")
  {
    transitionBehavior = TransitionBehavior::Sequential;
    server.send(200, "text/plain", "Transition type set to sequential");
    log_i("Transition type set to sequential");
    saveSettings();
  }
  else if (transitionStr == "random")
  {
    transitionBehavior = TransitionBehavior::Random;
    server.send(200, "text/plain", "Transition type set to random");
    log_i("Transition type set to random");
    saveSettings();
  }
  else
  {
    server.send(400, "text/plain", "Invalid transition type: " + transitionStr);
    log_w("Invalid transition type: %s", transitionStr.c_str());
    return;
  }
}

void restSetup()
{
  server.on("/", HTTP_GET, restIndex);
  server.on("/index.html", HTTP_GET, restIndex);
  server.on("/display", HTTP_GET, restDisplay);
  server.on("/display", HTTP_POST, restDisplay);
  server.on("/getState", HTTP_GET, restGetState);
  server.on("/getLightSensorReading", HTTP_GET, restGetLightSensorReading);
  server.on("/restart", HTTP_GET, restRestart);
  server.on("/syncTime", HTTP_GET, restSyncTime);
  server.on("/setLightSensorThreshold", HTTP_GET, restSetLightSensorThreshold);
  server.on("/setTransitionType", HTTP_GET, restSetTransitionType);

  server.on("/enableAnimationDay", HTTP_GET,
            []()
            { restSetAnimationState(true, false); });
  server.on("/disableAnimationDay", HTTP_GET,
            []()
            { restSetAnimationState(false, false); });
  server.on("/setDayBrightness", HTTP_GET,
            []()
            { restSetBrightness(&dayBrightness, "Day"); });
  server.on("/setDayMaxSpeed", HTTP_GET,
            []()
            { restSetSpeedFactor(&animationDaySpeedFactor, "Day"); });
  server.on("/setDayTransitionTime", HTTP_GET,
            []()
            { restSetTransitionTime(&dayTransitionTime, "Day"); });

  server.on("/enableAnimationNight", HTTP_GET,
            []()
            { restSetAnimationState(true, true); });
  server.on("/setNightMaxSpeed", HTTP_GET,
            []()
            { restSetSpeedFactor(&animationNightSpeedFactor, "Night"); });
  server.on("/disableAnimationNight", HTTP_GET,
            []()
            { restSetAnimationState(false, true); });
  server.on("/setNightBrightness", HTTP_GET,
            []()
            { restSetBrightness(&nightBrightness, "Night"); });
  server.on("/setNightTransitionTime", HTTP_GET,
            []()
            { restSetTransitionTime(&nightTransitionTime, "Night"); });

  // server.on("/recentLogs", HTTP_GET, restRecentLogs);
  server.begin();

  log_i("REST server running");
}