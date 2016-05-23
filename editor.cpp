#include "editor.h"

// Editor constructors
Editor::Editor(){};

Editor::Editor(int p, Encoder* enc){
  pinMode(p, INPUT_PULLUP);
  myButt = new Bounce(p, 50);
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
    default:
      target = 0;
      targetSize = 0;
      break;
  }
};

int Editor::editChannel(Encoder& e){
  if (e.read() == 4 && *MC < 15){
    editing = true;
    e.write(0);
    return *MC + 1;
  }
  else if (e.read() == -4 && *MC > 1){
    editing = true;
    e.write(0);
    return *MC - 1;
  }
  else if (e.read() > 4 || e.read() < -4){
    e.write(0);
    return *MC;
  }
  else {return *MC;}
};

void Editor::editInput(){
  editing = false;
};

void Editor::editButton(MIDIbutton& b){
  Serial.print("BUTTON ");
  Serial.print(b.number);
  Serial.print(" ");
  b.number = 37;
  Serial.print(b.number);
  target = 0;
  targetSize = 0;
  editing = false;
};

void Editor::editPot(MIDIpot& p){
  Serial.print("POT ");
  target = 0;
  targetSize = 0;
  editing = false;
};

