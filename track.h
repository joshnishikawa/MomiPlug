#ifndef track_h
#define track_h

#include "Arduino.h"
#include "Bounce.h"
#include "Encoder.h"
#include "MIDIbutton.h"
#include "MIDIenc.h"

extern byte MIDIchannel;

class Track{
    byte* MC = &MIDIchannel;
  public:
    Track();
    Track(int p, byte n);
    ~Track();
    Bounce *myTrack;
    int send();
    int vol(int incdec);
//    int scene;
    byte number;
    byte level;
    bool state;
};

//void record(Bounce rec,  Bounce stop, Track& T[]);

#endif

