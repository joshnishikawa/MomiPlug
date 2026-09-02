#pragma once
#include <Arduino.h>
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

class AppStateMachine {
public:
    AppStateMachine();

    void begin();
    void update();

    OperatingMode getMode() const { return currentMode; }

private:
    void handleEncoderButton();
    void processControlMode();
    void processTrackMode();
    void processConfigMode();
    void processSharedSensors();

    OperatingMode currentMode;
    bool configModified;

    // Control mode round-robin touch index
    uint8_t ctrlTouchIdx;

    // Track mode round-robin touch index
    uint8_t trackTouchIdx;
    bool trackBtnArmedLast[3];

    // Config mode edge detection tracking
    bool configBtn0Last;
    bool configBtn1Last;
    bool configBtn2Last;
    bool configFs0Last;
    bool configFs1Last;
    bool configExpLast;
};

extern AppStateMachine app;
