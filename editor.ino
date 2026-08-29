#include "editor.h"

// Editor constructors
Editor::Editor() : bounce(nullptr), encoder(nullptr), number(3), level(0), editAnalogInputRange(0), DP(0), newInLo(0), newInHi(0) {}

Editor::Editor(int a, int b, int p) : bounce(nullptr), encoder(nullptr), number(3), level(0), editAnalogInputRange(0), DP(0), newInLo(0), newInHi(0) {
  pinMode(p, INPUT_PULLUP);
  bounce = new Bounce(p, 50);
  encoder = new Encoder(a, b);
}

// Editor destructor
Editor::~Editor() {
  if (bounce) delete bounce;
  if (encoder) delete encoder;
}

int Editor::send() {
  int incdec = encoder->read();
  if (incdec >= 4 && level < 127) {
    level += 1;
    encoder->write(0);
    usbMIDI.sendControlChange(number, level, MIDIchannel);
    return level;
  }
  else if (incdec <= -4 && level > 0) {
    level -= 1;
    encoder->write(0);
    usbMIDI.sendControlChange(number, level, MIDIchannel);
    return level;
  }
  else if (incdec >= 4 || incdec <= -4) {
    encoder->write(0);
    return -1;
  }
  return -1;
}

int Editor::quadOne(byte val, byte minVal, byte maxVal) {
  int delta = encoder->read();
  if (delta >= 4) {
    encoder->write(0);
    if (val < maxVal) {
      return val + 1;
    } else {
      return minVal;
    }
  }
  else if (delta <= -4) {
    encoder->write(0);
    if (val > minVal) {
      return val - 1;
    } else {
      return maxVal;
    }
  }
  return -1;
}

byte Editor::editChannel(byte currentChannel) {
  int newValue = quadOne(currentChannel, 1, 16);
  if (newValue >= 1 && newValue <= 16) {
    return (byte)newValue;
  }
  return currentChannel;
}

byte Editor::setAnalog(int p) {
  int newValue = analogRead(p);
  if (newValue > newInHi) { newInHi = newValue; }
  else if (newValue < newInLo) { newInLo = newValue; }
  return (newInHi - newInLo > 127) ? 1 : 0;
}
