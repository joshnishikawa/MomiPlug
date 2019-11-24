#ifndef editor_h
#define editor_h

#include "Arduino.h"
#include "SevSeg.h"
#include "MIDIcontroller.h"
extern byte MIDIchannel;
extern SevSeg DSP;

class Editor{
    int quadOne(byte val, byte max);
    char DSPstring[5];
    byte DP;
  public:
    Editor();
    Editor(int a, int b, int p);
    ~Editor();
    Bounce* bounce;
    Encoder* encoder;
    int number;
    int level;
    byte editing;
    byte editAnalogInputRange;
    int newInLo, newInHi;
    byte editChannel();
    byte setAnalog(int p);
    int send(); //cuz interrupts won't let two objects use the same encoder pins
};
#endif
