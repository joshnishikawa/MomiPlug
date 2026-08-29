#ifndef editor_h
#define editor_h

#include "Arduino.h"
#include "SevSeg.h"
#include "MIDIcontroller.h"

extern byte MIDIchannel;
extern SevSeg DSP;

class Editor {
    int quadOne(byte val, byte minVal, byte maxVal);
    char DSPstring[5];
  public:
    Editor();
    Editor(int a, int b, int p);
    ~Editor();
    Bounce* bounce;
    Encoder* encoder;
    int number;
    int level;
    byte editAnalogInputRange;
    byte DP;
    int newInLo, newInHi;
    byte editChannel(byte currentChannel);
    byte setAnalog(int p);
    int send();
};

#endif
