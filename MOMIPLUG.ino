#include "MomiPlug.h"

MIDI_CREATE_DEFAULT_INSTANCE();
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI2);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI3);
USBHost teensyUSBHost;
USBHub hub1(teensyUSBHost);
USBHub hub2(teensyUSBHost);
MIDIDevice USBMIDI(teensyUSBHost);

// PIN ASSIGNMENTS ##################################################
//MIDI in  is pin 0 by default
//MIDI out is pin 1 by default
const int editPin = 2;        //edit button (push switch on encoder)
const int scl2 = 3;           // Serial Clock 2
const int sda2 = 4;           // Serial Data 2
const int fs1LED = 55;        // LED for Footswitch 1 - 55 cuz pin 5 got fried.
const int topRightLED = 6;    // LED on upper right
const int centerLED = 7;      // LED in center
const int topLeftLED = 8;     // LED on upper left
const int fs0LED = 9;         // LED for Footswitch 0
const int sel_a = 10;         // selectors for analog and digital mux inputs
const int sel_b = 11;
const int sel_c = 12;
const int LED0 = 13;          // the onboard (orange) LED
const int sel_d = 14;         // also SCK0 - use SPI.setSCK(14)
const int fs1Pin = 15;        // footswitch 1 (ring)
const int mic = 16;           // microphone UNTESTED
const int topLeftButton = 19; // onboard buttons
const int centerButton = 22;
const int topRightButton = 23;
const int muxPin0 = 20;       // muxed analog input
const int muxPin1 = 21;       // muxed analog input
const int bottomRightButton = 17;
const int bottomLeftButton = 18;
const int encPinB = 24;       // pin A of encoder
const int encPinA = 25;       // pin B of encoder
const int fs0Pin = 26;        // footswitch 0 (tip)

const uint8_t digitPins[] = {33,30,36,38};
const uint8_t segmentPins[] = {32,31,35,28,34,29,27,37};

const int expPin = 39;        // analog input for expression pedal
const int phonesL = A21;      // headphones L (tip) UNTESTED
const int phonesR = A22;      // headphones R (tip) UNTESTED


// DECLARATOINS ################################################################
bool editMode = false;
bool trackMode = false;

byte MIDIchannel = 3;
bool readMIDIthru = true;
bool readMUX0 = false;
bool readMUX1 = false;

SevSeg DSP;
char DSPstring[5] = "    ";
Editor editor = Editor(encPinA, encPinB, editPin);

Track* Ts[4];
MIDIbutton* Bs[23];
MIDIpot* Ps[18];

uint16_t inLo = 0; // for the paperclip FX
uint16_t inHi = 0; // for the paperclip FX


void setup(){ // INITIALIZATION ########################################################################################
  EEPROM.get(255, MIDIchannel);
  EEPROM.get(1, readMIDIthru);
  EEPROM.get(2, readMUX0);
  EEPROM.get(3, readMUX1);

  DSP.begin(COMMON_CATHODE, 4, digitPins, segmentPins);

  DSP.setBrightness(100); //Set the display to 100% brightness level
  
  Ts[0] = new Track(topLeftButton, 107);
  Ts[1] = new Track(centerButton, 108);
  Ts[2] = new Track(topRightButton, 109);

  Bs[0] = new MIDIbutton(topLeftButton, 102, LATCH, TOUCH);
  Bs[0]->setThreshold();
  Bs[1] = new MIDIbutton(centerButton, 103, LATCH, TOUCH);
  Bs[1]->setThreshold();
  Bs[2] = new MIDIbutton(topRightButton, 104, LATCH, TOUCH);
  Bs[2]->setThreshold();
  Bs[3] = new MIDIbutton(bottomRightButton, 105, LATCH, TOUCH);
  Bs[3]->setThreshold();
  Bs[4] = new MIDIbutton(bottomLeftButton, 106, LATCH, TOUCH);
  Bs[4]->setThreshold();

  Bs[5] = new MIDIbutton(fs1Pin, 80, MOMENTARY);
  EEPROM.get(4, Bs[5]->mode);
  Bs[6] = new MIDIbutton(fs0Pin, 81, LATCH);
  EEPROM.get(8, Bs[6]->mode);
  for(int i=7; i<23; i++){Bs[i] = new MIDIbutton(muxPin0,9+i, 1);} // CC 16~31

  Ps[0] = new MIDIpot(expPin, 85);
  EEPROM.get(12, Ps[0]->mode);
  Ps[0]->inputRange(10, 950);
  for(int i=1; i<17; i++){Ps[i] = new MIDIpot(muxPin0,47+i);} // CC 48~63

  inLo = touchRead(bottomLeftButton) * 1.02;
  inHi = inLo * 1.7;

  pinMode(sel_a, OUTPUT); //analog and digital mux selector 1
  pinMode(sel_b, OUTPUT); //analog and digital mux selector 2
  pinMode(sel_c, OUTPUT); //analog and digital mux selector 3
  pinMode(sel_d, OUTPUT); //analog and digital mux selector 4
  pinMode(fs0LED, OUTPUT);
  pinMode(fs1LED, OUTPUT);
  pinMode(LED0, OUTPUT);
  pinMode(topLeftLED, OUTPUT);
  pinMode(centerLED, OUTPUT);
  pinMode(topRightLED, OUTPUT);

  MIDI1.begin(MIDI_CHANNEL_OMNI); //MIDI1 MIDI2 and MIDI3 only need to send
  MIDI2.begin(MIDI_CHANNEL_OMNI); //no need to read each one
  MIDI3.begin(MIDI_CHANNEL_OMNI); //or set handlers for incoming messages

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOff(onNoteOff);
  MIDI.setHandleNoteOn(onNoteOn);
  MIDI.setHandleAfterTouchPoly(onPolyPressure);
  MIDI.setHandleControlChange(onControl);
  MIDI.setHandleProgramChange(onProgram);
  MIDI.setHandleAfterTouchChannel(onAfterTouch);
  MIDI.setHandlePitchBend(onPitchBend);

  teensyUSBHost.begin();
  USBMIDI.setHandleNoteOff(onUSBNoteOff);
  USBMIDI.setHandleNoteOn(onUSBNoteOn);
  USBMIDI.setHandleAfterTouchPoly(onUSBPolyPressure);
  USBMIDI.setHandleControlChange(onUSBControl);
  USBMIDI.setHandleProgramChange(onUSBProgram);
  USBMIDI.setHandleAfterTouchChannel(onUSBAfterTouch);
  USBMIDI.setHandlePitchChange(onUSBPitchBend);

  // UNCOMMENT THESE TO RESTORE DEFAULTS
//  EEPROM.put(255, 3); EEPROM.put(1, true);    // MIDIchannel, readMIDIthru
//  EEPROM.put(2, false); EEPROM.put(3, false); // readMUX0, readMux1
//  EEPROM.put(4, 0); EEPROM.put(8, 1);         // FS0 mode, FS1 mode
//  EEPROM.put(12, 0);                          // EXP killSwitch #
}


void loop(){ // PROGRAM ################################################################################################
  editor.bounce->update();

  if (editor.bounce->fallingEdge()){
    sprintf(DSPstring, "%4d", MIDIchannel);
  }

  else if (editor.bounce->risingEdge()){
    strcpy(DSPstring, "    ");
    editor.encoder->write(0);

    if (editMode){
      EEPROM.put(255, MIDIchannel);
      EEPROM.put(1, readMIDIthru);
      EEPROM.put(2, readMUX0);
      EEPROM.put(3, readMUX1);
      EEPROM.put(4, Bs[5]->mode);
      EEPROM.put(8, Bs[6]->mode);
      EEPROM.put(12, Ps[0]->killSwitch);
      editMode = false;
    }
    else{trackMode = !trackMode;
    }
    
    if(trackMode){
      for(int i=0; i<3; i++){ // bulk send track volumes
        usbMIDI.sendControlChange(Ts[i]->number, Ts[i]->level, MIDIchannel);
      }
      strcpy(DSPstring, "trac");
    }
    else{
      if(readMUX0){ // bulk send analog values
        for(int i=1; i<9; i++){ // Use i<9 for MUX8, i<17 for MUX16
          usbMIDI.sendControlChange(Ps[i]->number, Ps[i]->value, MIDIchannel);          
        }
      }
      strcpy(DSPstring, "ctrl");
    }
  }
  else if (editor.bounce->read() == LOW){
    selectMode();
  }
  else{
    tracOrCtrlMode();
    if (trackMode){ tracMode(); } else{ ctrlMode(); }
  }

  DSP.setChars(DSPstring);

  DSP.refreshDisplay(); // Display most recently stored information
}

//######################################################################################################################
void selectMode(){
  
//   int newChannel = editor.editChannel();
//   if (newChannel != MIDIchannel){
//     MIDIchannel = newChannel;
//     editMode = true;
//     sprintf(DSPstring, "%4d", MIDIchannel);
//   }

//   for (int i=0; i<5; i++){
//     if (Bs[i]->read() == 127){
//       Bs[i]->mode = Bs[i]->mode + 1;
//       if (Bs[i]->mode > 3){Bs[i]->mode = 0;}
//       editMode = true;
//     }
//   }
//   if (Bs[0]->read() == 127){
//     readMIDIthru = !readMIDIthru;
//     editMode = true;
//   }
//   if (Bs[1]->read() == 127){
//     readMUX0 = !readMUX0;
//     editMode = true;
//   }
//   if (Bs[2]->read() == 127){
//     readMUX1 = !readMUX1;
//     editMode = true;
//   }
//   if (Bs[3]->read() == 127){
//     // do something with this button
//     editMode = true;
//   }
//   if (Bs[4]->read() == 127){
//     if(readMUX1){
//       for(int i=1; i < 9; i++){ // USE i<9 FOR MUX8, i<17 FOR MUX16
// //        digitalWrite(sel_d, (i&15)>>3); // COMMENT FOR MUX8, UN- FOR MUX16
//         digitalWrite(sel_d, (i&7)>>2);
//         digitalWrite(sel_c, (i&3)>>1);
//         digitalWrite(sel_b, (i&1));

//         Ps[i+1]->send(FORCE);
//       }
//     }
//   }

//   if (Bs[5]->read() > 0){
//     Bs[5]->mode = !Bs[5]->mode;
//     editMode = true;
//     if(Bs[5]->mode){strcpy(DSPstring, "lach");}
//     else {strcpy(DSPstring, "hold");}
//   }
//   if (Bs[6]->read() > 0){
//     Bs[6]->mode = !Bs[6]->mode;
//     editMode = true;
//     if(Bs[6]->mode){strcpy(DSPstring, "lach");}
//     else {strcpy(DSPstring, "hold");}
//   }

//   if (Ps[0]->read() == Ps[0]->outHi){
//     Ps[0]->killSwitch = !Ps[0]->killSwitch;
//     editMode = true;
//     if(Ps[0]->killSwitch){strcpy(DSPstring, " cut");}
//     else{strcpy(DSPstring, "-cut");}
//   }
  
  digitalWrite(topLeftLED, readMIDIthru);
  digitalWrite(centerLED, readMUX0);
  digitalWrite(topRightLED, readMUX1);
}


void tracMode(){
  int incdec = editor.encoder->read();
  for(int i=0; i<3; i++){
    int newState = Ts[i]->send();
    if(newState >= 0){
      sprintf(DSPstring, "%4d", newState);
    }
    if (incdec){
      int newLevel = Ts[i]->vol(incdec);
      if(newLevel >= 0){
        sprintf(DSPstring, "%4d", newLevel);
      }
    }
  }
  if (incdec){editor.encoder->write(0);}
  byte recState = record(Bs[5]->read() == 127, Bs[6]->read() == 127);
  digitalWrite(fs0LED, recState);
  digitalWrite(fs1LED, !digitalRead(fs0Pin));
  digitalWrite(topLeftLED, Ts[0]->state);
  digitalWrite(centerLED, Ts[1]->state);
  digitalWrite(topRightLED, Ts[2]->state);
}


void ctrlMode(){
  int newVal = editor.send();
  if(newVal >= 0){
    sprintf(DSPstring, "%4d", editor.level);
    DSPstring[0] = 'r';
  }
  for(int i=0;i<5;i++){ // Send MIDI for Onboard Buttons
    newVal = Bs[i]->send();
    if (newVal >= 0){
      sprintf(DSPstring, "%4d", newVal);
      DSPstring[0] = 'b';
    }
  }

  for(int i=5;i<7;i++){ // Send MIDI for Foot Switches
    newVal = Bs[i]->send();
    if (newVal >= 0){
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


void tracOrCtrlMode(){
  int newVal = 0;
  newVal = Ps[0]->send();              // Send MIDI for EXP Pedal
  if (newVal >= 0){
    sprintf(DSPstring, "%4d", newVal);      
    DSPstring[0] = 'E';
  }

  if (readMIDIthru){
    teensyUSBHost.Task();
    USBMIDI.read();
    MIDI.read();
    int newValue = touchRead(bottomRightButton);
    chaos(LED0, newValue, inLo, inHi, 48, 84); // TODO: Other cool MIDI functions
  }
  else{ // Ignore (don't buffer) incoming MIDI.
    while(MIDI1.read()){} // TODO: also find a way to ignore USB host MIDI.
  }

  if (readMUX0){
    newVal = Ps[1]->send();
    if (newVal >= 0){
      sprintf(DSPstring, "%4d", newVal);
      DSPstring[0] = 'h';
    }
  }

  if (readMUX1){
    for(int i=0; i<8; i++){               // USE i<8 FOR MUX8, i<16 FOR MUX16
//        digitalWrite(sel_d, (i&15)>>3); // COMMENT FOR MUX8, UN- FOR MUX16
      digitalWrite(sel_d, (i&7)>>2);
      digitalWrite(sel_c, (i&3)>>1);
      digitalWrite(sel_b, (i&1));

      newVal = Ps[i+1]->send();
      if (newVal >= 0){
        sprintf(DSPstring, "%4d", newVal);
        DSPstring[0] = 'A';
      }
    }
  }
}