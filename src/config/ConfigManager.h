#pragma once
#include <Arduino.h>
#include <EEPROM.h>

constexpr uint32_t MOMI_CONFIG_MAGIC = 0x4D4F4D33; // 'MOM3'

struct MomiConfig {
    uint32_t magic;          // Magic number for EEPROM validity check
    uint8_t  midiChannel;    // Active MIDI Channel (1 - 16)
    bool     readMIDIthru;   // Pass-through incoming MIDI
    uint8_t  mux0Mode;       // MUX0 mode: 0 (disabled), 1 (single pot), 8 (8-ch mux)
    uint8_t  mux1Mode;       // MUX1 mode: 0 (disabled), 1 (single pot), 8 (8-ch mux)
    uint8_t  fs0Mode;        // Footswitch 0 mode (0: MOMENTARY, 1: LATCH)
    uint8_t  fs1Mode;        // Footswitch 1 mode (0: MOMENTARY, 1: LATCH)
    uint8_t  expKillSwitch;  // Expression pedal killswitch mode (0: off, 1: on)
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

    uint8_t getMux0Mode() const { return config.mux0Mode; }
    uint8_t cycleMux0Mode();

    uint8_t getMux1Mode() const { return config.mux1Mode; }
    uint8_t cycleMux1Mode();

    uint8_t getFs0Mode() const { return config.fs0Mode; }
    void toggleFs0Mode() { config.fs0Mode = !config.fs0Mode; }

    uint8_t getFs1Mode() const { return config.fs1Mode; }
    void toggleFs1Mode() { config.fs1Mode = !config.fs1Mode; }

    uint8_t getExpKillSwitch() const { return config.expKillSwitch; }
    void toggleExpKillSwitch() { config.expKillSwitch = !config.expKillSwitch; }

private:
    MomiConfig config;
};

extern ConfigManager configMgr;
