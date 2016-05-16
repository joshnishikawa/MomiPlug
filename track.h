#ifndef track_h
#define track_h

#include "Arduino.h"
#include "Bounce.h"
#include "Encoder.h"
#include "MIDIbutton.h"
#include "MIDIenc.h"

extern int* MC;

class Track{
  public:
    Track();
    Track(int p, int n);
    ~Track();
    Bounce *myTrack;
    int read();
    int vol(int incdec, int i);
//    int scene;
    int number;
    int level;
    bool state;
};

//void record(Bounce rec,  Bounce stop, Track& T[]);

#endif

