#include "ConfigManager.h"

ConfigManager configMgr;

void ConfigManager::resetDefaults() {
    config.magic = MOMI_CONFIG_MAGIC;
    config.midiChannel = 3;
    config.readMIDIthru = true;
    config.mux0Mode = 0;
    config.mux1Mode = 0;
    config.fs0Mode = 1; // LATCH
    config.fs1Mode = 0; // MOMENTARY
    config.expKillSwitch = 0;
}

void ConfigManager::load() {
    EEPROM.get(0, config);
    if (config.magic != MOMI_CONFIG_MAGIC || config.midiChannel < 1 || config.midiChannel > 16) {
        resetDefaults();
        save();
    }
    if (config.mux0Mode != 0 && config.mux0Mode != 1 && config.mux0Mode != 8) config.mux0Mode = 0;
    if (config.mux1Mode != 0 && config.mux1Mode != 1 && config.mux1Mode != 8) config.mux1Mode = 0;
}

void ConfigManager::save() {
    config.magic = MOMI_CONFIG_MAGIC;
    EEPROM.put(0, config);
}

uint8_t ConfigManager::cycleMux0Mode() {
    if (config.mux0Mode == 0) config.mux0Mode = 1;
    else if (config.mux0Mode == 1) config.mux0Mode = 8;
    else config.mux0Mode = 0;
    return config.mux0Mode;
}

uint8_t ConfigManager::cycleMux1Mode() {
    if (config.mux1Mode == 0) config.mux1Mode = 1;
    else if (config.mux1Mode == 1) config.mux1Mode = 8;
    else config.mux1Mode = 0;
    return config.mux1Mode;
}
