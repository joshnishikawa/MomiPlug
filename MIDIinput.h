#ifndef MIDIinput_h
#define MIDIinput_h

#include "Arduino.h"
int chaos(byte p, int inLo, int threshold, int inHi, byte outLo, byte outHi);

void outputRange(byte min, byte max);
void onNoteOff(byte channel, byte note, byte velocity);
void onNoteOn(byte channel, byte note, byte velocity);
void onPolyPressure(byte channel, byte note, byte pressure);
void onControl(byte channel, byte control, byte value);
void onProgram(byte channel, byte program);
void onAfterTouch(byte channel, byte pressure);
void onPitchBend(byte channel, int bend);

#endif

