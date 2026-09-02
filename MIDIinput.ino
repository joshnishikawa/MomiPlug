#include "MIDIinput.h"
#include <MIDI.h>

ChordAnalyzer analyzer;
ChordDisplay chordDisplay(Wire2, 0x3C, 4, 3);

int value = 0;
elapsedMillis timer = 0;
byte waiting = false;
unsigned int waitTime = 0;
byte touched = false;

byte MIDIchord[12] = {
  false, false, false, false, false, false, false, false, false, false, false, false
};
byte USBchord[12] = {
  false, false, false, false, false, false, false, false, false, false, false, false
};

byte chaos(byte pin, uint16_t rawTouch, uint16_t inLo, uint16_t inHi, uint16_t outLo, uint16_t outHi) {
  if (rawTouch < inLo) {
    if (value > 0) {
      usbMIDI.sendNoteOn(value, 0, MIDIchannel);
      value = 0;
    }
    digitalWrite(pin, LOW);
    waiting = false;
    return 0;
  }

  uint16_t newValue = map(rawTouch, inLo, inHi, outLo, outHi);
  newValue = constrain(newValue, outLo, outHi);

  if (waiting) { // Wait briefly to make notes audible
    if (timer > waitTime) {
      waiting = false;
    }
  }
  else if (newValue > 0) { // send MIDI
    if ((MIDIchord[newValue % 12] || USBchord[newValue % 12]) && newValue != value) {
      if (value > 0) {
        usbMIDI.sendNoteOn(value, 0, MIDIchannel); // Silence previous note
      }
      usbMIDI.sendNoteOn(newValue, 96, MIDIchannel);
      waitTime = (newValue < 150) ? (150 - newValue) : 10; // Hold note longer for lower notes
      value = newValue;
      timer = 0;
      waiting = true;
      digitalWrite(pin, HIGH);
    }
    else {
      digitalWrite(pin, LOW);
    }
  }
  return (byte)newValue;
}

// MIDI EVENT HANDLERS (DIN MIDI IN) ///////////////////////////////////////////
void onNoteOff(byte channel, byte note, byte velocity) {
  analyzer.noteOff(note);

  usbMIDI.sendNoteOff(note, 0, channel);    // Default Port 1 (Cable 0)
  usbMIDI.sendNoteOff(note, 0, channel, 2); // Port 3 (Cable 2)
  MIDIchord[note % 12] = false;
}

void onNoteOn(byte channel, byte note, byte velocity) {
  analyzer.noteOn(note, velocity);

  usbMIDI.sendNoteOn(note, velocity, channel);    // Default Port 1 (Cable 0)
  usbMIDI.sendNoteOn(note, velocity, channel, 2); // Port 3 (Cable 2)
  if (velocity == 0) {
    MIDIchord[note % 12] = false;
  } else {
    MIDIchord[note % 12] = true;
  }
}

void onPolyPressure(byte channel, byte note, byte pressure) {
  usbMIDI.sendPolyPressure(note, pressure, channel);
  usbMIDI.sendPolyPressure(note, pressure, channel, 2);
}

void onControl(byte channel, byte control, byte value) {
  if (control == 64) {analyzer.sustainControl(value);}

  usbMIDI.sendControlChange(control, value, channel);
  usbMIDI.sendControlChange(control, value, channel, 2);
}

void onProgram(byte channel, byte program) {
  usbMIDI.sendProgramChange(program, channel);
  usbMIDI.sendProgramChange(program, channel, 2);
}

void onAfterTouch(byte channel, byte pressure) {
  usbMIDI.sendAfterTouch(pressure, channel);
  usbMIDI.sendAfterTouch(pressure, channel, 2);
}

void onPitchBend(byte channel, int bend) {
  usbMIDI.sendPitchBend(bend, channel);
  usbMIDI.sendPitchBend(bend, channel, 2);
}

// USB HOST MIDI EVENT HANDLERS ///////////////////////////////////////////////
void onUSBNoteOff(byte channel, byte note, byte velocity) {
  usbMIDI.sendNoteOff(note, 0, channel);    // Default Port 1 (Cable 0)
  usbMIDI.sendNoteOff(note, 0, channel, 1); // Port 2 (Cable 1)
  usbMIDI.sendNoteOff(note, 0, channel, 3); // Port 4 (Cable 3)
  MIDI.sendNoteOff(note, 0, channel);       // DIN MIDI Out
  USBchord[note % 12] = false;
}

void onUSBNoteOn(byte channel, byte note, byte velocity) {
  usbMIDI.sendNoteOn(note, velocity, channel);    // Default Port 1 (Cable 0)
  usbMIDI.sendNoteOn(note, velocity, channel, 1); // Port 2 (Cable 1)
  usbMIDI.sendNoteOn(note, velocity, channel, 3); // Port 4 (Cable 3)
  MIDI.sendNoteOn(note, velocity, channel);       // DIN MIDI Out
  if (velocity == 0) {
    USBchord[note % 12] = false;
  } else {
    USBchord[note % 12] = true;
  }
}

void onUSBPolyPressure(byte channel, byte note, byte pressure) {
  usbMIDI.sendPolyPressure(note, pressure, channel);
  usbMIDI.sendPolyPressure(note, pressure, channel, 1);
  usbMIDI.sendPolyPressure(note, pressure, channel, 3);
  MIDI.sendAfterTouch(note, pressure, channel);
}

void onUSBControl(byte channel, byte control, byte value) {
  usbMIDI.sendControlChange(control, value, channel);
  usbMIDI.sendControlChange(control, value, channel, 1);
  usbMIDI.sendControlChange(control, value, channel, 3);
  MIDI.sendControlChange(control, value, channel);
}

void onUSBProgram(byte channel, byte program) {
  usbMIDI.sendProgramChange(program, channel);
  usbMIDI.sendProgramChange(program, channel, 1);
  usbMIDI.sendProgramChange(program, channel, 3);
  MIDI.sendProgramChange(program, channel);
}

void onUSBAfterTouch(byte channel, byte pressure) {
  usbMIDI.sendAfterTouch(pressure, channel);
  usbMIDI.sendAfterTouch(pressure, channel, 1);
  usbMIDI.sendAfterTouch(pressure, channel, 3);
  MIDI.sendAfterTouch(pressure, channel);
}

void onUSBPitchBend(byte channel, int bend) {
  usbMIDI.sendPitchBend(bend, channel);
  usbMIDI.sendPitchBend(bend, channel, 1);
  usbMIDI.sendPitchBend(bend, channel, 3);
  MIDI.sendPitchBend(bend, channel);
}
