#include <Arduino.h>
#include <IntervalTimer.h>
#include "src/midi/MidiRouter.h"
#include "src/ui/AppStateMachine.h"

// 1 kHz hardware timer for 7-segment display multiplexing
IntervalTimer displayTimer;

void displayTimerISR() {
    display.refreshHardware();
}

void setup() {
    Serial.begin(9600);

    // Initialize MIDI Routing & USB Host
    midiRouter.begin();

    // Initialize Application Engine & Hardware Subsystems
    app.begin();

    // Start background display refresh interrupt (1000 microseconds period)
    displayTimer.begin(displayTimerISR, 1000);
}

void loop() {
    // Process incoming DIN MIDI & 4-port USB Host packets
    midiRouter.process();

    // Update state machine, hardware inputs, displays, and synthesis
    app.update();
}