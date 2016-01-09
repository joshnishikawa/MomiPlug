#include "editor.h"

// Track constructors
Track::Track(){};

Track::Track(int p){
  pinMode(p, INPUT_PULLUP);
  myTrack = new Bounce(p, 10);
  level = 0;
  state = false;
};

// Track destructor
Track::~Track(){
  delete myTrack;
};


// Editor constructors
Editor::Editor(){};

Editor::Editor(int p, MIDIenc& enc){
  pinMode(p, INPUT_PULLUP);
  myButt = new Bounce(p, 10);
  myKnob = enc.myKnob;
  state = false;
  channel = MIDIchannel;
  scene = 102;
  returnme = 0;
};

// Editor destructor
Editor::~Editor(){
  delete myButt;
  delete myKnob;
};

int Editor::read(Track* Ts[], MIDIbutton& FS1, MIDIbutton& FS0){
  int returnme = -1;
  int incdec = myKnob->read();
  myKnob->write(0); // We only need to know which direction the knob goes.

  for(int i=0; i<5; i++){
    if(Ts[i]->state == true){                   // If a track is armed
      if((incdec == 1 && Ts[i]->level < 127) || // and isn't already maxed or
         (incdec == -1 && Ts[i]->level > 0)){   // at 0
        Ts[i]->level += incdec;                 // update track level.
        usbMIDI.sendControlChange(i+102,Ts[i]->level,channel); // and send.
        returnme = Ts[i]->level;
      }
    }
    if(Ts[i]->myTrack->update()){
      if(Ts[i]->myTrack->fallingEdge()){        // Arm or disarm tracks.
        usbMIDI.sendControlChange(i+80,127,channel);
        usbMIDI.sendControlChange(i+80,0,channel);
        Ts[i]->state = !Ts[i]->state;
        returnme = Ts[i]->state == true ? Ts[i]->level : -1; //Show level on arm
      }
    }
  }

  if(FS1.myButt->update()){
    if(FS1.myButt->fallingEdge()){              // For using a single footswitch
      scene = scene == 119 ? 107 : scene += 1 ; // to trigger a groups scenes in
      usbMIDI.sendControlChange(scene,127,channel);// in sequence using control
      usbMIDI.sendControlChange(scene,0,channel);// change numbers 107 ~ 119.
      returnme = scene;                         // 107 should be assigned to
    }                                           // the stop button (as is FS0)
  }                                             // to prevent wraparound
  if(FS0.myButt->update()){
    if(FS0.myButt->fallingEdge()){
      usbMIDI.sendControlChange(107,127,channel);
      usbMIDI.sendControlChange(107,0,channel);
      scene = 107;
      returnme = FS0.number;
    }
  }
  return returnme;
};


////////// TODO EDITING CONTROLLERS //////////

bool Editor::a(){
  return 0;
};

bool Editor::b(){
  return 0;
};

bool Editor::c(){
  return 0;
};

