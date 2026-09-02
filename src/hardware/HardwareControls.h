#pragma once
#include <Arduino.h>
#include <Bounce2.h>
#include <Encoder.h>
#include "MIDIcontroller.h"
#include "../config/PinMap.h"
#include "../config/MidiConstants.h"
#include "../config/ConfigManager.h"

class HardwareControls {
public:
    HardwareControls();

    void begin();
    void updateModesFromConfig(const MomiConfig& cfg);

    // Rotary Encoder CC Parameter (CC 3)
    int sendEncoderMidi(uint8_t channel);
    int readEncoderStep(); // Returns +1, -1, or 0
    void resetEncoder();

    // LEDs
    void initLeds();
    void setLedTopLeft(bool state)    { digitalWrite(Pins::LED_TOP_LEFT, state); }
    void setLedCenter(bool state)     { digitalWrite(Pins::LED_CENTER, state); }
    void setLedTopRight(bool state)   { digitalWrite(Pins::LED_TOP_RIGHT, state); }
    void setLedFs0(bool state)        { digitalWrite(Pins::LED_FOOTSWITCH_0, state); }
    void setLedFs1(bool state)        { digitalWrite(Pins::LED_FOOTSWITCH_1, state); }
    void setLedOnboard(bool state)    { digitalWrite(Pins::LED_ONBOARD, state); }

    // Physical Input Devices
    Bounce encoderButton;
    Encoder encoder;
    uint8_t encoderLevel;

    MIDIswitch touchTopLeft;
    MIDIswitch touchCenter;
    MIDIswitch touchTopRight;
    MIDIswitch touchBottomRight;
    MIDIswitch* touchButtons[4];

    MIDIswitch footSwitch1; // FS1 (Ring) CC 80
    MIDIswitch footSwitch0; // FS0 (Tip) CC 81

    MIDIpot expressionPedal;

    MIDIpot mux0Pots[8]; // CC 48..55 on Pin 20
    MIDIpot mux1Pots[8]; // CC 56..63 on Pin 21
};

extern HardwareControls hw;
