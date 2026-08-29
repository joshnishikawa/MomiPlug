#ifndef MomiPlug_h
#define MomiPlug_h

// Libraries required for MomiPlug
#include "Arduino.h"
#include <USBHost_t36.h>
#include <MIDI.h>
#include <EEPROM.h>
#include "SevSeg.h"
#include "MIDIcontroller.h"
#include "MIDIinput.h"
#include "track.h"
#include "editor.h"

// Configuration storage struct
struct MomiConfig {
  uint32_t magic;          // Magic number for EEPROM validity check
  uint8_t  midiChannel;    // Active MIDI Channel (1 - 16)
  bool     readMIDIthru;   // Pass-through incoming MIDI
  bool     readMUX0;       // Enable Multiplexer 0
  bool     readMUX1;       // Enable Multiplexer 1
  uint8_t  fs0Mode;        // Footswitch 0 mode (MOMENTARY / LATCH)
  uint8_t  fs1Mode;        // Footswitch 1 mode (MOMENTARY / LATCH)
  uint8_t  expKillSwitch;  // Expression pedal killswitch mode
};

#define MOMI_CONFIG_MAGIC 0x4D4F4D31 // 'MOM1'

void loadConfig();
void saveConfig();

#endif