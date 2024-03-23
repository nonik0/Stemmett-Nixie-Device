#pragma once

#include "animationBase.h"
#include "basicfadeAnimation.h"
#include "bubbleAnimation.h"
#include "cylonScanAnimation.h"
#include "ghostAnimation.h"
#include "nameAnimation.h"
#include "nameGlitchAnimation.h"
#include "pulsarAnimation.h"
#include "rainAnimation.h"
#include "randomScanAnimation.h"
#include "scanAnimation.h"
#include "slotMachineAnimation.h"

#define NUM_ANIMATIONS 11

enum AnimationType {
  Name = 0, // name is primary animation
  BasicFade,
  Bubble,
  CylonScan,
  Ghost,
  NameGlitch,
  Pulsar,
  Rain,
  RandomScan,
  Scan,
  SlotMachine,
  NumAnimations,
  Invalid,
};


enum TransitionBehavior {
  Random,
  Sequential,
};

AnimationType parseAnimationType(String animationType) {
  if (animationType == "BasicFade")
    return BasicFade;
  if (animationType == "Bubble")
    return Bubble;
  if (animationType == "CylonScan")
    return CylonScan;
  if (animationType == "Ghost")
    return Ghost;
  if (animationType == "Name")
    return Name;
  if (animationType == "NameGlitch")
    return NameGlitch;
  if (animationType == "Pulsar")
    return Pulsar;
  if (animationType == "Rain")
    return Rain;
  if (animationType == "RandomScan")
    return RandomScan;
  if (animationType == "Scan")
    return Scan;
  if (animationType == "SlotMachine")
    return SlotMachine;
  return Invalid;
}

// animation ideas:
// slot machine:
// - one cathode active each tube
// - one cathode active all tubes
// - stop sequence:
//   - end left to right each primary cathode
//   - end random tube each priamry cathode