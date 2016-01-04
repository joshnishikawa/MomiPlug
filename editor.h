#ifndef editor_h
#define editor_h

#include "Arduino.h"
#include "Bounce.h"
#include "Encoder.h"
#include "MIDIbutton.h"
#include "MIDIpot.h"

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

extern const int MOMIchannel;

class Track{
  public:
    Track();
    Track(int p);
    ~Track();
    Bounce *myTrack;
    int level;
    bool state;
};


class Editor{
    Bounce *myButt;
    Encoder *myKnob;
    int scene;
  public:
    Editor();
    Editor(int p, int a, int b);
    ~Editor();

    void check(MIDIbutton* Bs[], MIDIpot* Ps[], MIDIbutton& FS1, MIDIbutton& FS0);
    int read(Track* Ts[], MIDIbutton& FS1, MIDIbutton& FS0);
    bool a();
    bool b();
    bool c();
    bool state;
    int channel;
    int returnme;
};

#endif
