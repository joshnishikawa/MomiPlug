#include "MidiRouter.h"
#include "MidiEcho.h"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

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
    // DIN MIDI setup on Serial1 (Pins 0 RX1 & 1 TX1)
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff(); // Disable default raw pass-through so our handlers route transposed/FX notes
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
        while (Serial1.available() > 0) {
            MIDI.read();
        }
    } else {
        while (Serial1.available() > 0) {
            Serial1.read();
        }
    }

    // Drain USB client MIDI packets from PC host so endpoint buffers don't stall
    while (usbMIDI.read()) {}

    // Flush pending USB MIDI packets immediately
    usbMIDI.send_now();

    // Service active MIDI echo delay repeats
    midiEcho.update();
}

// ====================================================================
// DIN MIDI Event Handlers
// ====================================================================

void MidiRouter::handleNoteOff(byte channel, byte note, byte velocity) {
    int transNote = constrain(static_cast<int>(note) + configMgr.getTranspose(), 0, 127);
    midiRouter.analyzer.noteOff(transNote);

    usbMIDI.sendNoteOff(transNote, 0, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendNoteOff(transNote, 0, channel, UsbCable::CHORD_PORT);
    MIDI.sendNoteOff(transNote, 0, channel); // Forward to DIN Out

    uint8_t oct = configMgr.getOctaveMode();
    if (oct == OCTAVE_L_NONE || oct == OCTAVE_L_H) {
        if (transNote >= 12) {
            usbMIDI.sendNoteOff(transNote - 12, 0, channel, UsbCable::DEFAULT_PORT);
            usbMIDI.sendNoteOff(transNote - 12, 0, channel, UsbCable::CHORD_PORT);
            MIDI.sendNoteOff(transNote - 12, 0, channel);
        }
    }
    if (oct == OCTAVE_NONE_H || oct == OCTAVE_L_H) {
        if (transNote <= 115) {
            usbMIDI.sendNoteOff(transNote + 12, 0, channel, UsbCable::DEFAULT_PORT);
            usbMIDI.sendNoteOff(transNote + 12, 0, channel, UsbCable::CHORD_PORT);
            MIDI.sendNoteOff(transNote + 12, 0, channel);
        }
    }

    midiRouter.dinChords[transNote % 12] = false;
    midiEcho.onNoteOff(channel, static_cast<uint8_t>(transNote), UsbCable::DEFAULT_PORT);
    usbMIDI.send_now();
}

void MidiRouter::handleNoteOn(byte channel, byte note, byte velocity) {
    if (velocity == 0) {
        handleNoteOff(channel, note, 0);
        return;
    }

    int transNote = constrain(static_cast<int>(note) + configMgr.getTranspose(), 0, 127);
    midiRouter.analyzer.noteOn(transNote, velocity);

    usbMIDI.sendNoteOn(transNote, velocity, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendNoteOn(transNote, velocity, channel, UsbCable::CHORD_PORT);
    MIDI.sendNoteOn(transNote, velocity, channel); // Forward to DIN Out

    uint8_t oct = configMgr.getOctaveMode();
    if (oct == OCTAVE_L_NONE || oct == OCTAVE_L_H) {
        if (transNote >= 12) {
            usbMIDI.sendNoteOn(transNote - 12, velocity, channel, UsbCable::DEFAULT_PORT);
            usbMIDI.sendNoteOn(transNote - 12, velocity, channel, UsbCable::CHORD_PORT);
            MIDI.sendNoteOn(transNote - 12, velocity, channel);
        }
    }
    if (oct == OCTAVE_NONE_H || oct == OCTAVE_L_H) {
        if (transNote <= 115) {
            usbMIDI.sendNoteOn(transNote + 12, velocity, channel, UsbCable::DEFAULT_PORT);
            usbMIDI.sendNoteOn(transNote + 12, velocity, channel, UsbCable::CHORD_PORT);
            MIDI.sendNoteOn(transNote + 12, velocity, channel);
        }
    }

    midiRouter.dinChords[transNote % 12] = true;
    midiEcho.onNoteOn(channel, static_cast<uint8_t>(transNote), velocity, UsbCable::DEFAULT_PORT);
    usbMIDI.send_now();
}

void MidiRouter::handlePolyPressure(byte channel, byte note, byte pressure) {
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::CHORD_PORT);
    MIDI.sendAfterTouch(note, pressure, channel);
    usbMIDI.send_now();
}

void MidiRouter::handleControl(byte channel, byte control, byte value) {
    if (control == MidiCC::SUSTAIN_PEDAL) {
        midiRouter.analyzer.sustainControl(value);
    }
    usbMIDI.sendControlChange(control, value, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendControlChange(control, value, channel, UsbCable::CHORD_PORT);
    MIDI.sendControlChange(control, value, channel);
    usbMIDI.send_now();
}

void MidiRouter::handleProgram(byte channel, byte program) {
    usbMIDI.sendProgramChange(program, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendProgramChange(program, channel, UsbCable::CHORD_PORT);
    MIDI.sendProgramChange(program, channel);
    usbMIDI.send_now();
}

void MidiRouter::handleAfterTouch(byte channel, byte pressure) {
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::CHORD_PORT);
    MIDI.sendAfterTouch(pressure, channel);
    usbMIDI.send_now();
}

void MidiRouter::handlePitchBend(byte channel, int bend) {
    usbMIDI.sendPitchBend(bend, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendPitchBend(bend, channel, UsbCable::CHORD_PORT);
    MIDI.sendPitchBend(bend, channel);
    usbMIDI.send_now();
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
    usbMIDI.send_now();
}

void MidiRouter::handleUSBNoteOn(byte channel, byte note, byte velocity) {
    if (velocity == 0) {
        handleUSBNoteOff(channel, note, 0);
        return;
    }

    usbMIDI.sendNoteOn(note, velocity, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendNoteOn(note, velocity, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendNoteOn(note, velocity, channel, UsbCable::ROUTE_PORT);
    MIDI.sendNoteOn(note, velocity, channel); // Forward to DIN Out

    midiRouter.usbChords[note % 12] = true;
    usbMIDI.send_now();
}

void MidiRouter::handleUSBPolyPressure(byte channel, byte note, byte pressure) {
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendPolyPressure(note, pressure, channel, UsbCable::ROUTE_PORT);
    MIDI.sendAfterTouch(note, pressure, channel);
    usbMIDI.send_now();
}

void MidiRouter::handleUSBControl(byte channel, byte control, byte value) {
    usbMIDI.sendControlChange(control, value, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendControlChange(control, value, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendControlChange(control, value, channel, UsbCable::ROUTE_PORT);
    MIDI.sendControlChange(control, value, channel);
    usbMIDI.send_now();
}

void MidiRouter::handleUSBProgram(byte channel, byte program) {
    usbMIDI.sendProgramChange(program, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendProgramChange(program, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendProgramChange(program, channel, UsbCable::ROUTE_PORT);
    MIDI.sendProgramChange(program, channel);
    usbMIDI.send_now();
}

void MidiRouter::handleUSBAfterTouch(byte channel, byte pressure) {
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendAfterTouch(pressure, channel, UsbCable::ROUTE_PORT);
    MIDI.sendAfterTouch(pressure, channel);
    usbMIDI.send_now();
}

void MidiRouter::handleUSBPitchBend(byte channel, int bend) {
    usbMIDI.sendPitchBend(bend, channel, UsbCable::DEFAULT_PORT);
    usbMIDI.sendPitchBend(bend, channel, UsbCable::SYNTH_PORT);
    usbMIDI.sendPitchBend(bend, channel, UsbCable::ROUTE_PORT);
    MIDI.sendPitchBend(bend, channel);
    usbMIDI.send_now();
}
