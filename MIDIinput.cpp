#include "MIDIinput.h"

// constructors
MIDIinput::MIDIinput(){
  value = 0;
  loThreshold = 1500;
  hiThreshold = 5000;
  outLo = 0;
  outHi = 127;
  waiting = false;
  waitTime = 10; // millis
  timer = 0;
  touched = false;
};


// destructor
MIDIinput::~MIDIinput(){
};


bool chord[12] = {
  false,false,false,false,false,false,false,false,false,false,false,false
};

int MIDIinput::chaos(){
  int newValue = touchRead(18);
  if (waiting){ // Wait briefly to make notes audible.
    if (millis() - timer > waitTime){
      waiting = false;
    }
  }
  else {
    if (newValue > hiThreshold && touched == false){        // rising edge
      touched = true;
      newValue = -1;
    }
    else if (newValue <= loThreshold && touched == true){  // falling edge
      touched = false;
      usbMIDI.sendNoteOn(value, 0, MIDIchannel);
      timer = millis();
      waiting = true;
      newValue = 0;
    }
    else if (newValue > loThreshold && touched == true){   // send MIDI
      newValue = map(newValue, loThreshold, hiThreshold, outLo, outHi);
      newValue = constrain(newValue, outLo, outHi);
      if (chord[newValue % 12] == true && newValue != value){
        usbMIDI.sendNoteOn(value, 0, MIDIchannel); // cuz we don't want TOTAL chaos
        usbMIDI.sendNoteOn(newValue, outHi, MIDIchannel);
        waitTime = 127 - newValue; // not necessary. just adds flavor.
        value = newValue;
        timer = millis();
        waiting = true;
      }
    }
    else {newValue = -1;}
  }
  return newValue;
};

void MIDIinput::outputRange(byte min, byte max){
  outLo = min;
  outHi = max;
};

void MIDIinput::setThresholds(int loT, int hiT){
  loThreshold = loT;
  hiThreshold = hiT;
}

//EVENT HANDLERS////////////////////////////////////////////////////////////////
void onNoteOff(byte channel, byte note, byte velocity){
  usbMIDI.sendNoteOff(note, 0, channel);
  chord[note % 12] = false;
}
  
void onNoteOn(byte channel, byte note, byte velocity){
  usbMIDI.sendNoteOn(note, velocity, channel);
  if (velocity == 0){
    chord[note % 12] = false;    
  }
  else {chord[note % 12] = true;}
}

void onPolyPressure(byte channel, byte note, byte pressure){
	usbMIDI.sendPolyPressure(note, pressure, channel);
}

void onControl(byte channel, byte control, byte value){
	usbMIDI.sendControlChange(control, value, channel);
}

void onProgram(byte channel, byte program){
	usbMIDI.sendProgramChange(program, channel);
}

void onAfterTouch(byte channel, byte pressure){
	usbMIDI.sendAfterTouch(pressure, channel);
}

void onPitchBend(byte channel, int bend){
	usbMIDI.sendPitchBend(bend, channel);
}