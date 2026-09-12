#include "Metronome.h"

Metronome metronome;

Metronome::Metronome()
    : running(false),
      stoppedExplicitly(false),
      ledActive(false),
      clockCount(0),
      ledTimer(0),
      clockTimeoutTimer(0) {}

void Metronome::begin() {
    pinMode(Pins::LED_ONBOARD, OUTPUT);
    digitalWrite(Pins::LED_ONBOARD, LOW);
    running = false;
    stoppedExplicitly = false;
    ledActive = false;
    clockCount = 0;
}

void Metronome::triggerBeat() {
    digitalWrite(Pins::LED_ONBOARD, HIGH);
    ledActive = true;
    ledTimer = 0;
}

void Metronome::onStart() {
    running = true;
    stoppedExplicitly = false;
    clockCount = 0;
    clockTimeoutTimer = 0;
    triggerBeat(); // Beat 1 downbeat pulse immediately on transport start
}

void Metronome::onContinue() {
    running = true;
    stoppedExplicitly = false;
    clockTimeoutTimer = 0;
}

void Metronome::onStop() {
    running = false;
    stoppedExplicitly = true;
    digitalWrite(Pins::LED_ONBOARD, LOW);
    ledActive = false;
    clockCount = 0;
}

void Metronome::onClock() {
    clockTimeoutTimer = 0;

    // If DAW explicitly stopped playback, ignore background clocks
    if (stoppedExplicitly) {
        return;
    }

    running = true;
    clockCount++;

    // Standard MIDI 1.0 Timing Clock: 24 clocks per quarter note (1 beat)
    if (clockCount >= 24) {
        clockCount = 0;
        triggerBeat();
    }
}

void Metronome::update() {
    // Turn off LED after flash duration has elapsed
    if (ledActive && ledTimer >= FLASH_DURATION_MS) {
        digitalWrite(Pins::LED_ONBOARD, LOW);
        ledActive = false;
    }

    // Safety timeout: if clocks cease while running (e.g. cable disconnect or DAW quit)
    if (running && clockTimeoutTimer >= CLOCK_TIMEOUT_MS) {
        running = false;
        digitalWrite(Pins::LED_ONBOARD, LOW);
        ledActive = false;
        clockCount = 0;
    }
}
