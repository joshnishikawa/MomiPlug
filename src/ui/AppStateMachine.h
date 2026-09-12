#pragma once
#include <Arduino.h>
#include <FlickerTouch.h>
#include "../config/ConfigManager.h"
#include "../hardware/HardwareControls.h"
#include "../hardware/MuxManager.h"
#include "DisplayManager.h"
#include "../midi/TrackManager.h"
#include "../midi/ChaosEngine.h"
#include "../midi/MidiRouter.h"

enum class OperatingMode {
    CONTROL,
    TRACK,
    CONFIG
};

enum class ConfigTarget {
    NONE,
    MUX_MODE,      // Bottom Right (Pin 17)
    OCTAVE,        // Top Right (Pin 23)
    TRANSPOSE,     // Top Left (Pin 19)
    EXPRESSION,    // Bottom Left (Pin 18)
    USB_FX,        // Center (Pin 22)
    FS0_MODE,      // Footswitch 0
    FS1_MODE,      // Footswitch 1
    ANALOG_EXP,    // Expression pedal / MIDIpot
    MIDI_CHANNEL   // Rotary encoder alone
};

struct TouchDebounce {
    uint8_t pin;
    int onThreshold;
    int offHysteresis;
    bool state;
    int baseline;

    TouchDebounce(uint8_t p = 0, int onThresh = 1150, int hyst = 15)
        : pin(p), onThreshold(onThresh), offHysteresis(hyst), state(false), baseline(0) {}

    void calibrate(int fixedThresh = 0) {
        long total = 0;
        for (uint8_t i = 0; i < 8; i++) {
            total += flickerTouchRead(pin);
            delayMicroseconds(500);
        }
        baseline = (int)(total / 8);

        // If a fixed threshold is provided and is well above baseline (+40 to +180), respect it.
        // Otherwise, dynamically set threshold to baseline + 80 (with 20 hysteresis).
        if (fixedThresh > 0 && (fixedThresh >= baseline + 40) && (fixedThresh <= baseline + 180)) {
            onThreshold = fixedThresh;
            offHysteresis = 20;
        } else {
            onThreshold = baseline + 80;
            offHysteresis = 20;
        }
    }

    bool update() {
        int raw = flickerTouchRead(pin);
        if (!state) {
            if (raw >= onThreshold) {
                state = true;
                return true;
            }
        } else {
            if (raw < (onThreshold - offHysteresis)) {
                state = false;
            }
        }
        return false;
    }

    void reset() {
        state = false;
    }

    void prime() {
        int raw = flickerTouchRead(pin);
        state = (raw >= onThreshold);
    }
};

class AppStateMachine {
public:
    AppStateMachine();

    void begin();
    void update();

    OperatingMode getMode() const { return currentMode; }

private:
    void handleEncoderButton();
    void checkConfigTriggersWhileEditHeld();
    void processControlMode();
    void processTrackMode();
    void processConfigMode();
    void processSharedSensors();
    void resendMuxPots();

    OperatingMode currentMode;
    OperatingMode previousMode;
    ConfigTarget  activeTarget;
    bool          triggerFiredWhileEditHeld;
    bool          channelChangedWhileEditHeld;
    bool          channelDisplayShown;
    bool          configModified;
    elapsedMillis bootTimer;
    elapsedMillis editHoldTimer;
    elapsedMillis touchPollTimer;
    elapsedMillis configTouchTimer;

    // Expression / MIDIpot calibration
    int  initialExpReading;
    bool expCalibrating;
    int  minExpReading;
    int  maxExpReading;
    int  lastRawExp;

    // Control mode round-robin touch index
    uint8_t ctrlTouchIdx;

    // Track mode round-robin touch index
    uint8_t trackTouchIdx;
    bool trackBtnArmedLast[3];

    // Debounced capacitive touch controllers with hysteresis
    TouchDebounce touchBr;     // Pin 17 (Bottom Right) -> MUX mode
    TouchDebounce touchTr;     // Pin 23 (Top Right)    -> Octave mode
    TouchDebounce touchTl;     // Pin 19 (Top Left)     -> Transpose
    TouchDebounce touchCenter; // Pin 22 (Center)       -> USB FX
    TouchDebounce touchBl;     // Pin 18 (Bottom Left)  -> Expression / Chaos Pad

    // Config mode physical input edge tracking
    bool configFs0Last;
    bool configFs1Last;
    bool configExpLast;
};

extern AppStateMachine app;
