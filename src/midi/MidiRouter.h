#pragma once
#include <Arduino.h>
#include <USBHost_t36.h>
#include <MIDI.h>
#include "ChordAnalyzer.h"
#include "../config/MidiConstants.h"
#include "../config/ConfigManager.h"

class MidiRouter {
public:
    MidiRouter();

    void begin();
    void process();

    ChordAnalyzer& getAnalyzer() { return analyzer; }
    const bool* getDinChords() const { return dinChords; }
    const bool* getUsbChords() const { return usbChords; }

    // Static event handlers for DIN MIDI
    static void handleNoteOff(byte channel, byte note, byte velocity);
    static void handleNoteOn(byte channel, byte note, byte velocity);
    static void handlePolyPressure(byte channel, byte note, byte pressure);
    static void handleControl(byte channel, byte control, byte value);
    static void handleProgram(byte channel, byte program);
    static void handleAfterTouch(byte channel, byte pressure);
    static void handlePitchBend(byte channel, int bend);

    // Static event handlers for USB Host MIDI
    static void handleUSBNoteOff(byte channel, byte note, byte velocity);
    static void handleUSBNoteOn(byte channel, byte note, byte velocity);
    static void handleUSBPolyPressure(byte channel, byte note, byte pressure);
    static void handleUSBControl(byte channel, byte control, byte value);
    static void handleUSBProgram(byte channel, byte program);
    static void handleUSBAfterTouch(byte channel, byte pressure);
    static void handleUSBPitchBend(byte channel, int bend);

private:
    USBHost teensyUSBHost;
    USBHub  hub1;
    USBHub  hub2;
    USBHub  hub3;
    USBHub  hub4;

    MIDIDevice_BigBuffer midi1;
    MIDIDevice_BigBuffer midi2;
    MIDIDevice_BigBuffer midi3;
    MIDIDevice_BigBuffer midi4;

    ChordAnalyzer analyzer;

    bool dinChords[12];
    bool usbChords[12];
};

extern MidiRouter midiRouter;
