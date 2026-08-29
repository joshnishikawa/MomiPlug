#include "MomiPlug.h"

// MIDI and USB Host Setup
MIDI_CREATE_DEFAULT_INSTANCE();

USBHost teensyUSBHost;
USBHub hub1(teensyUSBHost);
USBHub hub2(teensyUSBHost);
USBHub hub3(teensyUSBHost);
USBHub hub4(teensyUSBHost);
MIDIDevice_BigBuffer midi1(teensyUSBHost);
MIDIDevice_BigBuffer midi2(teensyUSBHost);
MIDIDevice_BigBuffer midi3(teensyUSBHost);
MIDIDevice_BigBuffer midi4(teensyUSBHost);

// PIN ASSIGNMENTS ##################################################
// MIDI in  is pin 0 by default (RX1)
// MIDI out is pin 1 by default (TX1)
const int editPin = 2;        // edit button (push switch on encoder)
const int scl2 = 3;           // Serial Clock 2
const int sda2 = 4;           // Serial Data 2
const int fs0LED = 55;        // LED for Footswitch 1 (remapped pin 55)
const int topRightLED = 6;    // LED on upper right
const int centerLED = 7;      // LED in center
const int topLeftLED = 8;     // LED on upper left
const int fs1LED = 9;         // LED for Footswitch 0
const int sel_a = 10;         // selectors for analog and digital mux inputs
const int sel_b = 11;
const int sel_c = 12;
const int LED0 = 13;          // the onboard (orange) LED
const int sel_d = 14;         // also SCK0 - use SPI.setSCK(14)
const int fs1Pin = 15;        // footswitch 1 (ring)
const int mic = 16;           // microphone UNTESTED
const int bottomRightButton = 17; // regular latching button (Bs[3])
const int bottomLeftButton = 18;  // dedicated Chaos pad (Pin 18)
const int topLeftButton = 19;     // onboard buttons
const int muxPin0 = 20;           // muxed analog input
const int muxPin1 = 21;           // muxed analog input
const int centerButton = 22;
const int topRightButton = 23;
const int encPinB = 24;           // pin A of encoder
const int encPinA = 25;           // pin B of encoder
const int fs0Pin = 26;            // footswitch 0 (tip)

const uint8_t digitPins[] = {33, 30, 36, 38};
const uint8_t segmentPins[] = {32, 31, 35, 28, 34, 29, 27, 37};

const int expPin = 39;        // analog input for expression pedal
const int phonesL = A21;      // headphones L (tip) UNTESTED
const int phonesR = A22;      // headphones R (tip) UNTESTED

// TOUCH BUTTON THRESHOLDS ##########################################
// Custom thresholds for each latching capacitive touch switch:
// Bs[0]: Top Left     (Pin 19)
// Bs[1]: Center       (Pin 22)
// Bs[2]: Top Right    (Pin 23)
// Bs[3]: Bottom Right (Pin 17)
// Pin 18: Bottom Left (Dedicated Chaos Pad)
const int touchThresholds[5] = {
  1150, // Bs[0] Top Left (Pin 19)
  1150, // Bs[1] Center (Pin 22)
  1150, // Bs[2] Top Right (Pin 23)
  1400, // Bs[3] Bottom Right (Pin 17)
  0     // Bs[4] Unused (Pin 18 is dedicated Chaos pad)
};

// Indices of onboard touch buttons used in ctrlMode (excluding Chaos pad)
const uint8_t ctrlButtonIndices[4] = {0, 1, 2, 3};

// GLOBAL STATE & OBJECTS ###########################################
MomiConfig config;
byte MIDIchannel = 3;

bool editMode = false;
bool trackMode = false;

SevSeg DSP;
char DSPstring[5] = "    ";
Editor editor = Editor(encPinA, encPinB, editPin);
IntervalTimer displayTimer;

Track* Ts[3];
MIDIswitch* Bs[23];
MIDIpot* Ps[18];

uint16_t inLo = 1000; // for cap touch Chaos FX
uint16_t inHi = 1150; // for cap touch Chaos FX

// DISPLAY TIMER ISR ################################################
void displayTimerISR() {
  DSP.refreshDisplay();
}

// FORWARD DECLARATIONS #############################################
void selectMode();
void tracMode();
void ctrlMode();
void tracOrCtrlMode();

// CONFIG STORAGE ###################################################
void loadConfig() {
  EEPROM.get(0, config);
  if (config.magic != MOMI_CONFIG_MAGIC || config.midiChannel < 1 || config.midiChannel > 16) {
    config.magic = MOMI_CONFIG_MAGIC;
    config.midiChannel = 3;
    config.readMIDIthru = true;
    config.readMUX0 = false;
    config.readMUX1 = false;
    config.fs0Mode = 1; // LATCH
    config.fs1Mode = 0; // MOMENTARY
    config.expKillSwitch = 0;
    saveConfig();
  }
  MIDIchannel = config.midiChannel;
}

void saveConfig() {
  config.magic = MOMI_CONFIG_MAGIC;
  config.midiChannel = MIDIchannel;
  EEPROM.put(0, config);
}

// SETUP ############################################################
void setup() {
  loadConfig();

  DSP.begin(COMMON_CATHODE, 4, (uint8_t*)digitPins, (uint8_t*)segmentPins, true);
  DSP.setBrightness(100);

  // Background display timer interrupt at 1 kHz
  displayTimer.begin(displayTimerISR, 1000);

  // Software track states (CC 107, 108, 109)
  Ts[0] = new Track(107);
  Ts[1] = new Track(108);
  Ts[2] = new Track(109);

  // Physical hardware touch buttons (Latching)
  Bs[0] = new MIDIswitch(topLeftButton, 102, LATCH, TOUCH);
  Bs[1] = new MIDIswitch(centerButton, 103, LATCH, TOUCH);
  Bs[2] = new MIDIswitch(topRightButton, 104, LATCH, TOUCH);
  Bs[3] = new MIDIswitch(bottomRightButton, 105, LATCH, TOUCH);
  Bs[4] = nullptr; // Pin 18 (bottomLeftButton) is dedicated to Chaos pad

  // Set explicit specific thresholds for active capacitive touch switches
  Bs[0]->setThreshold(touchThresholds[0]);
  Bs[1]->setThreshold(touchThresholds[1]);
  Bs[2]->setThreshold(touchThresholds[2]);
  Bs[3]->setThreshold(touchThresholds[3]);

  Bs[5] = new MIDIswitch(fs1Pin, 80, config.fs1Mode);
  Bs[6] = new MIDIswitch(fs0Pin, 81, config.fs0Mode);
  for (int i = 7; i < 23; i++) {
    Bs[i] = new MIDIswitch(muxPin0, 9 + i, 1); // CC 16~31
  }

  Ps[0] = new MIDIpot(expPin, 85);
  Ps[0]->killSwitch = config.expKillSwitch;
  Ps[0]->inputRange(10, 900);
  for (int i = 1; i < 17; i++) {
    Ps[i] = new MIDIpot(muxPin0, 47 + i); // CC 48~63
  }

  // Calibrate baseline for dedicated Chaos pad on Pin 18 (bottomLeftButton)
  delay(50);
  // inLo = touchRead(bottomLeftButton) * 1.02;
  // inHi = inLo * 1.7;

  pinMode(sel_a, OUTPUT);
  pinMode(sel_b, OUTPUT);
  pinMode(sel_c, OUTPUT);
  pinMode(sel_d, OUTPUT);
  pinMode(fs0LED, OUTPUT);
  pinMode(fs1LED, OUTPUT);
  pinMode(LED0, OUTPUT);
  pinMode(topLeftLED, OUTPUT);
  pinMode(centerLED, OUTPUT);
  pinMode(topRightLED, OUTPUT);

  // DIN MIDI callbacks
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOff(onNoteOff);
  MIDI.setHandleNoteOn(onNoteOn);
  MIDI.setHandleAfterTouchPoly(onPolyPressure);
  MIDI.setHandleControlChange(onControl);
  MIDI.setHandleProgramChange(onProgram);
  MIDI.setHandleAfterTouchChannel(onAfterTouch);
  MIDI.setHandlePitchBend(onPitchBend);

  // Start USB Host controller
  teensyUSBHost.begin();

  // Attach handlers to all 4 USB Host MIDI BigBuffer instances
  auto attachUSBHandlers = [](auto &dev) {
    dev.setHandleNoteOff(onUSBNoteOff);
    dev.setHandleNoteOn(onUSBNoteOn);
    dev.setHandleControlChange(onUSBControl);
    dev.setHandleProgramChange(onUSBProgram);
    dev.setHandleAfterTouchChannel(onUSBAfterTouch);
    dev.setHandlePitchChange(onUSBPitchBend);
    dev.setHandleAfterTouchPoly(onUSBPolyPressure);
  };

  attachUSBHandlers(midi1);
  attachUSBHandlers(midi2);
  attachUSBHandlers(midi3);
  attachUSBHandlers(midi4);
}

// MAIN LOOP ########################################################
void loop() {
  // Service USB Host hardware continuously
  teensyUSBHost.Task();

  // Read and process all incoming USB Host and DIN MIDI packets
  while (midi1.read()) {}
  while (midi2.read()) {}
  while (midi3.read()) {}
  while (midi4.read()) {}

  if (config.readMIDIthru) {
    while (MIDI.read()) {}
  } else {
    while (Serial1.available()) {
      Serial1.read();
    }
  }

  editor.bounce->update();

  if (editor.bounce->fell()) {
    sprintf(DSPstring, "%4d", MIDIchannel);
  }
  else if (editor.bounce->rose()) {
    strcpy(DSPstring, "    ");
    editor.encoder->write(0);

    if (editMode) {
      saveConfig();
      editMode = false;
    }
    else {
      trackMode = !trackMode;
    }

    if (trackMode) {
      for (int i = 0; i < 3; i++) {
        usbMIDI.sendControlChange(Ts[i]->number, Ts[i]->level, MIDIchannel);
      }
      strcpy(DSPstring, "trac");
    }
    else {
      if (config.readMUX0) {
        for (int i = 1; i < 9; i++) {
          usbMIDI.sendControlChange(Ps[i]->number, Ps[i]->value, MIDIchannel);
        }
      }
      strcpy(DSPstring, "ctrl");
    }
  }
  else if (editor.bounce->read() == LOW) {
    selectMode();
  }
  else {
    tracOrCtrlMode();
    if (trackMode) {
      tracMode();
    } else {
      ctrlMode();
    }
  }

  DSP.setChars(DSPstring);
}

// CONFIGURATION & SELECTION MODE ###################################
void selectMode() {
  // 1. Channel edit via encoder rotation
  byte newChannel = editor.editChannel(MIDIchannel);
  if (newChannel != MIDIchannel) {
    MIDIchannel = newChannel;
    config.midiChannel = newChannel;
    editMode = true;
    sprintf(DSPstring, "C%3d", MIDIchannel);
  }

  // 2. Touch button toggles for MUX and Thru (edge detected)
  static bool btn0Last = false;
  static bool btn1Last = false;
  static bool btn2Last = false;
  static bool fs0Last = false;
  static bool fs1Last = false;
  static bool expLast = false;

  bool btn0Now = (Bs[0]->read() == 127);
  if (btn0Now && !btn0Last) {
    config.readMIDIthru = !config.readMIDIthru;
    editMode = true;
    strcpy(DSPstring, config.readMIDIthru ? "tru1" : "tru0");
  }
  btn0Last = btn0Now;

  bool btn1Now = (Bs[1]->read() == 127);
  if (btn1Now && !btn1Last) {
    config.readMUX0 = !config.readMUX0;
    editMode = true;
    strcpy(DSPstring, config.readMUX0 ? "mx01" : "mx00");
  }
  btn1Last = btn1Now;

  bool btn2Now = (Bs[2]->read() == 127);
  if (btn2Now && !btn2Last) {
    config.readMUX1 = !config.readMUX1;
    editMode = true;
    strcpy(DSPstring, config.readMUX1 ? "mx11" : "mx10");
  }
  btn2Last = btn2Now;

  // 3. Footswitch mode toggles (Momentary vs Latch)
  bool fs1Now = (digitalRead(fs1Pin) == LOW);
  if (fs1Now && !fs1Last) {
    config.fs1Mode = !config.fs1Mode;
    Bs[5]->mode = config.fs1Mode;
    editMode = true;
    strcpy(DSPstring, config.fs1Mode ? "1-lc" : "1-mo");
  }
  fs1Last = fs1Now;

  bool fs0Now = (digitalRead(fs0Pin) == LOW);
  if (fs0Now && !fs0Last) {
    config.fs0Mode = !config.fs0Mode;
    Bs[6]->mode = config.fs0Mode;
    editMode = true;
    strcpy(DSPstring, config.fs0Mode ? "0-lc" : "0-mo");
  }
  fs0Last = fs0Now;

  // 4. Expression pedal killswitch toggle (edge detected via raw analog read)
  int rawExp = analogRead(expPin);
  bool expNow = (rawExp >= 850);
  if (expNow && !expLast) {
    config.expKillSwitch = !config.expKillSwitch;
    Ps[0]->killSwitch = config.expKillSwitch;
    editMode = true;
    strcpy(DSPstring, config.expKillSwitch ? " cut" : "-cut");
  }
  expLast = expNow;

  // Visual status indicators while in edit mode
  digitalWrite(topLeftLED, config.readMIDIthru);
  digitalWrite(centerLED, config.readMUX0);
  digitalWrite(topRightLED, config.readMUX1);
}

// TRACK MODE #######################################################
void tracMode() {
  int encVal = editor.encoder->read();
  int incdec = 0;
  if (encVal >= 4) {
    incdec = 1;
    editor.encoder->write(0);
  } else if (encVal <= -4) {
    incdec = -1;
    editor.encoder->write(0);
  }

  // Use the single hardware button instances Bs[0..2] to arm/disarm tracks
  static uint8_t trackIdx = 0;
  static bool btnArmedLast[3] = {false, false, false};
  bool isPressed = (Bs[trackIdx]->read() == 127);
  if (isPressed && !btnArmedLast[trackIdx]) {
    int newLevel = Ts[trackIdx]->toggleArm();
    sprintf(DSPstring, "%4d", newLevel);
  }
  btnArmedLast[trackIdx] = isPressed;

  if (incdec != 0) {
    for (int i = 0; i < 3; i++) {
      int newLevel = Ts[i]->vol(incdec);
      if (newLevel >= 0) {
        sprintf(DSPstring, "%4d", newLevel);
      }
    }
  }
  trackIdx = (trackIdx + 1) % 3;

  byte recState = record(Bs[5]->read() == 127, Bs[6]->read() == 127);
  digitalWrite(fs0LED, recState);
  digitalWrite(fs1LED, !digitalRead(fs0Pin));
  digitalWrite(topLeftLED, Ts[0]->state);
  digitalWrite(centerLED, Ts[1]->state);
  digitalWrite(topRightLED, Ts[2]->state);
}

// CONTROL MODE #####################################################
void ctrlMode() {
  int newVal = editor.send();
  if (newVal >= 0) {
    sprintf(DSPstring, "%4d", editor.level);
    DSPstring[0] = 'r';
  }

  // Round-robin polling across the active onboard touch buttons (0, 1, 2, 3)
  static uint8_t touchIdx = 0;
  uint8_t btnIdx = ctrlButtonIndices[touchIdx];
  newVal = Bs[btnIdx]->send();
  if (newVal >= 0) {
    sprintf(DSPstring, "%4d", newVal);
    DSPstring[0] = 'b';
  }
  touchIdx = (touchIdx + 1) % 4;

  // Poll foot switches (digital pins)
  for (int i = 5; i < 7; i++) {
    newVal = Bs[i]->send();
    if (newVal >= 0) {
      sprintf(DSPstring, "%4d", newVal);
      DSPstring[0] = 'f';
    }
  }

  digitalWrite(topLeftLED, Bs[0]->state);
  digitalWrite(centerLED, Bs[1]->state);
  digitalWrite(topRightLED, Bs[2]->state);
  digitalWrite(fs0LED, Bs[5]->state);
  digitalWrite(fs1LED, Bs[6]->state);
}

// SHARED SENSORS & ROUTING #########################################
void tracOrCtrlMode() {
  int newVal = Ps[0]->send(); // Send MIDI for EXP Pedal
  if (newVal >= 0) {
    sprintf(DSPstring, "%4d", newVal);
    DSPstring[0] = 'E';
  }

  if (config.readMIDIthru) {
    // Sample dedicated chaos pad on Pin 18 (bottomLeftButton) on a 20ms cadence
    static elapsedMillis chaosTimer = 0;
    if (chaosTimer >= 20) {
      chaosTimer = 0;
      int newValue = touchRead(bottomLeftButton);
      chaos(LED0, newValue, inLo, inHi, 48, 84);
    }
  }

  if (config.readMUX0) {
    newVal = Ps[1]->send();
    if (newVal >= 0) {
      sprintf(DSPstring, "%4d", newVal);
      DSPstring[0] = 'h';
    }
  }

  if (config.readMUX1) {
    for (int i = 0; i < 8; i++) { // 8-channel MUX
      digitalWrite(sel_d, (i & 7) >> 2);
      digitalWrite(sel_c, (i & 3) >> 1);
      digitalWrite(sel_b, (i & 1));

      newVal = Ps[i + 1]->send();
      if (newVal >= 0) {
        sprintf(DSPstring, "%4d", newVal);
        DSPstring[0] = 'A';
      }
    }
  }
}