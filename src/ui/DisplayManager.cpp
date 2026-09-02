#include "DisplayManager.h"

DisplayManager display;

DisplayManager::DisplayManager()
    : chordDisplay(Wire2, 0x3C, 4, 3) {
    strcpy(currentString, "    ");
    strcpy(lastString, "");
}

void DisplayManager::begin() {
    // Initialize I2C Wire2 at 1 MHz (Fast Mode+) for OLED chord display
    Wire2.begin();
    Wire2.setClock(1000000);
    chordDisplay.begin();

    // 4-Digit Common Cathode 7-Segment display
    sevSeg.begin(
        COMMON_CATHODE,
        4,
        const_cast<uint8_t*>(Pins::DISPLAY_DIGITS),
        const_cast<uint8_t*>(Pins::DISPLAY_SEGMENTS),
        true
    );
    sevSeg.setBrightness(100);
}

void DisplayManager::refreshHardware() {
    sevSeg.refreshDisplay();
}

void DisplayManager::showText(const char* str) {
    snprintf(currentString, sizeof(currentString), "%-4s", str);
    updateBuffer();
}

void DisplayManager::showControlValue(char prefix, int value) {
    snprintf(currentString, sizeof(currentString), "%4d", constrain(value, 0, 999));
    currentString[0] = prefix;
    updateBuffer();
}

void DisplayManager::showNumber(int value) {
    snprintf(currentString, sizeof(currentString), "%4d", constrain(value, 0, 9999));
    updateBuffer();
}

void DisplayManager::showChannel(uint8_t channel) {
    snprintf(currentString, sizeof(currentString), "C%3d", channel);
    updateBuffer();
}

void DisplayManager::showMuxModes(uint8_t mux0Mode, uint8_t mux1Mode) {
    char c0 = (mux0Mode == 8 ? '8' : (mux0Mode == 1 ? '1' : '0'));
    char c1 = (mux1Mode == 8 ? '8' : (mux1Mode == 1 ? '1' : '0'));
    snprintf(currentString, sizeof(currentString), "%c  %c", c0, c1);
    updateBuffer();
}

void DisplayManager::clear() {
    strcpy(currentString, "    ");
    updateBuffer();
}

void DisplayManager::updateBuffer() {
    if (strcmp(lastString, currentString) != 0) {
        sevSeg.setChars(currentString);
        strcpy(lastString, currentString);
    }
}

void DisplayManager::updateChordDisplay(const ChordAnalysisResult& chord) {
    chordDisplay.update(chord);
}

void DisplayManager::updateChordDisplayIfChanged(ChordAnalyzer& analyzer) {
    if (analyzer.hasChanged()) {
        const ChordAnalysisResult& result = analyzer.analyze();
        chordDisplay.update(result);
    }
}
