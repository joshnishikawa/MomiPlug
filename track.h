#ifndef track_h
#define track_h

#include "Arduino.h"

extern byte MIDIchannel;

class Track {
  public:
    Track();
    Track(byte n);
    ~Track();
    int toggleArm();
    int vol(int incdec);
    byte number;
    byte level;
    byte state;
};

byte record(byte rec, byte stp);

#endif
