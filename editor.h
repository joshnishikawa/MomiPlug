#ifndef editor_h
#define editor_h

#include "Arduino.h"
#include "Bounce.h"
#include "Encoder.h"
#include "button.h"
#include "pot.h"

/*INCOMPLETE
    This is to be used for an encoder/switch
    that will allow different behaviors and 
    values to be set for all inputs via the
    MIDI controller itself. Currently, it 
    only sets a mode in which buttons will
    arm/disarm tracks in Ableton Live, correctly
    indicate which tracks are are armed/disarmed
    and allow the encoder to send CC messages 
    corresponding to armed tracks.
*/


class Track{
  public:
    Track();
    Track(int p);
    ~Track();
    Bounce *bTrak;
    int level;
    bool state;
};


class Editor{
    Bounce *bButn;
    Encoder *eKnob;
  public:
    Editor();
    Editor(int p, int a, int b);
    ~Editor();

    void Read(Button* Bs[], Pot* Ps[], Button& FS1, Button& FS2);
    int Tracks(Track* Ts[], Button& FS1);
    bool A();
    bool B();
    bool C();
    bool state;
    int channel;
    int returnme;
};

#endif
