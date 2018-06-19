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
//    int scene;
    byte number;
    byte level;
    bool state;
};

//void record(Bounce rec,  Bounce stop, Track& T[]);

#endif

