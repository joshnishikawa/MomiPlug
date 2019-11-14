#ifndef track_h
#define track_h

#include "Arduino.h"
#include "Flicker.h"

extern byte MIDIchannel;

class Track: public Flicker{
  byte pin;
  public:
    Track();
    Track(int p, byte n, int thresh);
    ~Track();
    int send();
    int vol(int incdec);
    byte number;
    byte level;
    uint8_t state;
};

uint8_t record(uint8_t, uint8_t);

#endif
