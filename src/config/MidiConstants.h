#pragma once
#include <stdint.h>

namespace MidiCC {
    // Encoder Parameter Control
    constexpr uint8_t ENCODER_PARAM       = 3;

    // Track Mode Controls
    constexpr uint8_t TRACK_ARM_BASE      = 107; // CC 107 (Track 0), 108 (Track 1), 109 (Track 2)
    constexpr uint8_t TRACK_VOL_OFFSET    = 3;   // Track Vol = Arm CC + 3 -> CC 110, 111, 112
    constexpr uint8_t SCENE_RECORD_START  = 111; // Scenes cycle CC 111..119
    constexpr uint8_t SCENE_RECORD_END    = 119;
    constexpr uint8_t SCENE_STOP          = 111;

    // Hardware Touch Controls (Latching)
    constexpr uint8_t TOUCH_TOP_LEFT      = 102;
    constexpr uint8_t TOUCH_CENTER        = 103;
    constexpr uint8_t TOUCH_TOP_RIGHT     = 104;
    constexpr uint8_t TOUCH_BOTTOM_RIGHT  = 105;

    // Footswitches & Pedals
    constexpr uint8_t FOOTSWITCH_1        = 80;  // FS1 (Ring)
    constexpr uint8_t FOOTSWITCH_0        = 81;  // FS0 (Tip)
    constexpr uint8_t EXPRESSION_PEDAL    = 85;

    // Multiplexer Pots (8 per bank)
    constexpr uint8_t MUX0_POT_BASE       = 48;  // CC 48..55 (Bank A)
    constexpr uint8_t MUX1_POT_BASE       = 56;  // CC 56..63 (Bank B)

    // Standard MIDI Control Changes
    constexpr uint8_t SUSTAIN_PEDAL       = 64;
}

namespace UsbCable {
    // 0-indexed virtual cable numbers for usbMIDI.send*(..., cable)
    constexpr uint8_t DEFAULT_PORT        = 0; // Port 1
    constexpr uint8_t SYNTH_PORT          = 1; // Port 2
    constexpr uint8_t CHORD_PORT          = 2; // Port 3
    constexpr uint8_t ROUTE_PORT          = 3; // Port 4
}

namespace TouchConfig {
    // Capacitive touch activation thresholds
    constexpr int THRESHOLD_TOP_LEFT      = 1150;
    constexpr int THRESHOLD_CENTER        = 1150;
    constexpr int THRESHOLD_TOP_RIGHT     = 1150;
    constexpr int THRESHOLD_BOTTOM_RIGHT  = 1400;

    // Chaos Pad (Pin 18)
    constexpr uint8_t CHAOS_NOTE_MIN      = 48; // C3
    constexpr uint8_t CHAOS_NOTE_MAX      = 84; // C6
    constexpr uint8_t CHAOS_VELOCITY      = 96;
    constexpr uint8_t CHAOS_POLL_MS       = 20; // 20ms cadence
}

namespace ExpressionConfig {
    constexpr int INPUT_MIN               = 10;
    constexpr int INPUT_MAX               = 900;
    constexpr int KILLSWITCH_THRESHOLD    = 850;
}
