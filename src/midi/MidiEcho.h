#pragma once
#include <Arduino.h>
#include "../config/PinMap.h"
#include "../config/MidiConstants.h"
#include "../config/ConfigManager.h"

struct ScheduledEcho {
    bool active;
    uint8_t note;
    uint8_t velocity;
    uint8_t channel;
    uint8_t port;
    uint32_t executeAt;
    bool isNoteOff;
};

class MidiEcho {
public:
    MidiEcho();

    void update();
    void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint8_t port, int rawTouch = -1);
    void onNoteOff(uint8_t channel, uint8_t note, uint8_t port);
    void silenceAll();

private:
    static constexpr uint8_t MAX_EVENTS = 64;
    ScheduledEcho events[MAX_EVENTS];

    void scheduleEvent(uint8_t channel, uint8_t note, uint8_t velocity, uint8_t port, uint32_t delayMs, bool isOff);
};

extern MidiEcho midiEcho;
