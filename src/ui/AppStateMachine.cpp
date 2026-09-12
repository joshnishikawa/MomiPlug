#include "AppStateMachine.h"

AppStateMachine app;

AppStateMachine::AppStateMachine()
    : currentMode(OperatingMode::CONTROL),
      previousMode(OperatingMode::CONTROL),
      activeTarget(ConfigTarget::NONE),
      triggerFiredWhileEditHeld(false),
      channelChangedWhileEditHeld(false),
      channelDisplayShown(false),
      configModified(false),
      bootTimer(0),
      editHoldTimer(0),
      touchPollTimer(0),
      initialExpReading(0),
      expCalibrating(false),
      minExpReading(1023),
      maxExpReading(0),
      lastRawExp(0),
      ctrlTouchIdx(0),
      trackTouchIdx(0),
      trackBtnArmedLast{false, false, false},
      configTouchBrLast(false),
      configTouchTrLast(false),
      configTouchTlLast(false),
      configTouchBlLast(false),
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

    currentMode = OperatingMode::CONTROL;
    previousMode = OperatingMode::CONTROL;
    activeTarget = ConfigTarget::NONE;
    triggerFiredWhileEditHeld = false;
    configModified = false;
    bootTimer = 0;

    display.showText("ctrl");
}

void AppStateMachine::update() {
    handleEncoderButton();
}

void AppStateMachine::handleEncoderButton() {
    hw.encoderButton.update();

    // Ignore spurious button edge transitions during startup settling (first 300ms)
    if (bootTimer < 300) {
        return;
    }

    // 1. If currently in latched CONFIG mode
    if (currentMode == OperatingMode::CONFIG) {
        // Press & release of EDIT button saves and returns to previous mode
        if (hw.encoderButton.rose()) {
            if (activeTarget == ConfigTarget::ANALOG_EXP && expCalibrating) {
                int finalHi = maxExpReading - 10;
                int finalLo = minExpReading + 10;
                if (finalHi > finalLo) {
                    configMgr.setExpInputRange(static_cast<uint16_t>(finalLo), static_cast<uint16_t>(finalHi));
                }
            }
            configMgr.save();
            hw.updateModesFromConfig(configMgr.get());

            currentMode = previousMode;
            activeTarget = ConfigTarget::NONE;
            triggerFiredWhileEditHeld = false;
            expCalibrating = false;

            if (currentMode == OperatingMode::TRACK) {
                display.showText("trac");
            } else {
                display.showText("ctrl");
                resendMuxPots();
            }
            return;
        }

        // Process configuration updates via rotary encoder and touch targets
        processConfigMode();
        return;
    }

    // 2. In normal mode (CONTROL or TRACK): EDIT button just pressed down
    if (hw.encoderButton.fell()) {
        editHoldTimer = 0;
        triggerFiredWhileEditHeld = false;
        channelChangedWhileEditHeld = false;
        channelDisplayShown = false;
        activeTarget = ConfigTarget::NONE;
        // Sample current states at moment of press so resting baseline doesn't trigger false edges
        configTouchBrLast = (touchRead(Pins::TOUCH_BOTTOM_RIGHT) >= TouchConfig::THRESHOLD_BOTTOM_RIGHT);
        configTouchTrLast = (touchRead(Pins::TOUCH_TOP_RIGHT) >= TouchConfig::THRESHOLD_TOP_RIGHT);
        configTouchTlLast = (touchRead(Pins::TOUCH_TOP_LEFT) >= TouchConfig::THRESHOLD_TOP_LEFT);
        configTouchBlLast = (touchRead(Pins::TOUCH_CHAOS_PAD) >= TouchConfig::THRESHOLD_BOTTOM_LEFT);
        configFs0Last = (digitalRead(Pins::FOOTSWITCH_0) == LOW);
        configFs1Last = (digitalRead(Pins::FOOTSWITCH_1) == LOW);
        configExpLast = (analogRead(Pins::EXPRESSION_PEDAL) >= ExpressionConfig::KILLSWITCH_THRESHOLD);
        hw.resetEncoder();
    }

    // 3. Normal mode (CONTROL or TRACK) with EDIT button held down
    if (hw.encoderButton.read() == LOW) {
        checkConfigTriggersWhileEditHeld();
        return;
    }

    // 4. EDIT button released
    if (hw.encoderButton.rose()) {
        configTouchBrLast = false;
        configTouchTrLast = false;
        configTouchTlLast = false;
        configTouchBlLast = false;
        configFs0Last = false;
        configFs1Last = false;
        configExpLast = false;
        if (triggerFiredWhileEditHeld) {
            // Latch into CONFIG mode (touch pad, footswitch, or pedal was triggered)
            previousMode = currentMode;
            currentMode = OperatingMode::CONFIG;
            hw.turnOffAllLeds();

            if (activeTarget == ConfigTarget::ANALOG_EXP) {
                display.showText("SEnS");
                initialExpReading = analogRead(Pins::EXPRESSION_PEDAL);
                expCalibrating = false;
                minExpReading = 1023;
                maxExpReading = 0;
            }
            return;
        } else if (channelChangedWhileEditHeld) {
            // Channel was changed while holding EDIT - save and return to operating mode
            configMgr.save();
            if (currentMode == OperatingMode::TRACK) {
                display.showText("trac");
            } else {
                display.showText("ctrl");
                resendMuxPots();
            }
            return;
        } else if (editHoldTimer < 300) {
            // Short press toggle between CONTROL and TRACK modes
            if (currentMode == OperatingMode::CONTROL) {
                currentMode = OperatingMode::TRACK;
                trackMgr.sendAllTrackLevels(configMgr.getMidiChannel());
                display.showText("trac");
            } else {
                currentMode = OperatingMode::CONTROL;
                display.showText("ctrl");
                resendMuxPots();
            }
            return;
        } else {
            // Held EDIT to view channel and released without changing anything
            if (currentMode == OperatingMode::TRACK) {
                display.showText("trac");
            } else {
                display.showText("ctrl");
                resendMuxPots();
            }
            return;
        }
    }

    // 5. Normal operation (CONTROL or TRACK mode)
    processSharedSensors();

    if (currentMode == OperatingMode::TRACK) {
        processTrackMode();
    } else {
        processControlMode();
    }
}

void AppStateMachine::checkConfigTriggersWhileEditHeld() {
    // 1. Bottom Right touch (Pin 17) -> Halt MUX reads immediately & select MUX mode
    int raw17 = touchRead(Pins::TOUCH_BOTTOM_RIGHT);
    bool brNow = (raw17 >= TouchConfig::THRESHOLD_BOTTOM_RIGHT);
    if (brNow && !configTouchBrLast) {
        hw.haltMuxReads = true;
        configMgr.setMuxMode(MUX_NONE);
        configMgr.save();
        activeTarget = ConfigTarget::MUX_MODE;
        triggerFiredWhileEditHeld = true;
        display.showText(configMgr.getMuxModeString());
    }
    configTouchBrLast = brNow;

    // 2. Top Right touch (Pin 23) -> Octave mode
    int raw23 = touchRead(Pins::TOUCH_TOP_RIGHT);
    bool trNow = (raw23 >= TouchConfig::THRESHOLD_TOP_RIGHT);
    if (trNow && !configTouchTrLast) {
        activeTarget = ConfigTarget::OCTAVE;
        triggerFiredWhileEditHeld = true;
        display.showText(configMgr.getOctaveModeString());
    }
    configTouchTrLast = trNow;

    // 3. Top Left touch (Pin 19) -> Transpose
    int raw19 = touchRead(Pins::TOUCH_TOP_LEFT);
    bool tlNow = (raw19 >= TouchConfig::THRESHOLD_TOP_LEFT);
    if (tlNow && !configTouchTlLast) {
        activeTarget = ConfigTarget::TRANSPOSE;
        triggerFiredWhileEditHeld = true;
        display.showNumber(configMgr.getTranspose());
    }
    configTouchTlLast = tlNow;

    // 4. Bottom Left touch (Pin 18) -> Expression mode
    int raw18 = touchRead(Pins::TOUCH_CHAOS_PAD);
    bool blNow = (raw18 >= TouchConfig::THRESHOLD_BOTTOM_LEFT);
    if (blNow && !configTouchBlLast) {
        activeTarget = ConfigTarget::EXPRESSION;
        triggerFiredWhileEditHeld = true;
        display.showText(configMgr.getExpModeString());
    }
    configTouchBlLast = blNow;

    // 6. Footswitch 0 -> Footswitch 0 mode
    bool fs0Now = (digitalRead(Pins::FOOTSWITCH_0) == LOW);
    if (fs0Now && !configFs0Last) {
        activeTarget = ConfigTarget::FS0_MODE;
        triggerFiredWhileEditHeld = true;
        display.showText(configMgr.getFs0Mode() ? "0-lc" : "0-mo");
    }
    configFs0Last = fs0Now;

    // 7. Footswitch 1 -> Footswitch 1 mode
    bool fs1Now = (digitalRead(Pins::FOOTSWITCH_1) == LOW);
    if (fs1Now && !configFs1Last) {
        activeTarget = ConfigTarget::FS1_MODE;
        triggerFiredWhileEditHeld = true;
        display.showText(configMgr.getFs1Mode() ? "1-lc" : "1-mo");
    }
    configFs1Last = fs1Now;

    // 8. Expression pedal / MIDIpot analog reading
    int rawExp = analogRead(Pins::EXPRESSION_PEDAL);
    bool expNow = (rawExp >= ExpressionConfig::KILLSWITCH_THRESHOLD);
    if (expNow && !configExpLast) {
        activeTarget = ConfigTarget::ANALOG_EXP;
        triggerFiredWhileEditHeld = true;
        configMgr.toggleExpKillSwitch();
        hw.expressionPedal.killSwitch = configMgr.getExpKillSwitch();
        display.showText(configMgr.getExpKillSwitch() ? " cut" : "-cut");
    }
    configExpLast = expNow;

    // 9. Rotary Encoder while EDIT is held
    int encStep = hw.readEncoderStep();
    if (encStep != 0) {
        switch (activeTarget) {
            case ConfigTarget::MUX_MODE:
                hw.haltMuxReads = true;
                configMgr.stepMuxMode(encStep);
                display.showText(configMgr.getMuxModeString());
                break;
            case ConfigTarget::OCTAVE:
                configMgr.stepOctaveMode(encStep);
                display.showText(configMgr.getOctaveModeString());
                break;
            case ConfigTarget::TRANSPOSE:
                configMgr.stepTranspose(encStep);
                display.showNumber(configMgr.getTranspose());
                break;
            case ConfigTarget::EXPRESSION:
                configMgr.stepExpMode(encStep);
                display.showText(configMgr.getExpModeString());
                break;
            case ConfigTarget::FS0_MODE:
                configMgr.toggleFs0Mode();
                hw.footSwitch0.mode = configMgr.getFs0Mode();
                display.showText(configMgr.getFs0Mode() ? "0-lc" : "0-mo");
                break;
            case ConfigTarget::FS1_MODE:
                configMgr.toggleFs1Mode();
                hw.footSwitch1.mode = configMgr.getFs1Mode();
                display.showText(configMgr.getFs1Mode() ? "1-lc" : "1-mo");
                break;
            case ConfigTarget::ANALOG_EXP:
                configMgr.stepExpCcNumber(encStep);
                display.showControlValue('c', configMgr.getExpCcNumber());
                break;
            case ConfigTarget::NONE:
            case ConfigTarget::MIDI_CHANNEL: {
                activeTarget = ConfigTarget::MIDI_CHANNEL;
                channelChangedWhileEditHeld = true;
                uint8_t ch = configMgr.getMidiChannel();
                if (encStep > 0) {
                    ch = (ch >= 16) ? 1 : (ch + 1);
                } else {
                    ch = (ch <= 1) ? 16 : (ch - 1);
                }
                configMgr.setMidiChannel(ch);
                display.showChannel(ch);
                break;
            }
        }
    } else if (activeTarget == ConfigTarget::NONE) {
        if (editHoldTimer >= 150 && !channelDisplayShown) {
            channelDisplayShown = true;
            display.showChannel(configMgr.getMidiChannel());
        }
    }

    // Status LEDs - none of the LEDs are lit in config mode
    if (triggerFiredWhileEditHeld) {
        hw.turnOffAllLeds();
    }
}

void AppStateMachine::processConfigMode() {
    // 1. Bottom Right touch (Pin 17) -> Halt MUX reads & MUX mode
    int raw17 = touchRead(Pins::TOUCH_BOTTOM_RIGHT);
    bool brNow = (raw17 >= TouchConfig::THRESHOLD_BOTTOM_RIGHT);
    if (brNow && !configTouchBrLast) {
        hw.haltMuxReads = true;
        configMgr.setMuxMode(MUX_NONE);
        configMgr.save();
        activeTarget = ConfigTarget::MUX_MODE;
        display.showText(configMgr.getMuxModeString());
    }
    configTouchBrLast = brNow;

    // 2. Top Right touch (Pin 23) -> Octave mode
    int raw23 = touchRead(Pins::TOUCH_TOP_RIGHT);
    bool trNow = (raw23 >= TouchConfig::THRESHOLD_TOP_RIGHT);
    if (trNow && !configTouchTrLast) {
        activeTarget = ConfigTarget::OCTAVE;
        display.showText(configMgr.getOctaveModeString());
    }
    configTouchTrLast = trNow;

    // 3. Top Left touch (Pin 19) -> Transpose
    int raw19 = touchRead(Pins::TOUCH_TOP_LEFT);
    bool tlNow = (raw19 >= TouchConfig::THRESHOLD_TOP_LEFT);
    if (tlNow && !configTouchTlLast) {
        activeTarget = ConfigTarget::TRANSPOSE;
        display.showNumber(configMgr.getTranspose());
    }
    configTouchTlLast = tlNow;

    // 4. Bottom Left touch (Pin 18) -> Expression mode
    int raw18 = touchRead(Pins::TOUCH_CHAOS_PAD);
    bool blNow = (raw18 >= TouchConfig::THRESHOLD_BOTTOM_LEFT);
    if (blNow && !configTouchBlLast) {
        activeTarget = ConfigTarget::EXPRESSION;
        display.showText(configMgr.getExpModeString());
    }
    configTouchBlLast = blNow;

    // 6. Footswitch 0 -> Footswitch 0 mode
    bool fs0Now = (digitalRead(Pins::FOOTSWITCH_0) == LOW);
    if (fs0Now && !configFs0Last) {
        activeTarget = ConfigTarget::FS0_MODE;
        display.showText(configMgr.getFs0Mode() ? "0-lc" : "0-mo");
    }
    configFs0Last = fs0Now;

    // 7. Footswitch 1 -> Footswitch 1 mode
    bool fs1Now = (digitalRead(Pins::FOOTSWITCH_1) == LOW);
    if (fs1Now && !configFs1Last) {
        activeTarget = ConfigTarget::FS1_MODE;
        display.showText(configMgr.getFs1Mode() ? "1-lc" : "1-mo");
    }
    configFs1Last = fs1Now;

    // 8. Adjust active target via Rotary Encoder
    int encStep = hw.readEncoderStep();
    if (encStep != 0) {
        switch (activeTarget) {
            case ConfigTarget::MUX_MODE:
                hw.haltMuxReads = true;
                configMgr.stepMuxMode(encStep);
                display.showText(configMgr.getMuxModeString());
                break;
            case ConfigTarget::OCTAVE:
                configMgr.stepOctaveMode(encStep);
                display.showText(configMgr.getOctaveModeString());
                break;
            case ConfigTarget::TRANSPOSE:
                configMgr.stepTranspose(encStep);
                display.showNumber(configMgr.getTranspose());
                break;
            case ConfigTarget::EXPRESSION:
                configMgr.stepExpMode(encStep);
                display.showText(configMgr.getExpModeString());
                break;
            case ConfigTarget::FS0_MODE:
                configMgr.toggleFs0Mode();
                hw.footSwitch0.mode = configMgr.getFs0Mode();
                display.showText(configMgr.getFs0Mode() ? "0-lc" : "0-mo");
                break;
            case ConfigTarget::FS1_MODE:
                configMgr.toggleFs1Mode();
                hw.footSwitch1.mode = configMgr.getFs1Mode();
                display.showText(configMgr.getFs1Mode() ? "1-lc" : "1-mo");
                break;
            case ConfigTarget::MIDI_CHANNEL: {
                uint8_t ch = configMgr.getMidiChannel();
                ch = (encStep > 0) ? ((ch >= 16) ? 1 : ch + 1) : ((ch <= 1) ? 16 : ch - 1);
                configMgr.setMidiChannel(ch);
                display.showChannel(ch);
                break;
            }
            case ConfigTarget::ANALOG_EXP:
                configMgr.stepExpCcNumber(encStep);
                display.showControlValue('c', configMgr.getExpCcNumber());
                break;
            case ConfigTarget::NONE:
                activeTarget = ConfigTarget::MIDI_CHANNEL;
                {
                    uint8_t ch = configMgr.getMidiChannel();
                    ch = (encStep > 0) ? ((ch >= 16) ? 1 : ch + 1) : ((ch <= 1) ? 16 : ch - 1);
                    configMgr.setMidiChannel(ch);
                    display.showChannel(ch);
                }
                break;
        }
    }

    // 9. Expression / MIDIpot calibration in SEnS state
    if (activeTarget == ConfigTarget::ANALOG_EXP) {
        int raw = analogRead(Pins::EXPRESSION_PEDAL);
        if (!expCalibrating) {
            if (abs(raw - initialExpReading) > 100) {
                expCalibrating = true;
                minExpReading = raw;
                maxExpReading = raw;
                lastRawExp = raw;
                display.showNumber(raw);
            }
        } else {
            if (raw < minExpReading) minExpReading = raw;
            if (raw > maxExpReading) maxExpReading = raw;
            if (abs(raw - lastRawExp) > 3) {
                display.showNumber(raw);
                lastRawExp = raw;
            }
        }
    }

    // None of the LEDs are lit in config mode
    hw.turnOffAllLeds();
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
    if (touchPollTimer >= 15) {
        touchPollTimer = 0;
        bool isPressed = (hw.touchButtons[trackTouchIdx]->read() == 127);
        if (isPressed && !trackBtnArmedLast[trackTouchIdx]) {
            int newLevel = trackMgr.toggleTrackArm(trackTouchIdx, ch);
            display.showNumber(newLevel);
        }
        trackBtnArmedLast[trackTouchIdx] = isPressed;
        trackTouchIdx = (trackTouchIdx + 1) % 3;
    }

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
    if (touchPollTimer >= 15) {
        touchPollTimer = 0;
        newVal = hw.touchButtons[ctrlTouchIdx]->send();
        if (newVal >= 0) {
            display.showControlValue('b', newVal);
        }
        ctrlTouchIdx = (ctrlTouchIdx + 1) % 4;
    }

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

    // 1. Expression Pedal (Pin 39)
    int newVal = hw.expressionPedal.send();
    if (newVal >= 0) {
        display.showControlValue('E', newVal);
    }

    // 2. Chord Display & Chaos Synth (Pin 18)
    if (configMgr.isMidiThruEnabled()) {
        display.updateChordDisplayIfChanged(midiRouter.getAnalyzer());
        uint8_t expMode = configMgr.getExpMode();
        if (expMode == EXP_CAOS || expMode == EXP_BOTH) {
            chaosEngine.update(ch, midiRouter.getDinChords(), midiRouter.getUsbChords());
        }
    }

    // 3. TOP PRIORITY: HALT ALL MUX READS if halted or if MUX_NONE
    if (hw.haltMuxReads || configMgr.isMuxHalted()) {
        return; // ABSOLUTELY NO READS ON PINS 20 OR 21!
    }

    // 4. Pin 20 - Single Pot
    if (configMgr.isPin20Pot()) {
        newVal = hw.mux0Pots[0].send();
        if (newVal >= 0) {
            display.showControlValue('h', newVal);
        }
    }

    // 5. Pin 21 - Single Pot
    if (configMgr.isPin21Pot()) {
        newVal = hw.mux1Pots[0].send();
        if (newVal >= 0) {
            display.showControlValue('H', newVal);
        }
    }

    // 6. MUX8 scanning (Pin 20 and/or Pin 21)
    bool mux0Is8 = configMgr.isPin20Mux8();
    bool mux1Is8 = configMgr.isPin21Mux8();
    if (mux0Is8 || mux1Is8) {
        for (uint8_t i = 0; i < 8; i++) {
            muxMgr.selectChannel(i);
            if (mux0Is8) {
                newVal = hw.mux0Pots[i].send();
                if (newVal >= 0) {
                    display.showControlValue('A', newVal);
                }
            }
            if (mux1Is8) {
                newVal = hw.mux1Pots[i].send();
                if (newVal >= 0) {
                    display.showControlValue('B', newVal);
                }
            }
        }
    }
}

void AppStateMachine::resendMuxPots() {
    if (hw.haltMuxReads || configMgr.isMuxHalted()) {
        return;
    }
    uint8_t ch = configMgr.getMidiChannel();
    if (configMgr.isPin20Pot()) {
        usbMIDI.sendControlChange(hw.mux0Pots[0].number, hw.mux0Pots[0].value, ch);
    } else if (configMgr.isPin20Mux8()) {
        for (int i = 0; i < 8; i++) {
            usbMIDI.sendControlChange(hw.mux0Pots[i].number, hw.mux0Pots[i].value, ch);
        }
    }
    if (configMgr.isPin21Pot()) {
        usbMIDI.sendControlChange(hw.mux1Pots[0].number, hw.mux1Pots[0].value, ch);
    } else if (configMgr.isPin21Mux8()) {
        for (int i = 0; i < 8; i++) {
            usbMIDI.sendControlChange(hw.mux1Pots[i].number, hw.mux1Pots[i].value, ch);
        }
    }
}
