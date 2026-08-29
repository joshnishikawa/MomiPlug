#ifndef MIDIinput_h
#define MIDIinput_h

#include "Arduino.h"

extern byte MIDIchannel;

byte chaos(byte pin, uint16_t newValue, uint16_t inLo, uint16_t inHi, uint16_t outLo, uint16_t outHi);

// DIN MIDI Event Handlers
void onNoteOff(byte channel, byte note, byte velocity);
void onNoteOn(byte channel, byte note, byte velocity);
void onPolyPressure(byte channel, byte note, byte pressure);
void onControl(byte channel, byte control, byte value);
void onProgram(byte channel, byte program);
void onAfterTouch(byte channel, byte pressure);
void onPitchBend(byte channel, int bend);

// USB Host MIDI Event Handlers
void onUSBNoteOff(byte channel, byte note, byte velocity);
void onUSBNoteOn(byte channel, byte note, byte velocity);
void onUSBPolyPressure(byte channel, byte note, byte pressure);
void onUSBControl(byte channel, byte control, byte value);
void onUSBProgram(byte channel, byte program);
void onUSBAfterTouch(byte channel, byte pressure);
void onUSBPitchBend(byte channel, int bend);

#endif
