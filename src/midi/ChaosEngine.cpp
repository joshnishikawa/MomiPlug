#include "ChaosEngine.h"

ChaosEngine chaosEngine;

ChaosEngine::ChaosEngine()
    : inLo(1000),
      inHi(1150),
      activeNote(0),
      waiting(false),
      waitTimeMs(0),
      noteTimer(0),
      pollTimer(0) {}

void ChaosEngine::calibrateBaseline() {
    delay(100);
    uint32_t baseline = 0;
    for (int i = 0; i < 16; i++) {
        baseline += touchRead(Pins::TOUCH_CHAOS_PAD);
        delay(5);
    }
    inLo = static_cast<uint16_t>((baseline / 16) + 150); // Touch threshold above idle baseline
    inHi = inLo + 500;                                   // Full touch range
}

void ChaosEngine::silence(uint8_t midiChannel) {
    if (activeNote > 0) {
        usbMIDI.sendNoteOn(activeNote, 0, midiChannel);
        activeNote = 0;
    }
    digitalWrite(Pins::LED_ONBOARD, LOW);
    waiting = false;
}

void ChaosEngine::update(uint8_t midiChannel, const bool dinChords[12], const bool usbChords[12]) {
    if (pollTimer < TouchConfig::CHAOS_POLL_MS) {
        return;
    }
    pollTimer = 0;

    int rawTouch = touchRead(Pins::TOUCH_CHAOS_PAD);

    // Below touch threshold: silence note and turn off LED
    if (rawTouch < inLo) {
        silence(midiChannel);
        return;
    }

    uint16_t mappedNote = map(
        rawTouch,
        inLo,
        inHi,
        TouchConfig::CHAOS_NOTE_MIN,
        TouchConfig::CHAOS_NOTE_MAX
    );
    mappedNote = constrain(mappedNote, TouchConfig::CHAOS_NOTE_MIN, TouchConfig::CHAOS_NOTE_MAX);

    if (waiting) {
        if (noteTimer > waitTimeMs) {
            waiting = false;
        }
    }
    else if (mappedNote > 0) {
        uint8_t pitchClass = mappedNote % 12;
        // Trigger note only if pitch class is present in active DIN or USB chords
        if ((dinChords[pitchClass] || usbChords[pitchClass]) && mappedNote != activeNote) {
            if (activeNote > 0) {
                usbMIDI.sendNoteOn(activeNote, 0, midiChannel); // Silence previous note
            }
            usbMIDI.sendNoteOn(mappedNote, TouchConfig::CHAOS_VELOCITY, midiChannel);
            waitTimeMs = (mappedNote < 150) ? (150 - mappedNote) : 10;
            activeNote = static_cast<uint8_t>(mappedNote);
            noteTimer = 0;
            waiting = true;
            digitalWrite(Pins::LED_ONBOARD, HIGH);
        } else {
            digitalWrite(Pins::LED_ONBOARD, LOW);
        }
    }
}
