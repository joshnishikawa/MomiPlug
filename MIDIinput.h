#ifndef MIDIinput_h
#define MIDIinput_h

#include "Arduino.h"
extern byte MIDIchannel;
int chaos(byte p, int upperLimit, int onThreshold, int offThreshold, byte outLo, byte outHi);
void outputRange(byte min, byte max);

void onNoteOff(byte channel, byte note, byte velocity);
void onNoteOn(byte channel, byte note, byte velocity);
void onPolyPressure(byte channel, byte note, byte pressure);
void onControl(byte channel, byte control, byte value);
void onProgram(byte channel, byte program);
void onAfterTouch(byte channel, byte pressure);
void onPitchBend(byte channel, int bend);

//void onUSBNoteOff(byte channel, byte note, byte velocity);
//void onUSBNoteOn(byte channel, byte note, byte velocity);
//void onUSBPolyPressure(byte channel, byte note, byte pressure);
//void onUSBControl(byte channel, byte control, byte value);
//void onUSBProgram(byte channel, byte program);
//void onUSBAfterTouch(byte channel, byte pressure);
//void onUSBPitchBend(byte channel, int bend);

#endif

