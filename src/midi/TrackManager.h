#pragma once
#include <Arduino.h>
#include "../config/MidiConstants.h"

class DawTrack {
public:
    DawTrack(uint8_t armCcNumber = 0);

    int toggleArm(uint8_t midiChannel);
    int adjustVolume(int incdec, uint8_t midiChannel);

    uint8_t getCcNumber() const { return armCc; }
    uint8_t getVolume() const   { return volume; }
    bool isArmed() const        { return armed; }

private:
    uint8_t armCc;
    uint8_t volume;
    bool    armed;
};

class TrackManager {
public:
    TrackManager();

    int toggleTrackArm(uint8_t trackIndex, uint8_t midiChannel);
    int adjustArmedTracksVolume(int incdec, uint8_t midiChannel);
    void sendAllTrackLevels(uint8_t midiChannel);

    bool handleRecording(bool recPressed, bool stopPressed, uint8_t midiChannel);

    bool isTrackArmed(uint8_t trackIndex) const;
    uint8_t getTrackVolume(uint8_t trackIndex) const;

    DawTrack tracks[3];

private:
    uint8_t currentScene;
};

extern TrackManager trackMgr;
