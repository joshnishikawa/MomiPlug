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

extern byte MIDIchannel;

class Editor{
    byte* MC = &MIDIchannel;
  public:
    Editor();
    Editor(int p, Encoder* enc);
    ~Editor();
    Bounce* myButt;
    Encoder* myKnob;

    void* target;
    int targetSize;
    void edit();
    int editChannel(Encoder& e);
    void editInput();
    void editButton(MIDIbutton& b);
    void editPot(MIDIpot& p);
    // TODO: add functions for editing MIDIenc, MIDInote and MIDIcapSens
    bool editing;
};

#endif
