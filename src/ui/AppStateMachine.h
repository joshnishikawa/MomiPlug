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
    FS0_MODE,      // Footswitch 0
    FS1_MODE,      // Footswitch 1
    ANALOG_EXP,    // Expression pedal / MIDIpot
    MIDI_CHANNEL   // Rotary encoder alone
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

    // Config mode edge tracking
    bool configTouchBrLast;
    bool configTouchTrLast;
    bool configTouchTlLast;
    bool configTouchBlLast;
    bool configFs0Last;
    bool configFs1Last;
    bool configExpLast;
};

extern AppStateMachine app;
