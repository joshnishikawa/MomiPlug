#include "AppStateMachine.h"

AppStateMachine app;

AppStateMachine::AppStateMachine()
    : currentMode(OperatingMode::CONTROL),
      configModified(false),
      ctrlTouchIdx(0),
      trackTouchIdx(0),
      trackBtnArmedLast{false, false, false},
      configBtn0Last(false),
      configBtn1Last(false),
      configBtn2Last(false),
      configFs0Last(false),
      configFs1Last(false),
      configExpLast(false) {}

void AppStateMachine::begin() {
    configMgr.load();
    hw.begin();
    hw.updateModesFromConfig(configMgr.get());
    muxMgr.begin();
    display.begin();
    chaosEngine.calibrateBaseline();
}

void AppStateMachine::update() {
    handleEncoderButton();
}

void AppStateMachine::handleEncoderButton() {
    hw.encoderButton.update();

    if (hw.encoderButton.fell()) {
        // Show MUX configuration on initial button press
        display.showMuxModes(configMgr.getMux0Mode(), configMgr.getMux1Mode());
    }
    else if (hw.encoderButton.rose()) {
        display.clear();
        hw.resetEncoder();

        if (configModified) {
            configMgr.save();
            hw.updateModesFromConfig(configMgr.get());
            configModified = false;
        }
        else {
            // Short press toggle between CONTROL and TRACK modes
            if (currentMode == OperatingMode::CONTROL) {
                currentMode = OperatingMode::TRACK;
                trackMgr.sendAllTrackLevels(configMgr.getMidiChannel());
                display.showText("trac");
            } else {
                currentMode = OperatingMode::CONTROL;
                // Resend pot states for active MUX configuration
                uint8_t ch = configMgr.getMidiChannel();
                if (configMgr.getMux0Mode() == 1) {
                    usbMIDI.sendControlChange(hw.mux0Pots[0].number, hw.mux0Pots[0].value, ch);
                } else if (configMgr.getMux0Mode() == 8) {
                    for (int i = 0; i < 8; i++) {
                        usbMIDI.sendControlChange(hw.mux0Pots[i].number, hw.mux0Pots[i].value, ch);
                    }
                }
                if (configMgr.getMux1Mode() == 1) {
                    usbMIDI.sendControlChange(hw.mux1Pots[0].number, hw.mux1Pots[0].value, ch);
                } else if (configMgr.getMux1Mode() == 8) {
                    for (int i = 0; i < 8; i++) {
                        usbMIDI.sendControlChange(hw.mux1Pots[i].number, hw.mux1Pots[i].value, ch);
                    }
                }
                display.showText("ctrl");
            }
        }
    }
    else if (hw.encoderButton.read() == LOW) {
        // Button held down -> Config / Select Mode
        processConfigMode();
    }
    else {
        // Normal operation (CONTROL or TRACK mode)
        processSharedSensors();

        if (currentMode == OperatingMode::TRACK) {
            processTrackMode();
        } else {
            processControlMode();
        }
    }
}

void AppStateMachine::processConfigMode() {
    // 1. Channel edit via encoder rotation
    int encStep = hw.readEncoderStep();
    if (encStep != 0) {
        uint8_t ch = configMgr.getMidiChannel();
        if (encStep > 0) {
            ch = (ch >= 16) ? 1 : (ch + 1);
        } else {
            ch = (ch <= 1) ? 16 : (ch - 1);
        }
        configMgr.setMidiChannel(ch);
        configModified = true;
        display.showChannel(ch);
    }

    // 2. Touch button toggles for Thru and MUX modes (edge detected)
    bool btn0Now = (hw.touchTopLeft.read() == 127);
    if (btn0Now != configBtn0Last) {
        configMgr.toggleMidiThru();
        configModified = true;
        display.showText(configMgr.isMidiThruEnabled() ? "tru1" : "tru0");
    }
    configBtn0Last = btn0Now;

    bool btn1Now = (hw.touchCenter.read() == 127);
    if (btn1Now != configBtn1Last) {
        configMgr.cycleMux0Mode();
        configModified = true;
        display.showMuxModes(configMgr.getMux0Mode(), configMgr.getMux1Mode());
    }
    configBtn1Last = btn1Now;

    bool btn2Now = (hw.touchTopRight.read() == 127);
    if (btn2Now != configBtn2Last) {
        configMgr.cycleMux1Mode();
        configModified = true;
        display.showMuxModes(configMgr.getMux0Mode(), configMgr.getMux1Mode());
    }
    configBtn2Last = btn2Now;

    // 3. Footswitch mode toggles (Momentary vs Latch)
    bool fs1Now = (digitalRead(Pins::FOOTSWITCH_1) == LOW);
    if (fs1Now && !configFs1Last) {
        configMgr.toggleFs1Mode();
        hw.footSwitch1.mode = configMgr.getFs1Mode();
        configModified = true;
        display.showText(configMgr.getFs1Mode() ? "1-lc" : "1-mo");
    }
    configFs1Last = fs1Now;

    bool fs0Now = (digitalRead(Pins::FOOTSWITCH_0) == LOW);
    if (fs0Now && !configFs0Last) {
        configMgr.toggleFs0Mode();
        hw.footSwitch0.mode = configMgr.getFs0Mode();
        configModified = true;
        display.showText(configMgr.getFs0Mode() ? "0-lc" : "0-mo");
    }
    configFs0Last = fs0Now;

    // 4. Expression pedal killswitch toggle (edge detected via raw analog read)
    int rawExp = analogRead(Pins::EXPRESSION_PEDAL);
    bool expNow = (rawExp >= ExpressionConfig::KILLSWITCH_THRESHOLD);
    if (expNow && !configExpLast) {
        configMgr.toggleExpKillSwitch();
        hw.expressionPedal.killSwitch = configMgr.getExpKillSwitch();
        configModified = true;
        display.showText(configMgr.getExpKillSwitch() ? " cut" : "-cut");
    }
    configExpLast = expNow;

    // Visual status indicators while in edit mode
    hw.setLedTopLeft(configMgr.isMidiThruEnabled());
    hw.setLedCenter(configMgr.getMux0Mode() != 0);
    hw.setLedTopRight(configMgr.getMux1Mode() != 0);
}

void AppStateMachine::processTrackMode() {
    uint8_t ch = configMgr.getMidiChannel();

    // Encoder adjusts volume of armed tracks
    int encStep = hw.readEncoderStep();
    if (encStep != 0) {
        int newLevel = trackMgr.adjustArmedTracksVolume(encStep, ch);
        if (newLevel >= 0) {
            display.showNumber(newLevel);
        }
    }

    // Touch buttons 0..2 arm/disarm tracks
    bool isPressed = (hw.touchButtons[trackTouchIdx]->read() == 127);
    if (isPressed && !trackBtnArmedLast[trackTouchIdx]) {
        int newLevel = trackMgr.toggleTrackArm(trackTouchIdx, ch);
        display.showNumber(newLevel);
    }
    trackBtnArmedLast[trackTouchIdx] = isPressed;
    trackTouchIdx = (trackTouchIdx + 1) % 3;

    // Footswitches trigger looper scene record & stop
    bool fs1Pressed = (hw.footSwitch1.read() == 127);
    bool fs0Pressed = (hw.footSwitch0.read() == 127);
    bool recState = trackMgr.handleRecording(fs1Pressed, fs0Pressed, ch);

    // Track Mode LED status
    hw.setLedFs0(recState);
    hw.setLedFs1(!digitalRead(Pins::FOOTSWITCH_0));
    hw.setLedTopLeft(trackMgr.isTrackArmed(0));
    hw.setLedCenter(trackMgr.isTrackArmed(1));
    hw.setLedTopRight(trackMgr.isTrackArmed(2));
}

void AppStateMachine::processControlMode() {
    uint8_t ch = configMgr.getMidiChannel();

    // 1. Encoder parameter CC 3
    int newVal = hw.sendEncoderMidi(ch);
    if (newVal >= 0) {
        display.showControlValue('r', hw.encoderLevel);
    }

    // 2. Round-robin polling across active onboard touch buttons (0..3)
    newVal = hw.touchButtons[ctrlTouchIdx]->send();
    if (newVal >= 0) {
        display.showControlValue('b', newVal);
    }
    ctrlTouchIdx = (ctrlTouchIdx + 1) % 4;

    // 3. Poll foot switches (FS1 CC 80, FS0 CC 81)
    newVal = hw.footSwitch1.send();
    if (newVal >= 0) {
        display.showControlValue('f', newVal);
    }
    newVal = hw.footSwitch0.send();
    if (newVal >= 0) {
        display.showControlValue('f', newVal);
    }

    // Control Mode LED status
    hw.setLedTopLeft(hw.touchTopLeft.state);
    hw.setLedCenter(hw.touchCenter.state);
    hw.setLedTopRight(hw.touchTopRight.state);
    hw.setLedFs0(hw.footSwitch1.state);
    hw.setLedFs1(hw.footSwitch0.state);
}

void AppStateMachine::processSharedSensors() {
    uint8_t ch = configMgr.getMidiChannel();

    // 1. Expression Pedal
    int newVal = hw.expressionPedal.send();
    if (newVal >= 0) {
        display.showControlValue('E', newVal);
    }

    // 2. Chord Display & Chaos Synth
    if (configMgr.isMidiThruEnabled()) {
        display.updateChordDisplayIfChanged(midiRouter.getAnalyzer());
        chaosEngine.update(ch, midiRouter.getDinChords(), midiRouter.getUsbChords());
    }

    // 3. Multiplexer Bank 0 Pot(s)
    if (configMgr.getMux0Mode() == 1) {
        newVal = hw.mux0Pots[0].send();
        if (newVal >= 0) {
            display.showControlValue('h', newVal);
        }
    }

    // 4. Multiplexer Bank 1 Pot(s)
    if (configMgr.getMux1Mode() == 1) {
        newVal = hw.mux1Pots[0].send();
        if (newVal >= 0) {
            display.showControlValue('H', newVal);
        }
    }

    // 5. 8-Channel Multiplexer Cycling
    if (configMgr.getMux0Mode() == 8 || configMgr.getMux1Mode() == 8) {
        for (uint8_t i = 0; i < 8; i++) {
            muxMgr.selectChannel(i);

            if (configMgr.getMux0Mode() == 8) {
                newVal = hw.mux0Pots[i].send();
                if (newVal >= 0) {
                    display.showControlValue('A', newVal);
                }
            }

            if (configMgr.getMux1Mode() == 8) {
                newVal = hw.mux1Pots[i].send();
                if (newVal >= 0) {
                    display.showControlValue('B', newVal);
                }
            }
        }
    }
}
