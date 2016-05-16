#ifndef editor_h
#define editor_h

#include "Arduino.h"
#include "Bounce.h"
#include "Encoder.h"
#include "MIDIbutton.h"
#include "MIDIpot.h"
#include "MIDIenc.h"
#include "MIDIcapSens.h"
#include "MIDInote.h"

/*INCOMPLETE
    This is to be used for an encoder/switch
    that will allow different behaviors and 
    values to be set for all inputs via the
    MIDI controller itself.
*/

extern int* MC;

class Editor{
  public:
    int channel;
    Editor();
    Editor(int p, Encoder* enc);
    ~Editor();
    Bounce* myButt;
    Encoder* myKnob;

    int read(MIDIbutton* Bs[], MIDIpot* Ps[], MIDIbutton& FS1, MIDIbutton& FS0);
    void edit();
    void* target;
    int targetSize;
    void editInput();
    void editButton(MIDIbutton b);
    void editPot(MIDIpot p);
    void editNote(MIDInote n);
    int editChannel(Encoder& e, int c);
    bool editing;
};

#endif
