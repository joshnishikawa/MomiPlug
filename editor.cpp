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
  channel = MIDIchannel;
  returnme = 0;
};

// Editor destructor
Editor::~Editor(){
  delete myButt;
  delete myKnob;
};


void Editor::read(MIDIbutton* Bs[], MIDIpot* Ps[], MIDIbutton& FS1, MIDIbutton& FS2){
  if(myButt->update()){
    if (myButt->risingEdge()){
      state = !state;
    //else if(digitalRead(myButt) == LOW){update all inputs}
    }
  }
};

int Editor::tracks(Track* Ts[], MIDIbutton& FS1){
  bool rec = false;
  if(FS1.myButt->update()){
    if(FS1.myButt->fallingEdge()){
      rec = true;
    }
  }

  int incdec = myKnob->read();
  myKnob->write(0);

  for(int i=0; i<5; i++){
    if(Ts[i]->state == true){
      if(rec == true){
        usbMIDI.sendControlChange(i+105,127,channel);
        usbMIDI.sendControlChange(i+105,0,channel);
        returnme = i+105;
      }
      if((incdec == 1 && Ts[i]->level < 127) ||
         (incdec == -1 && Ts[i]->level > 0)){
        Ts[i]->level += incdec;
        usbMIDI.sendControlChange(i+110,Ts[i]->level,channel);
        returnme = Ts[i]->level;
      }
    }
    if(Ts[i]->myTrack->update()){
      if(Ts[i]->myTrack->fallingEdge()){
        usbMIDI.sendControlChange(i+115,127,channel);
        usbMIDI.sendControlChange(i+115,0,channel);
        Ts[i]->state = !Ts[i]->state;
        returnme = Ts[i]->state == true ? Ts[i]->level : 0 ;
      }
      //else if(Ts[i]->myTrack->risingEdge()){for multi-assigning buttons}
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

