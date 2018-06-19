#include "editor.h"

// Editor constructors
Editor::Editor(){};

Editor::Editor(int a, int b, int p){
  pinMode(p, INPUT_PULLUP);
  bounce = new Bounce(p, 50);
  encoder = new Encoder(a, b);
  number = 3;
  level = 0;
  outHi = 127;
  outLo = 0;
  bTarget = 0;
  pTarget = 0;
  storageIndex = 0;
  editing = false;
  DP = 0;
};

// Editor destructor
Editor::~Editor(){
  delete bounce;
  delete encoder;
};

int Editor::send(){/////////////////////////////////////////////////////////////
  int incdec = encoder->read();
  if(incdec >= 1 && level < outHi){       // If turned up but not already maxed
    level += 1;
		incdec = level;
    usbMIDI.sendControlChange(number, level, MIDIchannel);
  }
  else if (incdec <= -1 && level > outLo){// If turned down but not bottomed out
    level -= 1;
		incdec = level;
    usbMIDI.sendControlChange(number, level, MIDIchannel);
  }
  else{incdec = -1;}
  encoder->write(0);
  return incdec;
};

int Editor::quadOne(byte val, byte max){
  int newValue = encoder->read();
  if (newValue == 4){
    if (val < max){
      encoder->write(0);
      newValue = val + 1;
    }
    else if (val == max){
      encoder->write(0);
      newValue = 0;
    }
  }
  else if (newValue == -4){
    if (val > 0){
      encoder->write(0);
      newValue = val - 1;
    }
    else if (val == 0){
      encoder->write(0);
      newValue = max;
    }
  }
  else if (newValue > 4 || newValue < -4){
    encoder->write(0);
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

void Editor::editButton(MIDIbutton& bt){////////////////////////////////////////
  byte newNumber = bt.number;
  byte newOutHi = bt.outHi;
  byte newOutLo = bt.outLo;
  byte newMode = bt.mode;
  int tempQuad = -1;

  sprintf(DSPstring, "%4d", bt.number);
  DP = 8;
  bool done = false;
  while(!done){
    bounce->update();
    if(bounce->risingEdge()){
      done = true;
    }
    tempQuad = quadOne(newNumber, 127);
    if (tempQuad >= 0){
      newNumber = tempQuad;
    }
    sprintf(DSPstring, "%4d", newNumber);
    DSP.DisplayString(DSPstring, DP);
  }

  sprintf(DSPstring, "%4d", bt.outHi);
  DP = 12;
  done = false;
  while(!done){
    bounce->update();
    if(bounce->risingEdge()){
      done = true;
    }  
    tempQuad = quadOne(newOutHi, 127);
    if (tempQuad >= 0){
      newOutHi = tempQuad;
    }
    sprintf(DSPstring, "%4d", newOutHi);
    DSP.DisplayString(DSPstring, DP);
  }  
  
  sprintf(DSPstring, "%4d", bt.outLo);
  DP = 14;
  done = false;
  while(!done){
    bounce->update();
    if(bounce->risingEdge()){
      done = true;
    }
    tempQuad = quadOne(newOutLo, 127);
    if (tempQuad >= 0){
      newOutLo = tempQuad;
    }
    sprintf(DSPstring, "%4d", newOutLo);
    DSP.DisplayString(DSPstring, DP);
  }
  
  strcpy(DSPstring, "    ");
  DP = 15;
  done = false;
  while(!done){
    bounce->update();
    if(bounce->risingEdge()){
      done = true;
    }
    tempQuad = quadOne(newMode, 2);
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
      strcpy(DSPstring, "hold");
    }
    else if(newMode == 1){
      strcpy(DSPstring, "lach");
    }
    else if(newMode == 2){
      strcpy(DSPstring, "onon");
    }
    DSP.DisplayString(DSPstring, DP);
  }

  strcpy(DSPstring, "    ");
  if (newNumber != bt.number){
    bt.number = newNumber;
    EEPROM.put(storageIndex, bt.number);
  }
  if (newMode != bt.mode){
    bt.mode = newMode;
    EEPROM.put(storageIndex + 1, bt.mode);
  }
  if (newOutLo != bt.outLo){
    bt.outLo = newOutLo;
    EEPROM.put(storageIndex + 2, bt.outLo);
  }
  if (newOutHi != bt.outHi){
    bt.outHi = newOutHi;
    EEPROM.put(storageIndex + 3, bt.outHi);
  }
  storageIndex = 0;
  return;
};

void Editor::editPot(MIDIpot& pt){///////////////////////////////////////////////
  byte newNumber = pt.number;
  byte newOutHi = pt.outHi;
  byte newOutLo = pt.outLo;
  byte newMode = pt.mode;
  int tempQuad = -1;
  
  sprintf(DSPstring, "%4d", pt.number);
  DP = 8;
  bool done = false;
  while(!done){
    bounce->update();
    if(bounce->risingEdge()){
      done = true;
    }
    tempQuad = quadOne(newNumber, 127);
    if (tempQuad >= 0){
      newNumber = tempQuad;
    }
    sprintf(DSPstring, "%4d", newNumber);
    DSP.DisplayString(DSPstring, DP);
  }

  sprintf(DSPstring, "%4d", pt.outHi);
  DP = 12;
  done = false;
  while(!done){
    bounce->update();
    if(bounce->risingEdge()){
      done = true;
    }
    tempQuad = quadOne(newOutHi, 127);
    if (tempQuad >= 0){
      newOutHi = tempQuad;
    }
    sprintf(DSPstring, "%4d", newOutHi);
    DSP.DisplayString(DSPstring, DP);
  }

  sprintf(DSPstring, "%4d", pt.outLo);
  DP = 14;
  done = false;
  while(!done){
    bounce->update();
    if(bounce->risingEdge()){
      done = true;
    }
    tempQuad = quadOne(newOutLo, 127);
    if (tempQuad >= 0){
      newOutLo = tempQuad;
    }
    sprintf(DSPstring, "%4d", newOutLo);
    DSP.DisplayString(DSPstring, DP);
  }

  sprintf(DSPstring, "%4d", pt.mode);
  DP = 15;
  done = false;
  int newHi = analogRead(pt.pin);
  int newLo = newHi;
  while(!done){
    bounce->update();
    if(bounce->risingEdge()){
      done = true;
    }
    
    int newVal = analogRead(pt.pin);
    if (newVal > newHi){
      newHi = newVal;
      Serial.print("High: "); Serial.println(newHi);
      Serial.print("Low : "); Serial.println(newLo);
    }
    else if (newVal < newLo){
      newLo = newVal;
      Serial.print("High: "); Serial.println(newHi);
      Serial.print("Low : "); Serial.println(newLo);
    }

    tempQuad = quadOne(newMode, 1);
    if(tempQuad == 0){
      newMode = 0;
      strcpy(DSPstring, " off");
    }
    else if(tempQuad == 1){
      newMode = 1;
      strcpy(DSPstring, "butn");
    }
    DSP.DisplayString(DSPstring, DP);
  }

  strcpy(DSPstring, "    ");
  if (newNumber != pt.number){
    pt.number = newNumber;
    EEPROM.put(storageIndex, pt.number);
  }
  if (newMode != pt.mode){
    pt.mode = newMode;
    EEPROM.put(storageIndex + 1, pt.mode);
  }
  if (newOutLo != pt.outLo && newOutHi != newOutLo){
    pt.outputRange(newOutLo, pt.outHi);
    EEPROM.put(storageIndex + 2, pt.outLo);
  }
  if (newOutHi != pt.outHi && newOutHi != newOutLo){
    pt.outputRange(pt.outLo, newOutHi);
    EEPROM.put(storageIndex + 3, pt.outHi);
  }
  if (newHi - newLo > 127){
    pt.inputRange(newLo, newHi);
    byte lowByte = ((newLo >> 0) & 0xFF); //FIXME: Can't this use eeprom_put_word() from MOMIPLUG.h ?
    byte highByte = ((newLo >> 8) & 0xFF);
    EEPROM.put(storageIndex + 256, lowByte);
    EEPROM.put(storageIndex + 257, highByte);
    lowByte = ((newHi >> 0) & 0xFF);      //FIXME: Can't this also use eeprom_put_word() from MOMIPLUG.h ?
    highByte = ((newHi >> 8) & 0xFF);
    EEPROM.put(storageIndex + 258, lowByte);
    EEPROM.put(storageIndex + 259, highByte);
  }
  storageIndex = 0;
  return;
};



/*

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
    else{newValue = -1;
    }

        if (newValue >= 0){
      newOutHi = newValue;
      sprintf(DSPstring, "%4d", newOutHi);
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
      sprintf(DSPstring, "%4d", newOutLo);
    }

*/

