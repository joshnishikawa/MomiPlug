#pragma once
#include <stdint.h>

namespace Pins {
    // Encoder and Navigation
    constexpr uint8_t ENCODER_A          = 25;
    constexpr uint8_t ENCODER_B          = 24;
    constexpr uint8_t ENCODER_BUTTON     = 2;  // Push switch on encoder

    // Status LEDs
    constexpr uint8_t LED_TOP_LEFT       = 8;
    constexpr uint8_t LED_CENTER         = 7;
    constexpr uint8_t LED_TOP_RIGHT      = 6;
    constexpr uint8_t LED_FOOTSWITCH_0   = 55; // Remapped pin 55
    constexpr uint8_t LED_FOOTSWITCH_1   = 9;
    constexpr uint8_t LED_ONBOARD        = 13; // Teensy onboard orange LED

    // Touch Buttons & Chaos Pad (Capacitive Touch)
    constexpr uint8_t TOUCH_TOP_LEFT     = 19;
    constexpr uint8_t TOUCH_CENTER       = 22;
    constexpr uint8_t TOUCH_TOP_RIGHT    = 23;
    constexpr uint8_t TOUCH_BOTTOM_RIGHT = 17;
    constexpr uint8_t TOUCH_CHAOS_PAD    = 18; // Dedicated touch Chaos pad (Bottom-Left)

    // Footswitch & Expression Inputs
    constexpr uint8_t FOOTSWITCH_0       = 26; // Footswitch 0 (Tip)
    constexpr uint8_t FOOTSWITCH_1       = 15; // Footswitch 1 (Ring)
    constexpr uint8_t EXPRESSION_PEDAL   = 39; // Analog input for expression pedal

    // Analog Multiplexers (CD4051)
    constexpr uint8_t MUX_SEL_A          = 10; // Selector line A
    constexpr uint8_t MUX_SEL_B          = 11; // Selector line B
    constexpr uint8_t MUX_SEL_C          = 12; // Selector line C
    constexpr uint8_t MUX_SEL_D          = 14; // Selector line D (also SCK0)
    constexpr uint8_t MUX_ANALOG_IN_0    = 20; // MUX 0 signal line
    constexpr uint8_t MUX_ANALOG_IN_1    = 21; // MUX 1 signal line

    // 4-Digit 7-Segment Display
    constexpr uint8_t DISPLAY_DIGITS[4]   = {33, 30, 36, 38};
    constexpr uint8_t DISPLAY_SEGMENTS[8] = {32, 31, 35, 28, 34, 29, 27, 37};

    // I2C Wire2 (OLED Chord Display)
    constexpr uint8_t I2C2_SCL           = 3;
    constexpr uint8_t I2C2_SDA           = 4;

    // Audio & Mic (Auxiliary / Reserved)
    constexpr uint8_t MIC                = 16;
}
