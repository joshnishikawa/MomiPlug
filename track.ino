#include "track.h"

// Track constructors
Track::Track() : number(0), level(0), state(false) {}

Track::Track(byte n) : number(n), level(0), state(false) {}

// Track destructor
Track::~Track() {}

int Track::toggleArm() {
  state = !state;
  usbMIDI.sendControlChange(number, 127, MIDIchannel);
  usbMIDI.sendControlChange(number, 0, MIDIchannel);
  return state ? level : 0;
}

int Track::vol(int incdec) {
  int returnme = -1;
  if (state && incdec != 0) { // If the track is armed
    int step = (incdec > 0) ? 1 : -1;
    if ((step > 0 && level < 127) || (step < 0 && level > 0)) {
      level = (byte)constrain(level + step, 0, 127);
      usbMIDI.sendControlChange(number + 3, level, MIDIchannel);
      returnme = level;
    }
  }
  return returnme;
}

byte record(byte rec, byte stp) {
  static int scene = 111;
  if (rec) { // uses CC# 111~119 to trigger scenes
    scene = (scene >= 119) ? 111 : scene + 1;
    usbMIDI.sendControlChange(scene, 127, MIDIchannel);
    usbMIDI.sendControlChange(scene, 0, MIDIchannel);
  }
  if (stp) { // uses CC#111 to stop rec and reset the cycle
    usbMIDI.sendControlChange(111, 127, MIDIchannel);
    usbMIDI.sendControlChange(111, 0, MIDIchannel);
    scene = 111;
  }
  return (scene > 111) ? 1 : 0;
}
