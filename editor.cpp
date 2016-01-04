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

Editor::Editor(int p, int a, int b){
  pinMode(p, INPUT_PULLUP);
  myButt = new Bounce(p, 10);
  myKnob = new Encoder(a, b);
  state = false;
  channel = MOMIchannel;
  scene = 102;
  returnme = 0;
};

// Editor destructor
Editor::~Editor(){
  delete myButt;
  delete myKnob;
};


void Editor::check(MIDIbutton* Bs[], MIDIpot* Ps[], MIDIbutton& FS1, MIDIbutton& FS2){
  if(myButt->update()){
    if (myButt->risingEdge()){
      state = !state;
    //else if(digitalRead(myButt) == LOW){update all inputs}
    }
  }
};

int Editor::read(Track* Ts[], MIDIbutton& FS1, MIDIbutton& FS0){
  int incdec = myKnob->read();
  myKnob->write(0); // We only need to know which direction the knob is going.

  for(int i=0; i<5; i++){
    if(Ts[i]->state == true){                   // If a track is armed
      if((incdec == 1 && Ts[i]->level < 127) || // and isn't already maxed or
         (incdec == -1 && Ts[i]->level > 0)){   // at 0
        Ts[i]->level += incdec;                 // update track level.
        usbMIDI.sendControlChange(i+20,Ts[i]->level,channel); // and send.
        returnme = Ts[i]->level;
      }
    }
    if(Ts[i]->myTrack->update()){
      if(Ts[i]->myTrack->fallingEdge()){        // Arm or disarm tracks.
        usbMIDI.sendControlChange(i+25,127,channel);
        usbMIDI.sendControlChange(i+25,0,channel);
        Ts[i]->state = !Ts[i]->state;
        returnme = Ts[i]->state == true ? Ts[i]->level : 0; // Show level on arm
      }
    }
  }

  if(FS1.myButt->update()){
    if(FS1.myButt->fallingEdge()){              // For using a single footswitch
      scene = scene == 119 ? 102 : scene += 1 ; // to trigger a groups scenes in
      usbMIDI.sendControlChange(scene,127,channel);// in sequence using control
      usbMIDI.sendControlChange(scene,0,channel);// change numbers 102 ~ 119.
      returnme = scene;                         // 102 should be assigned to
    }                                           // the stop button (as is FS0)
  }                                             // to prevent wraparound
  if(FS0.myButt->update()){
    if(FS0.myButt->fallingEdge()){
      usbMIDI.sendControlChange(102,127,channel);
      usbMIDI.sendControlChange(102,0,channel);
      scene = 102;
      returnme = 15;
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

