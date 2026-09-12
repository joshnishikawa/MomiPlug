#pragma once
#include <Arduino.h>
#include "../config/PinMap.h"

class Metronome {
public:
    Metronome();

    void begin();
    void update();

    // Event handlers for USB client MIDI (DAW real-time messages)
    void onClock();
    void onStart();
    void onContinue();
    void onStop();

    bool isRunning() const { return running; }
    uint8_t getClockCount() const { return clockCount; }

private:
    void triggerBeat();

    bool running;
    bool stoppedExplicitly;
    bool ledActive;
    uint8_t clockCount;
    elapsedMillis ledTimer;
    elapsedMillis clockTimeoutTimer;

    static constexpr uint32_t FLASH_DURATION_MS = 30;  // Crisp visual flash duration on beat
    static constexpr uint32_t CLOCK_TIMEOUT_MS  = 500; // Turn off LED if clocks cease for 500ms
};

extern Metronome metronome;
