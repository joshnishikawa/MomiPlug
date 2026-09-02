#include "HardwareControls.h"

HardwareControls hw;

HardwareControls::HardwareControls()
    : encoderButton(Pins::ENCODER_BUTTON, 50),
      encoder(Pins::ENCODER_A, Pins::ENCODER_B),
      encoderLevel(0),
      touchTopLeft(Pins::TOUCH_TOP_LEFT, MidiCC::TOUCH_TOP_LEFT, LATCH, TOUCH),
      touchCenter(Pins::TOUCH_CENTER, MidiCC::TOUCH_CENTER, LATCH, TOUCH),
      touchTopRight(Pins::TOUCH_TOP_RIGHT, MidiCC::TOUCH_TOP_RIGHT, LATCH, TOUCH),
      touchBottomRight(Pins::TOUCH_BOTTOM_RIGHT, MidiCC::TOUCH_BOTTOM_RIGHT, LATCH, TOUCH),
      footSwitch1(Pins::FOOTSWITCH_1, MidiCC::FOOTSWITCH_1, MOMENTARY),
      footSwitch0(Pins::FOOTSWITCH_0, MidiCC::FOOTSWITCH_0, LATCH),
      expressionPedal(Pins::EXPRESSION_PEDAL, MidiCC::EXPRESSION_PEDAL),
      mux0Pots{
          MIDIpot(Pins::MUX_ANALOG_IN_0, MidiCC::MUX0_POT_BASE + 0),
          MIDIpot(Pins::MUX_ANALOG_IN_0, MidiCC::MUX0_POT_BASE + 1),
          MIDIpot(Pins::MUX_ANALOG_IN_0, MidiCC::MUX0_POT_BASE + 2),
          MIDIpot(Pins::MUX_ANALOG_IN_0, MidiCC::MUX0_POT_BASE + 3),
          MIDIpot(Pins::MUX_ANALOG_IN_0, MidiCC::MUX0_POT_BASE + 4),
          MIDIpot(Pins::MUX_ANALOG_IN_0, MidiCC::MUX0_POT_BASE + 5),
          MIDIpot(Pins::MUX_ANALOG_IN_0, MidiCC::MUX0_POT_BASE + 6),
          MIDIpot(Pins::MUX_ANALOG_IN_0, MidiCC::MUX0_POT_BASE + 7)
      },
      mux1Pots{
          MIDIpot(Pins::MUX_ANALOG_IN_1, MidiCC::MUX1_POT_BASE + 0),
          MIDIpot(Pins::MUX_ANALOG_IN_1, MidiCC::MUX1_POT_BASE + 1),
          MIDIpot(Pins::MUX_ANALOG_IN_1, MidiCC::MUX1_POT_BASE + 2),
          MIDIpot(Pins::MUX_ANALOG_IN_1, MidiCC::MUX1_POT_BASE + 3),
          MIDIpot(Pins::MUX_ANALOG_IN_1, MidiCC::MUX1_POT_BASE + 4),
          MIDIpot(Pins::MUX_ANALOG_IN_1, MidiCC::MUX1_POT_BASE + 5),
          MIDIpot(Pins::MUX_ANALOG_IN_1, MidiCC::MUX1_POT_BASE + 6),
          MIDIpot(Pins::MUX_ANALOG_IN_1, MidiCC::MUX1_POT_BASE + 7)
      }
{
    touchButtons[0] = &touchTopLeft;
    touchButtons[1] = &touchCenter;
    touchButtons[2] = &touchTopRight;
    touchButtons[3] = &touchBottomRight;
}

void HardwareControls::begin() {
    pinMode(Pins::ENCODER_BUTTON, INPUT_PULLUP);

    touchTopLeft.setThreshold(TouchConfig::THRESHOLD_TOP_LEFT);
    touchCenter.setThreshold(TouchConfig::THRESHOLD_CENTER);
    touchTopRight.setThreshold(TouchConfig::THRESHOLD_TOP_RIGHT);
    touchBottomRight.setThreshold(TouchConfig::THRESHOLD_BOTTOM_RIGHT);

    expressionPedal.inputRange(ExpressionConfig::INPUT_MIN, ExpressionConfig::INPUT_MAX);

    initLeds();
}

void HardwareControls::initLeds() {
    pinMode(Pins::LED_TOP_LEFT, OUTPUT);
    pinMode(Pins::LED_CENTER, OUTPUT);
    pinMode(Pins::LED_TOP_RIGHT, OUTPUT);
    pinMode(Pins::LED_FOOTSWITCH_0, OUTPUT);
    pinMode(Pins::LED_FOOTSWITCH_1, OUTPUT);
    pinMode(Pins::LED_ONBOARD, OUTPUT);
}

void HardwareControls::updateModesFromConfig(const MomiConfig& cfg) {
    footSwitch0.mode = cfg.fs0Mode;
    footSwitch1.mode = cfg.fs1Mode;
    expressionPedal.killSwitch = cfg.expKillSwitch;
}

int HardwareControls::sendEncoderMidi(uint8_t channel) {
    int incdec = encoder.read();
    if (incdec >= 4 && encoderLevel < 127) {
        encoderLevel += 1;
        encoder.write(0);
        usbMIDI.sendControlChange(MidiCC::ENCODER_PARAM, encoderLevel, channel);
        return encoderLevel;
    }
    else if (incdec <= -4 && encoderLevel > 0) {
        encoderLevel -= 1;
        encoder.write(0);
        usbMIDI.sendControlChange(MidiCC::ENCODER_PARAM, encoderLevel, channel);
        return encoderLevel;
    }
    else if (incdec >= 4 || incdec <= -4) {
        encoder.write(0);
        return -1;
    }
    return -1;
}

int HardwareControls::readEncoderStep() {
    int delta = encoder.read();
    if (delta >= 4) {
        encoder.write(0);
        return 1;
    } else if (delta <= -4) {
        encoder.write(0);
        return -1;
    }
    return 0;
}

void HardwareControls::resetEncoder() {
    encoder.write(0);
}
