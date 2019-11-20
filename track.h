#ifndef track_h
#define track_h

#include "Arduino.h"
#include "Flicker.h"

extern byte MIDIchannel;

class Track: public TouchSwitch{
  byte pin;
  public:
    Track();
    Track(int p, byte n);
    ~Track();
    int send();
    int vol(int incdec);
    byte number;
    byte level;
    byte state;
};

byte record(byte, byte);

#endif
