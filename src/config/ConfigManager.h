#pragma once
#include <Arduino.h>
#include <EEPROM.h>

constexpr uint32_t MOMI_CONFIG_MAGIC = 0x4D4F4D38; // 'MOM8'

// MUX Operating Modes (8 modes from TODO.txt)
enum MuxMode : uint8_t {
    MUX_NONE   = 0, // '----' Both MUX pins ignored
    MUX_A_NONE = 1, // 'A---' Pin 20 single pot, pin 21 ignored
    MUX_A_A    = 2, // 'A--A' Pins 20 & 21 each single pot
    MUX_8_NONE = 3, // '8---' Pin 20 reads MUX8, pin 21 ignored
    MUX_8_A    = 4, // '8--A' Pin 20 reads MUX8, pin 21 single pot
    MUX_8_8    = 5, // '8--8' Pins 20 & 21 each read MUX8
    MUX_SPI    = 6, // 'SPI-' SPI on MOSI 11, MISO 12, CS 20, SCK 14
    MUX_SPI_A  = 7  // 'SPIA' SPI on pin 20, single pot on pin 21
};

// Octave modes (4 modes from TODO.txt)
enum OctaveMode : uint8_t {
    OCTAVE_L_NONE = 0, // 'ocL-' -12
    OCTAVE_NONE_H = 1, // 'oc-H' +12
    OCTAVE_L_H    = 2, // 'ocLH' -12 and +12
    OCTAVE_OFF    = 3  // 'oc--' only note played
};

// Expression modes (4 modes from TODO.txt)
enum ExpMode : uint8_t {
    EXP_CAOS = 0, // 'CAOS'
    EXP_ECHO = 1, // 'ECHO'
    EXP_BOTH = 2, // 'both'
    EXP_NONE = 3  // '----'
};

struct MomiConfig {
    uint32_t magic;          // Magic number for EEPROM validity check
    uint8_t  midiChannel;    // Active MIDI Channel (1 - 16)
    bool     readMIDIthru;   // Pass-through incoming MIDI
    uint8_t  muxMode;        // MuxMode (0..7)
    uint8_t  octaveMode;     // OctaveMode (0..3)
    int8_t   transpose;      // Transpose (-12..+12)
    uint8_t  expMode;        // ExpMode (0..3)
    uint8_t  fs0Mode;        // Footswitch 0 mode (0: MOMENTARY, 1: LATCH)
    uint8_t  fs1Mode;        // Footswitch 1 mode (0: MOMENTARY, 1: LATCH)
    uint8_t  expKillSwitch;  // Expression pedal killswitch mode (0: off, 1: on)
    uint8_t  expCcNumber;    // Expression CC number (default 85)
    uint16_t expInLo;        // Expression calibration lower bound (default 10)
    uint16_t expInHi;        // Expression calibration upper bound (default 900)
};

class ConfigManager {
public:
    void load();
    void save();
    void resetDefaults();

    MomiConfig& get() { return config; }
    const MomiConfig& get() const { return config; }

    uint8_t getMidiChannel() const { return config.midiChannel; }
    void setMidiChannel(uint8_t ch) { config.midiChannel = constrain(ch, 1, 16); }

    bool isMidiThruEnabled() const { return config.readMIDIthru; }
    void toggleMidiThru() { config.readMIDIthru = !config.readMIDIthru; }

    uint8_t getMuxMode() const { return config.muxMode; }
    void setMuxMode(uint8_t mode) { config.muxMode = mode % 8; }
    uint8_t stepMuxMode(int delta);
    const char* getMuxModeString() const;

    // Helper queries for MUX pin routing
    bool isMuxHalted() const { return config.muxMode == MUX_NONE; }
    bool isPin20Pot() const { return config.muxMode == MUX_A_NONE || config.muxMode == MUX_A_A; }
    bool isPin20Mux8() const { return config.muxMode == MUX_8_NONE || config.muxMode == MUX_8_A || config.muxMode == MUX_8_8; }
    bool isPin20Spi() const { return config.muxMode == MUX_SPI || config.muxMode == MUX_SPI_A; }
    bool isPin21Pot() const { return config.muxMode == MUX_A_A || config.muxMode == MUX_8_A || config.muxMode == MUX_SPI_A; }
    bool isPin21Mux8() const { return config.muxMode == MUX_8_8; }

    // Octave mode
    uint8_t getOctaveMode() const { return config.octaveMode; }
    void setOctaveMode(uint8_t mode) { config.octaveMode = mode % 4; }
    uint8_t stepOctaveMode(int delta);
    const char* getOctaveModeString() const;

    // Transpose
    int8_t getTranspose() const { return config.transpose; }
    void setTranspose(int8_t semi) { config.transpose = constrain(semi, -12, 12); }
    int8_t stepTranspose(int delta);

    // Expression mode
    uint8_t getExpMode() const { return config.expMode; }
    void setExpMode(uint8_t mode) { config.expMode = mode % 4; }
    uint8_t stepExpMode(int delta);
    const char* getExpModeString() const;

    // Footswitches
    uint8_t getFs0Mode() const { return config.fs0Mode; }
    void toggleFs0Mode() { config.fs0Mode = !config.fs0Mode; }

    uint8_t getFs1Mode() const { return config.fs1Mode; }
    void toggleFs1Mode() { config.fs1Mode = !config.fs1Mode; }

    // Expression pedal / pot
    uint8_t getExpKillSwitch() const { return config.expKillSwitch; }
    void toggleExpKillSwitch() { config.expKillSwitch = !config.expKillSwitch; }

    uint8_t getExpCcNumber() const { return config.expCcNumber; }
    void setExpCcNumber(uint8_t cc) { config.expCcNumber = constrain(cc, 0, 127); }
    uint8_t stepExpCcNumber(int delta);

    uint16_t getExpInLo() const { return config.expInLo; }
    uint16_t getExpInHi() const { return config.expInHi; }
    void setExpInputRange(uint16_t inLo, uint16_t inHi) {
        config.expInLo = inLo;
        config.expInHi = inHi;
    }

private:
    MomiConfig config;
};

extern ConfigManager configMgr;
