#include "track.h"

// Track constructors
Track::Track(){};

Track::Track(int p, byte n){
  pinMode(p, INPUT_PULLUP);
  myTrack = new Bounce(p, 50);
//  scene = 107;
  number = n;
  level = 0;
  state = false;
};

// Track destructor
Track::~Track(){
  delete myTrack;
};

int Track::send(){
  int returnme = -1;
  myTrack->update();
  if(myTrack->fallingEdge()){              // Arm or disarm tracks.
    usbMIDI.sendControlChange(number,127,MIDIchannel);
    usbMIDI.sendControlChange(number,0,MIDIchannel);
    state = !state;
    returnme = state == true ? level : 0; //Show level on arm
  }
  return returnme;
};

int Track::vol(int incdec){
  int returnme = -1;
  if(state == true && incdec != 0){        // If the track is armed
    if((incdec == 1 && level < 127) ||     // and isn't already maxed or
       (incdec == -1 && level > 0)){       // already at 0
      level += incdec;                     // update track level.
      usbMIDI.sendControlChange(number+22,level,MIDIchannel); // & send.
      returnme = level;
    }
  }
  return returnme;
};

/*
void record(Bounce rec, Bounce stop, Track& Ts[]){
  rec->update();
  if(rec->fallingEdge()){                     // For using a single footswitch
    scene = scene == 119 ? 107 : scene + 1;   // to trigger a groups scenes in
    usbMIDI.sendControlChange(scene,127,MIDIchannel); // in sequence using control
    usbMIDI.sendControlChange(scene,0,MIDIchannel);   // change numbers 107 ~ 119.
    returnme = scene;                         // 107 should be assigned to
  }                                           // the stop button (as is FS0)
                                              // to prevent wraparound
  stop->update();
  if(stop->fallingEdge()){
    usbMIDI.sendControlChange(107,127,MIDIchannel);
    usbMIDI.sendControlChange(107,0,MIDIchannel);
    scene = 107;
    returnme = FS0.number;
  }
  return returnme;
};
*/