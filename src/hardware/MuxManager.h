#pragma once
#include <Arduino.h>
#include "../config/PinMap.h"

class MuxManager {
public:
    void begin();
    void selectChannel(uint8_t channel);
};

extern MuxManager muxMgr;
