#ifndef MIDIinput_h
#define MIDIinput_h

#include "Arduino.h"

extern byte MIDIchannel;

class MIDIinput{
    bool waiting;
    unsigned int waitTime;
    unsigned long int timer;
    bool touched;
    
  public:
    MIDIinput();
   	~MIDIinput();

    int chaos();
    int value;
    int hiThreshold, loThreshold;
    void setThresholds(int loT, int hiT);
    byte outLo, outHi;
    void outputRange(byte min, byte max);
};

    void outputRange(byte min, byte max);
    void onNoteOff(byte channel, byte note, byte velocity);
    void onNoteOn(byte channel, byte note, byte velocity);
    void onPolyPressure(byte channel, byte note, byte pressure);
    void onControl(byte channel, byte control, byte value);
    void onProgram(byte channel, byte program);
    void onAfterTouch(byte channel, byte pressure);
    void onPitchBend(byte channel, int bend);


#endif

