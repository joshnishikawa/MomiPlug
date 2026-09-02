#include "MuxManager.h"

MuxManager muxMgr;

void MuxManager::begin() {
    pinMode(Pins::MUX_SEL_A, OUTPUT);
    pinMode(Pins::MUX_SEL_B, OUTPUT);
    pinMode(Pins::MUX_SEL_C, OUTPUT);
    pinMode(Pins::MUX_SEL_D, OUTPUT);
}

void MuxManager::selectChannel(uint8_t channel) {
    // 3-bit multiplexer addressing (0..7) on select lines B, C, D
    digitalWrite(Pins::MUX_SEL_D, (channel & 7) >> 2);
    digitalWrite(Pins::MUX_SEL_C, (channel & 3) >> 1);
    digitalWrite(Pins::MUX_SEL_B, (channel & 1));
}
