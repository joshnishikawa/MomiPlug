#ifndef editor_h
#define editor_h

#include "Arduino.h"
#include <EEPROM.h>
#include "SevSeg.h"
#include "MIDIcontroller.h"
extern byte MIDIchannel;
extern SevSeg DSP;

class Editor{
    int quadOne(byte val, byte max);
    char DSPstring[5];
    byte DP;
  public:
    Editor();
    Editor(int a, int b, int p);
    ~Editor();
    Bounce* bounce;
    Encoder* encoder;
    int number;
    int level, outHi, outLo;
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

