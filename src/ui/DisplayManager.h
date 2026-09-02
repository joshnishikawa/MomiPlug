#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "SevSeg.h"
#include "ChordDisplay.h"
#include "ChordAnalyzer.h"
#include "../config/PinMap.h"

class DisplayManager {
public:
    DisplayManager();

    void begin();
    void refreshHardware(); // Called inside 1kHz timer ISR

    void showText(const char* str);
    void showControlValue(char prefix, int value);
    void showNumber(int value);
    void showChannel(uint8_t channel);
    void showMuxModes(uint8_t mux0Mode, uint8_t mux1Mode);
    void clear();

    void updateChordDisplay(const ChordAnalysisResult& chord);
    void updateChordDisplayIfChanged(ChordAnalyzer& analyzer);

    void updateBuffer(); // Pushes string buffer to SevSeg if changed

private:
    SevSeg sevSeg;
    ChordDisplay chordDisplay;
    char currentString[5];
    char lastString[5];
};

extern DisplayManager display;
