// #include "animation.h"
// #include "tubes.h"

// bool Animation::handleTick() {
//   _duration--;
// }

// bool Animation::isComplete() {
//   return _duration < 0;
// }

// bool SlotMachineAnimation::handleTick() {
//   Animation::handleTick();

//   bool update = false;
//   _delay--;

//   if (_delay < 0) {
//     for (int i = 0; i < NUM_TUBES; i++) {
//       Tubes[i].ActiveCathode = (Tubes[i].ActiveCathode + 1) % TubeCathodeCount[Tubes[i].Type];
//     }
//   }

//   return update;
// }