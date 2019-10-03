#include "track.h"

// Track constructors
Track::Track() : Flicker(0, 0){};

Track::Track(int p, byte n, int thresh) : Flicker(p, thresh){
  number = n;
  level = 0;
  state = false;
};

// Track destructor
Track::~Track(){};

int Track::send(){
  int returnme = -1;
	Flicker::update();
  if(Flicker::risingEdge()){ // Arm or disarm tracks.
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
      usbMIDI.sendControlChange(number+3,level,MIDIchannel); // & send.
      returnme = level;
    }
  }
  return returnme;
};

bool record(bool rec, bool stp){
  static int scene = 111;
  if(rec){ // uses CC# 111~119 to trigger scenes
    scene = scene == 119 ? 111 : scene + 1;
    usbMIDI.sendControlChange(scene,127,MIDIchannel);
    usbMIDI.sendControlChange(scene,0,MIDIchannel);
  }
  if(stp){ // uses CC#111 to stop rec and reset the cycle
    usbMIDI.sendControlChange(111,127,MIDIchannel);
    usbMIDI.sendControlChange(111,0,MIDIchannel);
    scene = 111;
  }
  if(scene > 111){return true;}else{return false;}
};


