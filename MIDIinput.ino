#include "MIDIinput.h"
int value = 0;
elapsedMillis timer = 0;
byte waiting = false;
unsigned int waitTime;
byte touched = false;

byte MIDIchord[12] = {
  false,false,false,false,false,false,false,false,false,false,false,false
};
byte USBchord[12] = {
  false,false,false,false,false,false,false,false,false,false,false,false
};

byte chaos(byte pin, uint16_t newValue, uint16_t inLo, uint16_t inHi){
  newValue = map(newValue, inLo, inHi, 0, 127);
  newValue = constrain(newValue, 0, 127);

  if (waiting){ // Wait briefly to make notes audible.
    if (timer > waitTime){
      waiting = false;
    }
  }
  else if (newValue > 0){ // send MIDI
    if ( (MIDIchord[newValue % 12] == true || USBchord[newValue % 12] == true) && newValue != value ){
      usbMIDI.sendNoteOn(value, 0, MIDIchannel); // we don't want TOTAL chaos
      usbMIDI.sendNoteOn(newValue, 96, MIDIchannel);
      waitTime = 150 - newValue; // Hold note longer for lower notes; shorter for higher.
      value = newValue;
      timer = 0;
      waiting = true;
      digitalWrite(pin, HIGH);
    }
    else if (newValue == 0){
      for(int i=0; i<12; i++){
        usbMIDI.sendNoteOn(MIDIchord[i], 0, MIDIchannel);
        usbMIDI.sendNoteOn(USBchord[i], 0, MIDIchannel);
      }
      digitalWrite(pin, LOW);
    }
    else{digitalWrite(pin, LOW);}
  }
  return newValue;
};

//MIDI EVENT HANDLERS///////////////////////////////////////////////////////////
void onNoteOff(byte channel, byte note, byte velocity){
  usbMIDI.sendNoteOff(note, 0, channel);
  usbMIDI.sendNoteOff(note, 0, channel, 2); // send to separate ports
  MIDIchord[note % 12] = false;
}
  
void onNoteOn(byte channel, byte note, byte velocity){
  usbMIDI.sendNoteOn(note, velocity, channel);
  usbMIDI.sendNoteOn(note, velocity, channel, 2); // send to separate ports
  if (velocity == 0){
    MIDIchord[note % 12] = false;    
  }
  else {MIDIchord[note % 12] = true;}
}

void onPolyPressure(byte channel, byte note, byte pressure){
	usbMIDI.sendPolyPressure(note, pressure, channel);
  usbMIDI.sendPolyPressure(note, pressure, channel, 2);
}

void onControl(byte channel, byte control, byte value){
	usbMIDI.sendControlChange(control, value, channel);
  usbMIDI.sendControlChange(control, value, channel, 2);
}

void onProgram(byte channel, byte program){
	usbMIDI.sendProgramChange(program, channel);
  usbMIDI.sendProgramChange(program, channel, 2);
}

void onAfterTouch(byte channel, byte pressure){
	usbMIDI.sendAfterTouch(pressure, channel);
  usbMIDI.sendAfterTouch(pressure, channel, 2);
}

void onPitchBend(byte channel, int bend){
	usbMIDI.sendPitchBend(bend, channel);
  usbMIDI.sendPitchBend(bend, channel, 2);
}

//USB MIDI EVENT HANDLERS///////////////////////////////////////////////////////
void onUSBNoteOff(byte channel, byte note, byte velocity){
  usbMIDI.sendNoteOff(note, 0, channel, 1); // send to separate ports
  usbMIDI.sendNoteOff(note, 0, channel, 3); // send to separate ports
  MIDI.sendNoteOff(note, 0, channel);
  USBchord[note % 12] = false;
}
  
void onUSBNoteOn(byte channel, byte note, byte velocity){
  usbMIDI.sendNoteOn(note, velocity, channel, 1); // send to separate ports
  usbMIDI.sendNoteOn(note, velocity, channel, 3); // send to separate ports
  MIDI.sendNoteOn(note, velocity, channel);
  if (velocity == 0){
    USBchord[note % 12] = false;    
  }
  else {USBchord[note % 12] = true;}
}

void onUSBPolyPressure(byte channel, byte note, byte pressure){
  usbMIDI.sendPolyPressure(note, pressure, channel, 1);
  usbMIDI.sendPolyPressure(note, pressure, channel, 3);
  MIDI.sendPolyPressure(note, pressure, channel);
}

void onUSBControl(byte channel, byte control, byte value){
  usbMIDI.sendControlChange(control, value, channel, 1);
  usbMIDI.sendControlChange(control, value, channel, 3);
  MIDI.sendControlChange(control, value, channel);
}

void onUSBProgram(byte channel, byte program){
  usbMIDI.sendProgramChange(program, channel, 1);
  usbMIDI.sendProgramChange(program, channel, 3);
  MIDI.sendProgramChange(program, channel);
}

void onUSBAfterTouch(byte channel, byte pressure){
  usbMIDI.sendAfterTouch(pressure, channel, 1);
  usbMIDI.sendAfterTouch(pressure, channel, 3);
  MIDI.sendAfterTouch(pressure, channel);
}

void onUSBPitchBend(byte channel, int bend){
  usbMIDI.sendPitchBend(bend, channel, 1);
  usbMIDI.sendPitchBend(bend, channel, 3);
  MIDI.sendPitchBend(bend, channel);
}
