#pragma once
#include <Arduino.h>
#include "../config/PinMap.h"
#include "../config/MidiConstants.h"

class ChaosEngine {
public:
    ChaosEngine();

    void calibrateBaseline();
    void update(uint8_t midiChannel, const bool dinChords[12], const bool usbChords[12]);
    void silence(uint8_t midiChannel);

    uint16_t getInLow() const  { return inLo; }
    uint16_t getInHigh() const { return inHi; }
    int      getLastTouch() const { return lastRawTouch; }

private:
    uint16_t inLo;
    uint16_t inHi;
    int      lastRawTouch;
    uint8_t  activeNote;
    bool     waiting;
    uint16_t waitTimeMs;
    elapsedMillis noteTimer;
    elapsedMillis pollTimer;
};

extern ChaosEngine chaosEngine;
