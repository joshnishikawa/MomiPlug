#ifndef editor_h
#define editor_h

#include "Arduino.h"
#include "Bounce.h"
#include "Encoder.h"
#include <EEPROM.h>
#include "display.h"
#include "MIDIbutton.h"
#include "MIDIpot.h"
#include "MIDIenc.h"
#include "MIDIcapSens.h"
#include "MIDInote.h"

extern byte MIDIchannel;
extern Display DSP;

class Editor{
    int quadOne(byte val, byte max);
  public:
    Editor();
    Editor(int a, int b, int p);
    ~Editor();
    Bounce* myButt;
    Encoder* myKnob;
    int number;
    int level;
    bool editing;
    MIDIbutton* bTarget;
    MIDIpot* pTarget;
    int storageIndex;
    byte editChannel();
    void editButton(MIDIbutton& bt);
    void editPot(MIDIpot& pt);
    // TODO: add functions for editing MIDIenc, MIDInote and MIDIcapSens
    int read(int tmp, int min, int max); //to let analog inputs edit themselves
    int send(); //cuz interrupts won't let two objects use the same encoder pins
};

#endif
