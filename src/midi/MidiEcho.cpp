#include "MidiEcho.h"
#include "ChaosEngine.h"
#include <MIDI.h>

extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> MIDI;

MidiEcho midiEcho;

MidiEcho::MidiEcho() {
    for (uint8_t i = 0; i < MAX_EVENTS; i++) {
        events[i].active = false;
    }
}

void MidiEcho::scheduleEvent(uint8_t channel, uint8_t note, uint8_t velocity, uint8_t port, uint32_t delayMs, bool isOff) {
    uint32_t targetTime = millis() + delayMs;
    for (uint8_t i = 0; i < MAX_EVENTS; i++) {
        if (!events[i].active) {
            events[i].active = true;
            events[i].channel = channel;
            events[i].note = note;
            events[i].velocity = velocity;
            events[i].port = port;
            events[i].executeAt = targetTime;
            events[i].isNoteOff = isOff;
            return;
        }
    }
}

void MidiEcho::onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint8_t port, int rawTouch) {
    uint8_t expMode = configMgr.getExpMode();
    if (expMode != EXP_ECHO && expMode != EXP_BOTH) {
        return;
    }

    if (rawTouch < 0) {
        rawTouch = chaosEngine.getLastTouch();
    }
    // If not being touched (below touch threshold), do not echo at all
    if (rawTouch < EchoConfig::TOUCH_THRESHOLD) {
        return;
    }

    // Higher touch reading -> more repeats and longer decay time
    int repeats = map(
        rawTouch,
        EchoConfig::TOUCH_THRESHOLD,
        EchoConfig::TOUCH_FIRM,
        EchoConfig::MIN_REPEATS,
        EchoConfig::MAX_REPEATS
    );
    repeats = constrain(repeats, EchoConfig::MIN_REPEATS, EchoConfig::MAX_REPEATS);

    int delayInterval = map(
        rawTouch,
        EchoConfig::TOUCH_THRESHOLD,
        EchoConfig::TOUCH_FIRM,
        EchoConfig::MIN_DELAY_MS,
        EchoConfig::MAX_DELAY_MS
    );
    delayInterval = constrain(delayInterval, EchoConfig::MIN_DELAY_MS, EchoConfig::MAX_DELAY_MS);

    for (int r = 1; r <= repeats; r++) {
        uint32_t onDelay = r * delayInterval;
        uint32_t offDelay = onDelay + (delayInterval * 2 / 3);
        int decayVel = (velocity * (repeats - r + 1)) / (repeats + 1);
        decayVel = constrain(decayVel, 1, 127);

        scheduleEvent(channel, note, static_cast<uint8_t>(decayVel), port, onDelay, false);
        scheduleEvent(channel, note, 0, port, offDelay, true);
    }
}

void MidiEcho::onNoteOff(uint8_t channel, uint8_t note, uint8_t port) {
    // When the primary note is released, allow scheduled echoes to decay naturally.
}

void MidiEcho::silenceAll() {
    bool sentAny = false;
    for (uint8_t i = 0; i < MAX_EVENTS; i++) {
        if (events[i].active && !events[i].isNoteOff) {
            usbMIDI.sendNoteOff(events[i].note, 0, events[i].channel, events[i].port);
            MIDI.sendNoteOff(events[i].note, 0, events[i].channel);
            sentAny = true;
        }
        events[i].active = false;
    }
    if (sentAny) {
        usbMIDI.send_now();
    }
}

void MidiEcho::update() {
    uint32_t now = millis();
    bool sentAny = false;
    for (uint8_t i = 0; i < MAX_EVENTS; i++) {
        if (events[i].active && now >= events[i].executeAt) {
            events[i].active = false;
            if (events[i].isNoteOff) {
                usbMIDI.sendNoteOff(events[i].note, 0, events[i].channel, events[i].port);
                MIDI.sendNoteOff(events[i].note, 0, events[i].channel);
            } else {
                usbMIDI.sendNoteOn(events[i].note, events[i].velocity, events[i].channel, events[i].port);
                MIDI.sendNoteOn(events[i].note, events[i].velocity, events[i].channel);
            }
            sentAny = true;
        }
    }
    if (sentAny) {
        usbMIDI.send_now();
    }
}
