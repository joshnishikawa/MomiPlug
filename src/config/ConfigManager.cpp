#include "ConfigManager.h"

ConfigManager configMgr;

void ConfigManager::resetDefaults() {
    config.magic = MOMI_CONFIG_MAGIC;
    config.midiChannel = 3;
    config.readMIDIthru = true;
    config.muxMode = MUX_NONE; // '----' Halts all MUX pin reads by default
    config.octaveMode = OCTAVE_OFF; // 'oc--'
    config.transpose = 0;
    config.expMode = EXP_CAOS; // 'CAOS'
    config.fs0Mode = 1; // LATCH
    config.fs1Mode = 0; // MOMENTARY
    config.expKillSwitch = 0;
    config.expCcNumber = 85;
    config.expInLo = 10;
    config.expInHi = 900;
}

void ConfigManager::load() {
    EEPROM.get(0, config);
    if (config.magic != MOMI_CONFIG_MAGIC || config.midiChannel < 1 || config.midiChannel > 16) {
        resetDefaults();
        save();
    }
    if (config.muxMode > 7) config.muxMode = MUX_NONE;
    if (config.octaveMode > 3) config.octaveMode = OCTAVE_OFF;
    if (config.transpose < -12 || config.transpose > 12) config.transpose = 0;
    if (config.expMode > 3) config.expMode = EXP_CAOS;
    if (config.expCcNumber > 127) config.expCcNumber = 85;
    config.readMIDIthru = true;
}

void ConfigManager::save() {
    config.magic = MOMI_CONFIG_MAGIC;
    EEPROM.put(0, config);
}

uint8_t ConfigManager::stepMuxMode(int delta) {
    int next = static_cast<int>(config.muxMode) + delta;
    if (next < 0) next = 7;
    else if (next > 7) next = 0;
    config.muxMode = static_cast<uint8_t>(next);
    return config.muxMode;
}

const char* ConfigManager::getMuxModeString() const {
    switch (config.muxMode) {
        case MUX_NONE:   return "----";
        case MUX_A_NONE: return "A---";
        case MUX_A_A:    return "A--A";
        case MUX_8_NONE: return "8---";
        case MUX_8_A:    return "8--A";
        case MUX_8_8:    return "8--8";
        case MUX_SPI:    return "SPI-";
        case MUX_SPI_A:  return "SPIA";
        default:         return "----";
    }
}

uint8_t ConfigManager::stepOctaveMode(int delta) {
    int next = static_cast<int>(config.octaveMode) + delta;
    if (next < 0) next = 3;
    else if (next > 3) next = 0;
    config.octaveMode = static_cast<uint8_t>(next);
    return config.octaveMode;
}

const char* ConfigManager::getOctaveModeString() const {
    switch (config.octaveMode) {
        case OCTAVE_L_NONE: return "ocL-";
        case OCTAVE_NONE_H: return "oc-H";
        case OCTAVE_L_H:    return "ocLH";
        case OCTAVE_OFF:    return "oc--";
        default:            return "oc--";
    }
}

int8_t ConfigManager::stepTranspose(int delta) {
    config.transpose = constrain(config.transpose + delta, -12, 12);
    return config.transpose;
}

uint8_t ConfigManager::stepExpMode(int delta) {
    int next = static_cast<int>(config.expMode) + delta;
    if (next < 0) next = 3;
    else if (next > 3) next = 0;
    config.expMode = static_cast<uint8_t>(next);
    return config.expMode;
}

const char* ConfigManager::getExpModeString() const {
    switch (config.expMode) {
        case EXP_CAOS: return "CAOS";
        case EXP_ECHO: return "ECHO";
        case EXP_BOTH: return "both";
        case EXP_NONE: return "----";
        default:       return "----";
    }
}

uint8_t ConfigManager::stepExpCcNumber(int delta) {
    int next = static_cast<int>(config.expCcNumber) + delta;
    config.expCcNumber = static_cast<uint8_t>(constrain(next, 0, 127));
    return config.expCcNumber;
}
