#include "MidiRouter.h"

MIDI_CREATE_DEFAULT_INSTANCE();

MidiRouter midiRouter;

MidiRouter::MidiRouter()
    : hub1(teensyUSBHost),
      hub2(teensyUSBHost),
      hub3(teensyUSBHost),
      hub4(teensyUSBHost),
      midi1(teensyUSBHost),
      midi2(teensyUSBHost),
      midi3(teensyUSBHost),
      midi4(teensyUSBHost),
      dinChords{false},
      usbChords{false} {}

void MidiRouter::begin() {
    // DIN MIDI setup
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleAfterTouchPoly(handlePolyPressure);
    MIDI.setHandleControlChange(handleControl);
    MIDI.setHandleProgramChange(handleProgram);
    MIDI.setHandleAfterTouchChannel(handleAfterTouch);
    MIDI.setHandlePitchBend(handlePitchBend);

    // USB Host controller start
    teensyUSBHost.begin();

    // Attach handlers to all 4 USB Host MIDI BigBuffer instances
    auto attachUSBHandlers = [](MIDIDevice_BigBuffer &dev) {
        dev.setHandleNoteOff(handleUSBNoteOff);
        dev.setHandleNoteOn(handleUSBNoteOn);
        dev.setHandleControlChange(handleUSBControl);
        dev.setHandleProgramChange(handleUSBProgram);
        dev.setHandleAfterTouchChannel(handleUSBAfterTouch);
        dev.setHandlePitchChange(handleUSBPitchBend);
        dev.setHandleAfterTouchPoly(handleUSBPolyPressure);
    };

    attachUSBHandlers(midi1);
    attachUSBHandlers(midi2);
    attachUSBHandlers(midi3);
    attachUSBHandlers(midi4);
}

void MidiRouter::process() {
    // Service USB Host hardware
    teensyUSBHost.Task();

    // Drain USB Host MIDI packets
    while (midi1.read()) {}
    while (midi2.read()) {}
    while (midi3.read()) {}
    while (midi4.read()) {}

    // Process DIN MIDI if Thru is enabled, otherwise flush Serial1
    if (configMgr.isMidiThruEnabled()) {
        while (MIDI.read()) {}
    } else {
        while (Serial1.available()) {
            Serial1.read();
        }
    }
}

// ====================================================================
// DIN MIDI Event Handlers
// ====================================================================

void MidiRouter::handleNoteOff(byte channel, byte note, byte velocity) {
    midiRouter.analyzer.noteOff(note);

    usbMIDI.sendNoteOff(note, 0, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendNoteOff(note, 0, channel, UsbCable::CHORD_PORT);

    midiRouter.dinChords[note % 12] = false;
}

void MidiRouter::handleNoteOn(byte channel, byte note, byte velocity) {
    midiRouter.analyzer.noteOn(note, velocity);

    usbMIDI.sendNoteOn(note, velocity, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendNoteOn(note, velocity, channel, UsbCable::CHORD_PORT);

    midiRouter.dinChords[note % 12] = (velocity != 0);
}

void MidiRouter::handlePolyPressure(byte channel, byte note, byte pressure) {
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::CHORD_PORT);
}

void MidiRouter::handleControl(byte channel, byte control, byte value) {
    if (control == MidiCC::SUSTAIN_PEDAL) {
        midiRouter.analyzer.sustainControl(value);
    }
    usbMIDI.sendControlChange(control, value, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendControlChange(control, value, channel, UsbCable::CHORD_PORT);
}

void MidiRouter::handleProgram(byte channel, byte program) {
    usbMIDI.sendProgramChange(program, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendProgramChange(program, channel, UsbCable::CHORD_PORT);
}

void MidiRouter::handleAfterTouch(byte channel, byte pressure) {
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::CHORD_PORT);
}

void MidiRouter::handlePitchBend(byte channel, int bend) {
    usbMIDI.sendPitchBend(bend, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendPitchBend(bend, channel, UsbCable::CHORD_PORT);
}

// ====================================================================
// USB Host MIDI Event Handlers
// ====================================================================

void MidiRouter::handleUSBNoteOff(byte channel, byte note, byte velocity) {
    usbMIDI.sendNoteOff(note, 0, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendNoteOff(note, 0, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendNoteOff(note, 0, channel, UsbCable::ROUTE_PORT);
    MIDI.sendNoteOff(note, 0, channel); // Forward to DIN Out

    midiRouter.usbChords[note % 12] = false;
}

void MidiRouter::handleUSBNoteOn(byte channel, byte note, byte velocity) {
    usbMIDI.sendNoteOn(note, velocity, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendNoteOn(note, velocity, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendNoteOn(note, velocity, channel, UsbCable::ROUTE_PORT);
    MIDI.sendNoteOn(note, velocity, channel); // Forward to DIN Out

    midiRouter.usbChords[note % 12] = (velocity != 0);
}

void MidiRouter::handleUSBPolyPressure(byte channel, byte note, byte pressure) {
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::ROUTE_PORT);
    MIDI.sendAfterTouch(note, pressure, channel);
}

void MidiRouter::handleUSBControl(byte channel, byte control, byte value) {
    usbMIDI.sendControlChange(control, value, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendControlChange(control, value, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendControlChange(control, value, channel, UsbCable::ROUTE_PORT);
    MIDI.sendControlChange(control, value, channel);
}

void MidiRouter::handleUSBProgram(byte channel, byte program) {
    usbMIDI.sendProgramChange(program, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendProgramChange(program, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendProgramChange(program, channel, UsbCable::ROUTE_PORT);
    MIDI.sendProgramChange(program, channel);
}

void MidiRouter::handleUSBAfterTouch(byte channel, byte pressure) {
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::ROUTE_PORT);
    MIDI.sendAfterTouch(pressure, channel);
}

void MidiRouter::handleUSBPitchBend(byte channel, int bend) {
    usbMIDI.sendPitchBend(bend, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendPitchBend(bend, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendPitchBend(bend, channel, UsbCable::ROUTE_PORT);
    MIDI.sendPitchBend(bend, channel);
}
