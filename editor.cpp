#include "editor.h"

// Track constructors
Track::Track(){};

Track::Track(int p){
  pinMode(p, INPUT_PULLUP);
  bTrak = new Bounce(p, 10);
  level = 0;
  state = false;
};

// Track destructor
Track::~Track(){
  delete bTrak;
};


// Editor constructors
Editor::Editor(){};

Editor::Editor(int p, int a, int b){
  pinMode(p, INPUT_PULLUP);
  bButn = new Bounce(p, 10);
  eKnob = new Encoder(a, b);
  state = false;
  channel = 1;  //FIXME user set channel
  returnme = 0;
};

// Editor destructor
Editor::~Editor(){
  delete bButn;
  delete eKnob;
};


void Editor::Read(Button* Bs[], Pot* Ps[], Button& FS1, Button& FS2){
  if(bButn->update()){
    if (bButn->risingEdge()){
      state = state == true ? false : true;
    //else if(digitalRead(bButn) == LOW){update all inputs}
    }
  }
};

int Editor::Tracks(Track* Ts[], Button& FS1){
  bool rec = false;
  if(FS1.bButn->update()){
    if(FS1.bButn->fallingEdge()){
      rec = true;
    }
  }

  int incdec = eKnob->read();
  eKnob->write(0);

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
    if(Ts[i]->bTrak->update()){
      if(Ts[i]->bTrak->fallingEdge()){
        usbMIDI.sendControlChange(i+115,127,channel);
        usbMIDI.sendControlChange(i+115,0,channel);
        Ts[i]->state = Ts[i]->state == true ? false : true;
        returnme = Ts[i]->state == true ? Ts[i]->level : 0 ;
      }
      //else if(Ts[i]->bTrak->risingEdge()){for multi-assigning buttons}
    }
  }
  return returnme;
};


////////// TODO EDITING CONTROLLERS //////////

bool Editor::A(){
  return 0;
};

bool Editor::B(){
  return 0;
};

bool Editor::C(){
  return 0;
};

