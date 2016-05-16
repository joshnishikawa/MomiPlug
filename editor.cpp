#include "editor.h"

// Editor constructors
Editor::Editor(){};

Editor::Editor(int p, Encoder* enc){
  pinMode(p, INPUT_PULLUP);
  myButt = new Bounce(p, 10);
  myKnob = enc;
  target = 0;
  targetSize = 0;
  editing = false;
};

// Editor destructor
Editor::~Editor(){
  delete myButt;
};


void Editor::edit(){
  switch (targetSize){
    case 0:
      editInput();
      break;
    case sizeof(MIDIbutton):
      MIDIbutton* bTarget;
      bTarget = (MIDIbutton*)target;
      editButton(*bTarget);
      break;
    case sizeof(MIDIpot):
      MIDIpot* pTarget;
      pTarget = (MIDIpot*)target;
      editPot(*pTarget);
      break;
    case sizeof(MIDInote):
      MIDInote* nTarget;
      nTarget = (MIDInote*)target;
      editNote(*nTarget);
      break;
    default:
      break;
  }
};

int Editor::editChannel(Encoder& e, int c){
  if (e.read() == 4 && c < 15){
    editing = true;
    e.write(0);
    return c + 1;
  }
  else if (e.read() == -4 && c > 1){
    editing = true;
    e.write(0);
    return c - 1;
  }
  else if (e.read() > 4 || e.read() < -4){
    e.write(0);
    return c;
  }
  else {return c;}
};

void Editor::editInput(){
  editing = false;
};

void Editor::editButton(MIDIbutton b){
  Serial.print("BUTTON ");
  targetSize = 0;
  editing = false;
};

void Editor::editPot(MIDIpot p){
  Serial.print("POT ");
  targetSize = 0;
  editing = false;
};

void Editor::editNote(MIDInote n){
  Serial.print("NOTE ");
  targetSize = 0;
  editing = false;
};

