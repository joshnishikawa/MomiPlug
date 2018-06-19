#include "editor.h"

// Editor constructors
Editor::Editor(){};

Editor::Editor(int a, int b, int p){
  pinMode(p, INPUT_PULLUP);
  myButt = new Bounce(p, 50);
  myKnob = new Encoder(a, b);
  number = 3;
  level = 0;
  bTarget = 0;
  pTarget = 0;
  storageIndex = 0;
  editing = false;
};

// Editor destructor
Editor::~Editor(){
  delete myButt;
  delete myKnob;
};

int Editor::send(){/////////////////////////////////////////////////////////////
  int newLevel = -1;
  int incdec = myKnob->read(); // Using only +1 or -1
  if((incdec >= 1 && level < 127) || (incdec <= -1 && level > 0)){
    // If turned up but not already maxed OR down but not already bottomed out
    newLevel = level + incdec;  // and return new value
    level = newLevel;
    usbMIDI.sendControlChange(number, level, MIDIchannel);
    myKnob->write(0);           // then reset to 0
  }
  else{newLevel = -1;}
  return newLevel;
};

int Editor::quadOne(byte val, byte max){
  int newValue = myKnob->read();
  if (newValue == 4){
    if (val < max){
      myKnob->write(0);
      newValue = val + 1;
    }
    else if (val == max){
      myKnob->write(0);
      newValue = 0;
    }
  }
  else if (newValue == -4){
    if (val > 0){
      myKnob->write(0);
      newValue = val - 1;
    }
    else if (val == 0){
      myKnob->write(0);
      newValue = max;
    }
  }
  else if (newValue > 4 || newValue < -4){
    myKnob->write(0);
    newValue = -1;
  }
  else{newValue = -1;}
  return newValue;
};

byte Editor::editChannel(){/////////////////////////////////////////////////////
  int newValue = quadOne(MIDIchannel, 15);
  if (newValue >= 0){
    return newValue;
  }
  else{return MIDIchannel;}
};

void Editor::editButton(MIDIbutton& bt){/////////////////////////////////////////
  bool done = false;
  int tempQuad = -1;
  byte newNumber = bt.number;
  byte newOutHi = bt.outHi;
  byte newOutLo = bt.outLo;
  byte newMode = bt.mode;

  DSP.states(0,0,0,0,0,0,0,0);
  while(!done){
    myButt->update();
    if(myButt->risingEdge()){
      done = true;
    }
    bt.myButt->update();
    if(bt.myButt->read() == HIGH){
      tempQuad = quadOne(newNumber, 127);
      if (tempQuad >= 0){
        newNumber = tempQuad;
      }
      DSP.value(newNumber);
    }
    else{
      tempQuad = quadOne(newOutHi, 127);
      if (tempQuad >= 0){
        newOutHi = tempQuad;
      }
      DSP.value(newOutHi);
    }
    DSP.blink(1);
    DSP.print();
  }
  DSP.states(0,0,0,0,0,0,0,0);
  done = false;
  while(!done){
    myButt->update();
    if(myButt->risingEdge()){
      done = true;
    }
    bt.myButt->update();
    if (bt.myButt->read() == HIGH){
      tempQuad = quadOne(bt.mode, 2);
      if(tempQuad == 0){
        newMode = 0;
      }
      else if(tempQuad == 1){
        newMode = 1;
      }
      else if(tempQuad == 2){
        newMode = 2;
      }
      if(newMode == 0){
        DSP.verbal("Hld");
      }
      else if(newMode == 1){
        DSP.verbal("Lch");
      }
      else if(newMode == 2){
        DSP.verbal("INS");
      }
    }
    else{
      tempQuad = quadOne(newOutLo, 127);
      if (tempQuad >= 0){
        newOutLo = tempQuad;
      }
      DSP.value(newOutLo);
    }
    DSP.blink(2);
    DSP.print();
  }
  if (newNumber != bt.number){
    bt.number = newNumber;
    EEPROM.put(storageIndex, bt.number);
  }
  if (newOutHi != bt.outHi){
    bt.outHi = newOutHi;
    EEPROM.put(storageIndex + 2, bt.outHi);
  }
  if (newMode != bt.mode){
    bt.mode = newMode;
    EEPROM.put(storageIndex + 3, bt.mode);
  }
  if (newOutLo != bt.outLo){
    bt.outLo = newOutLo;
    EEPROM.put(storageIndex + 1, bt.outLo);
  }
  storageIndex = 0;
  return;
};

void Editor::editPot(MIDIpot& pt){///////////////////////////////////////////////
  bool done = false;
  int tempQuad = -1;
  byte newNumber = pt.number;
  byte newOutHi = pt.outHi;
  byte newOutLo = pt.outLo;
  byte newMode = pt.mode;
  int value = -1;
  
  DSP.value(pt.number);
  DSP.states(0,0,0,0,0,0,0,0);
  while(!done){
    myButt->update();
    if(myButt->risingEdge()){
      done = true;
    }
    tempQuad = quadOne(newNumber, 127);
    if (tempQuad >= 0){
      newNumber = tempQuad;
      DSP.value(newNumber);
    }

    int newValue = analogRead(pt.pin);
    if (newValue >= pt.inHi){
      newValue = 127;
    }
    else if (newValue <= pt.inLo){
      newValue = 0;
    }
    else if (newValue % 8 == 0){
      newValue = map(newValue, pt.inLo, pt.inHi, 0, 127);
      newValue = pt.invert ? constrain(newValue, 127, 0)
                           : constrain(newValue, 0, 127);
    }
    else if (newValue == value){
      newValue = -1;
    }
    else{newValue = -1;
    }
    if (newValue >= 0){
      newOutHi = newValue;
      DSP.value(newOutHi);
    }
    DSP.blink(1);
    DSP.print();
  }
  value = -1;
  DSP.value(pt.outLo);
  DSP.states(0,0,0,0,0,0,0,0);
  done = false;
  while(!done){
    myButt->update();
    if(myButt->risingEdge()){
      done = true;
    }
    tempQuad = quadOne(newMode, 1);
    if(tempQuad == 0){
      newMode = 0;
      DSP.verbal("off");
    }
    else if(tempQuad == 1){
      newMode = 1;
      DSP.verbal("but");
    }
    int newValue = analogRead(pt.pin);
    if (newValue >= pt.inHi){
      newValue = 127;
    }
    else if (newValue <= pt.inLo){
      newValue = 0;
    }
    else if (newValue % 8 == 0){
      newValue = map(newValue, pt.inLo, pt.inHi, 0, 127);
      newValue = pt.invert ? constrain(newValue, 127, 0)
                           : constrain(newValue, 0, 127);
    }
    else if (newValue == value){
      newValue = -1;
    }
    else{newValue = -1;
    }
    if (newValue >= 0){
      newOutLo = newValue;
      DSP.value(newOutLo);
    }
    DSP.blink(2);
    DSP.print();
  }
  
  if (newNumber != pt.number){
    pt.number = newNumber;
    EEPROM.put(storageIndex, pt.number);
  }
  if (newMode != pt.mode){
    pt.mode = newMode;
    EEPROM.put(storageIndex + 3, pt.mode);
  }
  if (newOutHi != pt.outHi && newOutHi != newOutLo){
    pt.outputRange(pt.outLo, newOutHi);
    EEPROM.put(storageIndex + 2, pt.outHi);
  }
  if (newOutLo != pt.outLo && newOutHi != newOutLo){
    pt.outputRange(newOutLo, pt.outHi);
    EEPROM.put(storageIndex + 1, pt.outLo);
  }
  storageIndex = 0;
  return;
};

