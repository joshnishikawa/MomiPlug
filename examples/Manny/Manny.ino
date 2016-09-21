#include "MOMIPLUG.h"
const int touchPin = 15;

void setup(){
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOff(onNoteOff);
  MIDI.setHandleNoteOn(onNoteOn);
  MIDI.setHandleAfterTouchPoly(onPolyPressure);
  MIDI.setHandleControlChange(onControl);
  MIDI.setHandleProgramChange(onProgram);
  MIDI.setHandleAfterTouchChannel(onAfterTouch);
  MIDI.setHandlePitchBend(onPitchBend);
}

void loop(){
  MIDI.read();    
  chaos(touchPin, 930, 1300, 2500, 36, 107);
}
