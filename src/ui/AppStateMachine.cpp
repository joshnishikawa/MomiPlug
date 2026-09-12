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
      configTouchTimer(0),
      initialExpReading(0),
      expCalibrating(false),
      minExpReading(1023),
      maxExpReading(0),
      lastRawExp(0),
      ctrlTouchIdx(0),
      trackTouchIdx(0),
      trackBtnArmedLast{false, false, false},
      touchBr(Pins::TOUCH_BOTTOM_RIGHT, TouchConfig::THRESHOLD_BOTTOM_RIGHT, 15),
      touchTr(Pins::TOUCH_TOP_RIGHT, TouchConfig::THRESHOLD_TOP_RIGHT, 30),
      touchTl(Pins::TOUCH_TOP_LEFT, TouchConfig::THRESHOLD_TOP_LEFT, 30),
      touchCenter(Pins::TOUCH_CENTER, TouchConfig::THRESHOLD_CENTER, 30),
      touchBl(Pins::TOUCH_CHAOS_PAD, TouchConfig::THRESHOLD_BOTTOM_LEFT, 20),
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

    touchBr = TouchDebounce(Pins::TOUCH_BOTTOM_RIGHT, TouchConfig::THRESHOLD_BOTTOM_RIGHT, 15);
    touchBr.calibrate(TouchConfig::THRESHOLD_BOTTOM_RIGHT);
    touchTr = TouchDebounce(Pins::TOUCH_TOP_RIGHT, TouchConfig::THRESHOLD_TOP_RIGHT, 30);
    touchTl = TouchDebounce(Pins::TOUCH_TOP_LEFT, TouchConfig::THRESHOLD_TOP_LEFT, 30);
    touchCenter = TouchDebounce(Pins::TOUCH_CENTER, TouchConfig::THRESHOLD_CENTER, 30);
    touchBl = TouchDebounce(Pins::TOUCH_CHAOS_PAD, TouchConfig::THRESHOLD_BOTTOM_LEFT, 20);

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
    // "EDIT falling edge - stop all MUX reads display MIDI channel"
    if (hw.encoderButton.fell()) {
        hw.haltMuxReads = true;
        // Immediately halt MUX reads and ground analog lines so they cannot interfere with touch sensing
        pinMode(Pins::MUX_ANALOG_IN_0, INPUT_PULLDOWN);
        pinMode(Pins::MUX_ANALOG_IN_1, INPUT_PULLDOWN);
        digitalWrite(Pins::MUX_SEL_A, LOW);
        digitalWrite(Pins::MUX_SEL_B, LOW);
        digitalWrite(Pins::MUX_SEL_C, LOW);
        digitalWrite(Pins::MUX_SEL_D, LOW);

        display.showChannel(configMgr.getMidiChannel());

        triggerFiredWhileEditHeld = false;
        channelChangedWhileEditHeld = false;
        activeTarget = ConfigTarget::NONE;
        hw.resetEncoder();

        // Calibrate touchBr to ensure it works reliably in the current environment
        touchBr.calibrate(TouchConfig::THRESHOLD_BOTTOM_RIGHT);
        touchBr.reset();
        touchTr.reset();
        touchTl.reset();
        touchCenter.reset();
        touchBl.reset();
        configFs0Last = (digitalRead(Pins::FOOTSWITCH_0) == LOW);
        configFs1Last = (digitalRead(Pins::FOOTSWITCH_1) == LOW);
        configExpLast = (analogRead(Pins::EXPRESSION_PEDAL) >= ExpressionConfig::KILLSWITCH_THRESHOLD);
        configTouchTimer = 0;

        Serial.printf("[EDIT] Pin 17 baseline: %d, thresh: %d (cfg: %d)\n",
                      touchBr.baseline, touchBr.onThreshold, TouchConfig::THRESHOLD_BOTTOM_RIGHT);
        return;
    }

    // 3. Normal mode (CONTROL or TRACK) with EDIT button held down
    // "EDIT low - detect input from footswitches, expression pedal, encoder, pins 17, 18, 19, 22 and 23
    //  and display appropriate info on 7-segment display."
    if (hw.encoderButton.read() == LOW) {
        checkConfigTriggersWhileEditHeld();
        return;
    }

    // 4. EDIT button released
    // "EDIT rising edge - if none of those inputs were triggered, toggle TRACK/CONTROL modes.
    //  If any of those inputs were triggered, we enter the config mode that that input corresponds to."
    if (hw.encoderButton.rose()) {
        if (!triggerFiredWhileEditHeld) {
            // No inputs were triggered: toggle TRACK/CONTROL modes
            if (currentMode == OperatingMode::CONTROL) {
                currentMode = OperatingMode::TRACK;
                trackMgr.sendAllTrackLevels(configMgr.getMidiChannel());
                display.showText("trac");
            } else {
                currentMode = OperatingMode::CONTROL;
                display.showText("ctrl");
                resendMuxPots();
            }

            // Restore MUX reads according to config
            hw.haltMuxReads = configMgr.isMuxHalted();
            return;
        } else {
            // An input was triggered: enter the config mode corresponding to that input
            previousMode = currentMode;
            currentMode = OperatingMode::CONFIG;
            hw.turnOffAllLeds();
            hw.haltMuxReads = true;
            pinMode(Pins::MUX_ANALOG_IN_0, INPUT_PULLDOWN);
            pinMode(Pins::MUX_ANALOG_IN_1, INPUT_PULLDOWN);

            if (activeTarget == ConfigTarget::ANALOG_EXP) {
                display.showText("SEnS");
                initialExpReading = analogRead(Pins::EXPRESSION_PEDAL);
                expCalibrating = false;
                minExpReading = 1023;
                maxExpReading = 0;
            }

            // Reset touch states for config mode
            touchBr.reset();
            touchTr.reset();
            touchTl.reset();
            touchCenter.reset();
            touchBl.reset();
            configFs0Last = (digitalRead(Pins::FOOTSWITCH_0) == LOW);
            configFs1Last = (digitalRead(Pins::FOOTSWITCH_1) == LOW);
            configExpLast = (analogRead(Pins::EXPRESSION_PEDAL) >= ExpressionConfig::KILLSWITCH_THRESHOLD);
            configTouchTimer = 0;
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
    // 1. Rotary Encoder while EDIT is held
    int encStep = hw.readEncoderStep();
    if (encStep != 0) {
        triggerFiredWhileEditHeld = true;
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
            case ConfigTarget::USB_FX:
                configMgr.toggleUsbFxWet();
                display.showText(configMgr.isUsbFxWet() ? " wet" : " dry");
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
            case ConfigTarget::ANALOG_EXP:
                configMgr.stepExpCcNumber(encStep);
                display.showControlValue('c', configMgr.getExpCcNumber());
                break;
        }
    }

    // 2. Capacitive touch checks (rate-limited to 15ms with 3-sample median and hysteresis)
    if (configTouchTimer >= 15) {
        configTouchTimer = 0;

        // Bottom Right touch (Pin 17) -> MUX mode
        if (touchBr.update()) {
            hw.haltMuxReads = true;
            activeTarget = ConfigTarget::MUX_MODE;
            triggerFiredWhileEditHeld = true;
            display.showText(configMgr.getMuxModeString());
            hw.turnOffAllLeds();
            Serial.printf("[TOUCH17 TRIGGERED] MUX Mode: %s\n", configMgr.getMuxModeString());
            return;
        }

        // Top Right touch (Pin 23) -> Octave mode
        if (touchTr.update()) {
            activeTarget = ConfigTarget::OCTAVE;
            triggerFiredWhileEditHeld = true;
            display.showText(configMgr.getOctaveModeString());
            hw.turnOffAllLeds();
            return;
        }

        // Top Left touch (Pin 19) -> Transpose
        if (touchTl.update()) {
            activeTarget = ConfigTarget::TRANSPOSE;
            triggerFiredWhileEditHeld = true;
            display.showNumber(configMgr.getTranspose());
            hw.turnOffAllLeds();
            return;
        }

        // Center touch (Pin 22) -> USB FX (wet, dry)
        if (touchCenter.update()) {
            activeTarget = ConfigTarget::USB_FX;
            triggerFiredWhileEditHeld = true;
            configMgr.toggleUsbFxWet();
            display.showText(configMgr.isUsbFxWet() ? " wet" : " dry");
            hw.turnOffAllLeds();
            return;
        }

        // Bottom Left touch (Pin 18) -> Expression / MIDI FX mode
        if (touchBl.update()) {
            activeTarget = ConfigTarget::EXPRESSION;
            triggerFiredWhileEditHeld = true;
            display.showText(configMgr.getExpModeString());
            hw.turnOffAllLeds();
            return;
        }
    }

    // 3. Footswitches
    bool fs0Now = (digitalRead(Pins::FOOTSWITCH_0) == LOW);
    if (fs0Now && !configFs0Last) {
        activeTarget = ConfigTarget::FS0_MODE;
        triggerFiredWhileEditHeld = true;
        configMgr.toggleFs0Mode();
        hw.footSwitch0.mode = configMgr.getFs0Mode();
        display.showText(configMgr.getFs0Mode() ? "0-lc" : "0-mo");
    }
    configFs0Last = fs0Now;

    bool fs1Now = (digitalRead(Pins::FOOTSWITCH_1) == LOW);
    if (fs1Now && !configFs1Last) {
        activeTarget = ConfigTarget::FS1_MODE;
        triggerFiredWhileEditHeld = true;
        configMgr.toggleFs1Mode();
        hw.footSwitch1.mode = configMgr.getFs1Mode();
        display.showText(configMgr.getFs1Mode() ? "1-lc" : "1-mo");
    }
    configFs1Last = fs1Now;

    // 4. Expression pedal / MIDIpot analog reading
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

    // Status LEDs - none of the LEDs are lit in config mode
    hw.turnOffAllLeds();

    // Stream live touchPin 17 readings to Serial while EDIT is held
    static elapsedMillis dbgTimer;
    if (dbgTimer >= 150) {
        dbgTimer = 0;
        int raw17 = flickerTouchRead(Pins::TOUCH_BOTTOM_RIGHT);
        Serial.printf("[TOUCH17] raw: %d | base: %d | thresh: %d | state: %d | target: %d\n",
                      raw17, touchBr.baseline, touchBr.onThreshold, (int)touchBr.state, (int)activeTarget);
    }
}

void AppStateMachine::processConfigMode() {
    // Suspend trigger updates while encoder button is pressed down (user is clicking to save and exit)
    if (hw.encoderButton.read() == LOW) {
        return;
    }

    // 1. Adjust active target via Rotary Encoder
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
            case ConfigTarget::USB_FX:
                configMgr.toggleUsbFxWet();
                display.showText(configMgr.isUsbFxWet() ? " wet" : " dry");
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
            case ConfigTarget::MIDI_CHANNEL:
            case ConfigTarget::NONE:
                activeTarget = ConfigTarget::MIDI_CHANNEL;
                {
                    uint8_t ch = configMgr.getMidiChannel();
                    ch = (encStep > 0) ? ((ch >= 16) ? 1 : ch + 1) : ((ch <= 1) ? 16 : ch - 1);
                    configMgr.setMidiChannel(ch);
                    display.showChannel(ch);
                }
                break;
            case ConfigTarget::ANALOG_EXP:
                configMgr.stepExpCcNumber(encStep);
                display.showControlValue('c', configMgr.getExpCcNumber());
                break;
        }
    }

    // 2. Detect touch and physical inputs while in config mode
    // Rate-limit capacitive touch polling to 15ms
    if (configTouchTimer >= 15) {
        configTouchTimer = 0;

        // Bottom Right touch (Pin 17) -> MUX mode
        if (touchBr.update()) {
            hw.haltMuxReads = true;
            activeTarget = ConfigTarget::MUX_MODE;
            display.showText(configMgr.getMuxModeString());
            hw.turnOffAllLeds();
            Serial.printf("[CONFIG TOUCH17 TRIGGERED] MUX Mode: %s\n", configMgr.getMuxModeString());
            return;
        }

        // Top Right touch (Pin 23) -> Octave mode
        if (touchTr.update()) {
            activeTarget = ConfigTarget::OCTAVE;
            display.showText(configMgr.getOctaveModeString());
            hw.turnOffAllLeds();
            return;
        }

        // Top Left touch (Pin 19) -> Transpose
        if (touchTl.update()) {
            activeTarget = ConfigTarget::TRANSPOSE;
            display.showNumber(configMgr.getTranspose());
            hw.turnOffAllLeds();
            return;
        }

        // Center touch (Pin 22) -> USB FX (wet, dry)
        if (touchCenter.update()) {
            activeTarget = ConfigTarget::USB_FX;
            configMgr.toggleUsbFxWet();
            display.showText(configMgr.isUsbFxWet() ? " wet" : " dry");
            hw.turnOffAllLeds();
            return;
        }

        // Bottom Left touch (Pin 18) -> Expression mode
        if (touchBl.update()) {
            activeTarget = ConfigTarget::EXPRESSION;
            display.showText(configMgr.getExpModeString());
            hw.turnOffAllLeds();
            return;
        }
    }

    // Footswitch 0 -> Footswitch 0 mode
    bool fs0Now = (digitalRead(Pins::FOOTSWITCH_0) == LOW);
    if (fs0Now && !configFs0Last) {
        activeTarget = ConfigTarget::FS0_MODE;
        configMgr.toggleFs0Mode();
        hw.footSwitch0.mode = configMgr.getFs0Mode();
        display.showText(configMgr.getFs0Mode() ? "0-lc" : "0-mo");
    }
    configFs0Last = fs0Now;

    // Footswitch 1 -> Footswitch 1 mode
    bool fs1Now = (digitalRead(Pins::FOOTSWITCH_1) == LOW);
    if (fs1Now && !configFs1Last) {
        activeTarget = ConfigTarget::FS1_MODE;
        configMgr.toggleFs1Mode();
        hw.footSwitch1.mode = configMgr.getFs1Mode();
        display.showText(configMgr.getFs1Mode() ? "1-lc" : "1-mo");
    }
    configFs1Last = fs1Now;

    // 3. Expression / MIDIpot calibration in SEnS state
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
    display.updateChordDisplayIfChanged(midiRouter.getAnalyzer());
    uint8_t expMode = configMgr.getExpMode();
    if (expMode == EXP_CAOS || expMode == EXP_BOTH || expMode == EXP_ECHO) {
        if (configMgr.isUsbFxWet()) {
            chaosEngine.update(ch, midiRouter.getDinChords(), midiRouter.getUsbChords());
        } else {
            bool emptyChords[12] = {false};
            chaosEngine.update(ch, midiRouter.getDinChords(), emptyChords);
        }
    }

    // 3. TOP PRIORITY: HALT ALL MUX READS if halted or if MUX_NONE
    if (hw.haltMuxReads || configMgr.isMuxHalted()) {
        return; // ABSOLUTELY NO READS ON PINS 20 OR 21!
    }

    // Rate-limit active MUX reads to 20ms (50 Hz) to eliminate ADC noise and USB MIDI flooding
    static elapsedMillis muxPollTimer;
    if (muxPollTimer < 20) {
        return;
    }
    muxPollTimer = 0;

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
