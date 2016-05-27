#include "MOMIPLUG.h"
//#include <usb_keyboard.h> // if also using for keyboard input

// PIN ASSIGNMENTS ##################################################
                         //MIDI input is read from pin 0 by default
const int ringPin = 1;   //footswitch 1
const int sel_a = 2;     //selectors for analog mux
const int sel_b = 3;     //
const int sel_c = 4;     //
const int sel_d = 5;     //
const int tipPin = 6;    //footswitch 0
const int clockPin = 7;  //clock pin for shift registers
const int latchPin = 8;  //latch pin for shift registers
                         //pins 9 ~ 13 reserved for USB host module
const int but0pin = 14;  //onboard buttons
const int but1pin = 15;  //
const int but2pin = 16;  //
const int but3pin = 17;  //
const int but4pin = 18;  //
const int editPin = 19;  //edit button (push switch on encoder)
const int encPinA = 20;  //pin A of encoder
const int encPinB = 21;  //pin B of encoder
const int serPin = 22;   //serial output to shift registers 
const int expPin = A9;   //analog input for expression pedal
const int muxPin0 = A10; //analog input from muxes
const int muxPin1 = A11; //
byte MIDIchannel;
bool readFS;
bool readEXP;
bool readMUX;
bool readMIDI;
bool readUSB;
bool trackMode;
bool pauseMUXread;// keeps MUX selectors stable until the selected
                  // pot is edited or non-MUXed input is triggered       

// DECLARATOINS ##################################################
Editor editor = Editor(encPinA, encPinB, editPin);
Display DSP(serPin, clockPin, latchPin);
Track* Ts[5];
MIDIbutton* Bs[7];
MIDIpot* Ps[17];
MIDIenc* Es[0];
MIDInote* Ns[0];
MIDIcapSens* Cs[0];

uint8_t states[25]{
  7,                  //default MIDIchannel 
  1, 1, 0, 1, 1, 0,   //interface booleans
  0,                  //Onboard Encoder level
  0, 0, 0, 0, 0,      //Track levels
  0, 0, 0, 0, 0,      //Track states
  0, 0, 0, 0, 0, 0, 0 //Button/FS states
};
uint8_t buttonSettings[8]{
    14, 0, 127, 1,    //FS0 number, min, max, mode
    15, 0, 127, 1     //FS1 number, min, max, mode
};
uint8_t potSettings8bit[4]{
    9, 0, 127, 1      //EXP number, min out, max out, kill
};

// INITIALIZATION ##################################################
void setup(){
//  Uncomment only to restore all default settings.
//  for(uint8_t i=0; i<25;i++){EEPROM.put(i, states[i]);}
//  for(uint8_t i=0;i<sizeof(buttonSettings); i++){EEPROM.put(i+25, buttonSettings[i]);}
//  for(uint8_t i=0;i<sizeof(potSettings8bit); i++){EEPROM.put(i+161, potSettings8bit[i]);}

  pauseMUXread = false;
  EEPROM.get(0, MIDIchannel);
  EEPROM.get(1, readFS);
  EEPROM.get(2, readEXP);
  EEPROM.get(3, readMUX);
  EEPROM.get(4, readMIDI);
  EEPROM.get(5, readUSB);
  EEPROM.get(6, trackMode);
  EEPROM.get(7, editor.level);

  Ts[0] = new Track(but0pin, 80); // read number, level and state from EEPROM
  Ts[1] = new Track(but1pin, 81);
  Ts[2] = new Track(but2pin, 82);
  Ts[3] = new Track(but3pin, 83);
  Ts[4] = new Track(but4pin, 84);
  for (int i=0; i<5; i++){
    Ts[i]->level = EEPROM.read(i+8);
    Ts[i]->state = EEPROM.read(i+13);
  }

  Bs[0] = new MIDIbutton(but0pin, 85, 1);
  Bs[1] = new MIDIbutton(but1pin, 86, 1);
  Bs[2] = new MIDIbutton(but2pin, 87, 1);
  Bs[3] = new MIDIbutton(but3pin, 89, 1);
  Bs[4] = new MIDIbutton(but4pin, 90, 1);
  Bs[5] = new MIDIbutton(ringPin, EEPROM.read(25), EEPROM.read(26), EEPROM.read(27), EEPROM.read(28));
  Bs[6] = new MIDIbutton(tipPin,  EEPROM.read(29), EEPROM.read(30), EEPROM.read(31), EEPROM.read(32));
  for (int i=0; i<7; i++){Bs[i]->state = EEPROM.read(i+18);}
  Bs[5]->myButt->update(); // Will crash if not updated immediately after wake.
  Bs[6]->myButt->update(); // Will crash if not updated immediately after wake.
  
  Ps[0] = new MIDIpot(expPin, EEPROM.read(161), EEPROM.read(162), EEPROM.read(163), EEPROM.read(164));

  for(int i=1; i<9; i++){
    Ps[i] = new MIDIpot(muxPin0,16+i);  // 17~24 are sliders
    Ps[i+8] = new MIDIpot(muxPin1,24+i);// 25~32 are pots
  }

  pinMode(muxPin0, INPUT);            //analog input from muxes
  pinMode(muxPin1, INPUT);            //
  pinMode(sel_a, OUTPUT);             //analog mux selector 1
  pinMode(sel_b, OUTPUT);             //analog mux selector 2
  pinMode(sel_c, OUTPUT);             //analog mux selector 3
//  pinMode(sel_d, OUTPUT);           //analog mux selector 4 (if MUX16)

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOff(onNoteOff);
  MIDI.setHandleNoteOn(onNoteOn);
  MIDI.setHandleAfterTouchPoly(onPolyPressure);
  MIDI.setHandleControlChange(onControl);
  MIDI.setHandleProgramChange(onProgram);
  MIDI.setHandleAfterTouchChannel(onAfterTouch);
  MIDI.setHandlePitchBend(onPitchBend);
}

// PROGRAM ##################################################
void loop(){
  editor.myButt->update();
  // On Button Release ##################################################
  if (editor.myButt->risingEdge()){
    DSP.clear();
    editor.myKnob->write(0);
    if (editor.editing == true){
      if (editor.storageIndex == 0){
        editor.editing = false;
        pauseMUXread = false; // true again now that we're finished editing
      }
      else if(editor.storageIndex >= 165){
        editor.editPot(*editor.pTarget);
        editor.editing = false;
        pauseMUXread = false; // true again now that we're finished editing
      }
      else if(editor.storageIndex >= 161){
        editor.editPot(*editor.pTarget);
        editor.editing = false;
        pauseMUXread = false; // true again now that we're finished editing
      }
      else if(editor.storageIndex >= 25){
        editor.editButton(*editor.bTarget);
        editor.editing = false;
        pauseMUXread = false; // true again now that we're finished editing
      }
      else{
        editor.editing = false;
        pauseMUXread = false;
      }
    }
    else{
      trackMode = !trackMode;
      EEPROM.put(6, trackMode);
      if(trackMode){
        DSP.verbal("tr ");
      }
      else{
        DSP.verbal("CC ");
      }
    }
  }
  // On Button Press ##################################################
  else if (editor.myButt->fallingEdge()){
    DSP.clear();
  }
  // On Hold ##################################################
  else if (editor.myButt->read() == LOW){
    int newChannel = editor.editChannel();
    if (newChannel != MIDIchannel){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      MIDIchannel = newChannel;
      editor.storageIndex = 0;
      EEPROM.put(0, MIDIchannel);
    }
    Bs[0]->myButt->update();
    if (Bs[0]->myButt->fallingEdge()){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      readFS = !readFS;
      EEPROM.put(1, readFS);
    }
    Bs[1]->myButt->update();
    if (Bs[1]->myButt->fallingEdge()){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      readEXP = !readEXP;
      EEPROM.put(2, readEXP);
    }
    Bs[2]->myButt->update();
    if (Bs[2]->myButt->fallingEdge()){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      readMUX = !readMUX;
      EEPROM.put(3, readMUX);
    }
    Bs[3]->myButt->update();
    if (Bs[3]->myButt->fallingEdge()){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      readMIDI = !readMIDI;
      EEPROM.put(4, readMIDI);
    }
    Bs[4]->myButt->update();
    if (Bs[4]->myButt->fallingEdge()){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      readUSB = !readUSB;
      EEPROM.put(5, readUSB);
    }
    if (readFS){
      for (uint8_t i=0;i<2;i++){
        if (Bs[i+5]->read() >= 0){ //values can't be stored for the first 5 (on board) buttons
          pauseMUXread = false; // now that we're editing something else
          editor.editing = true;
          editor.storageIndex = i*4+25;
          //Button's index * number of bytes stored for each button + adress where button storage begins
          editor.bTarget = Bs[i+5];
        }
      }
    }
    if (readEXP && Ps[0]->read() >= 0){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      editor.storageIndex = 161; //Pot storage begins at EEPROM address 161
      editor.pTarget = Ps[0];
    }
    if (readMUX && !pauseMUXread){// Read analog mux
      for(int i=0; i<8; i++){
        digitalWrite(sel_a, (i&7)>>2);
        digitalWrite(sel_b, (i&3)>>1);
        digitalWrite(sel_c, (i&1));
        if (Ps[i+1]->read() >= 0){
          editor.editing = true;
          editor.storageIndex = (i+1)*4+161;
          //Pot's index * number of bytes stored for each pot + adress where pot storage begins
          editor.pTarget = Ps[i+1];
          pauseMUXread = true; //until finished editing
          break;
        }
        if (Ps[i+9]->read() >= 0){
          editor.editing = true;
          editor.storageIndex = (i+9)*4+161;
          //Pot's index * number of bytes stored for each pot + adress where pot storage begins
          editor.pTarget = Ps[i+9];
          pauseMUXread = true; //until finished editing
          break;
        }
      }
    }
    DSP.value(MIDIchannel); // Display channel and inputs being read when held
    DSP.states(false, false, false, readFS, readEXP, readMUX, readMIDI, readUSB);
  }
  // In Track Mode ##################################################
  else if (trackMode == true){
    int incdec = editor.myKnob->read();
    for(int i=0; i<5; i++){
      int newState = Ts[i]->send();
      if(newState >= 0){
        EEPROM.put(i+13, Ts[i]->state);
        DSP.value(newState);
      }
      if (incdec){
        int newLevel = Ts[i]->vol(incdec);
        if(newLevel >= 0){
          EEPROM.put(i+8, newLevel);
          DSP.value(newLevel);
        }
      }
    }
    if (incdec){
      editor.myKnob->write(0);
    }
//    record(Bs[5]->myButt, Bs[6]->myButt, *Ts[]);
    DSP.states(trackMode, Bs[5]->state, Bs[6]->state, Ts[0]->state, Ts[1]->state, Ts[2]->state, Ts[3]->state, Ts[4]->state);
  }
  // In Control Mode ##################################################
  else{
    int newEncVal = editor.send();
      if(newEncVal >= 0 ){
        EEPROM.put(7, newEncVal);
        DSP.value(newEncVal);
        editor.myKnob->write(0);
      }
    for(int i=0; i<5; i++){   // Send MIDI for on-board buttons
      int newState = Bs[i]->send();
      if (newState >= 0){
        EEPROM.put(i+18, Bs[i]->state);
        DSP.value(newState);
      }
    }
    if (readFS){              // Send MIDI for Foot Switches
      for(int i=5; i<7; i++){
        int newState = Bs[i]->send();
        if (newState >= 0){
          EEPROM.put(i+18, Bs[i]->state);
          DSP.value(newState);
        }
      }
    }
    if (readEXP){             // Send MIDI for EXP Pedal
      DSP.value(Ps[0]->send());
    }
    if (readMUX){             // Send MIDI for Muxed input
      for(int i=0; i<8; i++){
        digitalWrite(sel_a, (i&7)>>2);
        digitalWrite(sel_b, (i&3)>>1);
        digitalWrite(sel_c, (i&1));
        DSP.value(Ps[i+1]->send());
        DSP.value(Ps[i+9]->send());
      }
    }
    if (readMIDI){            // MIDI to usbMIDI
      MIDI.read();    
    }
    if (readUSB){             // read USB input
      // TODO: set this up
    }
    DSP.states(trackMode, Bs[5]->state, Bs[6]->state, Bs[0]->state, Bs[1]->state, Bs[2]->state, Bs[3]->state, Bs[4]->state);
  }
  DSP.print(); // Display most recently stored information
}

