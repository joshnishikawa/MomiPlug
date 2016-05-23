#include "MOMIPLUG.h"
#include <EEPROM.h>
//#include <usb_keyboard.h> // if also using for keyboard input

//DECLARATIONS #######################################################
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
byte MIDIchannel; // read from EEPROM
bool readFS;      //
bool readEXP;     //
bool readMUX;     //
bool readMIDI;    //
bool readUSB;     //
bool trackMode;   //

Track* Ts[5];
MIDIenc* Es[1];
MIDIbutton* Bs[7];
MIDIpot* Ps[17];
MIDInote* Ns[0];
MIDIcapSens* Cs[0];
Display DSP(serPin, clockPin, latchPin);

//INITIALIZATION #####################################################
uint8_t preset8bit[]{
  7, 1, 1, 0, 1, 1, 0,          //channel & interface bools
  0, 0, 0, 0, 0,                //Track levels
  0, 0, 0, 0, 0,                //Track states
  14, 15, 0, 0, 127, 127, 1, 1, //Footswitch numbers, mins, maxs, modes
  0, 0, 0, 0, 0, 0, 0,          //Button/FS states
  9, 0, 127, 1,                 //EXP number, min out, max out, kill
  0                             //Onboard Encoder level
};
uint16_t preset16bit[]{
  0, 1023                       //EXP pedal analog input min and max
};

void setup(){
//UNCOMMENT ONLY TO RESTORE ALL DEFAULT SETTINGS
//  for(uint8_t i=0; i<(sizeof(preset8bit)/sizeof(preset8bit[0]));i++){EEPROM.put(i, preset8bit[i]);}
//  for(uint16_t i=40;i<(sizeof(preset16bit)/sizeof(preset16bit[0])); i+=2){EEPROM.put(i, preset16bit[i/2-20]);}

  EEPROM.get(0, MIDIchannel);
  EEPROM.get(1, readFS);
  EEPROM.get(2, readEXP);
  EEPROM.get(3, readMUX);
  EEPROM.get(4, readMIDI);
  EEPROM.get(5, readUSB);
  EEPROM.get(6, trackMode);
  Ts[0] = new Track(but0pin, 80); // read number, level and state from EEPROM
  Ts[1] = new Track(but1pin, 81);
  Ts[2] = new Track(but2pin, 82);
  Ts[3] = new Track(but3pin, 83);
  Ts[4] = new Track(but4pin, 84);
  for (int i=0; i<5; i++){
    Ts[i]->level = EEPROM.read(i+7);
    Ts[i]->state = EEPROM.read(i+12);
  }

  Bs[0] = new MIDIbutton(but0pin, 85, 1);
  Bs[1] = new MIDIbutton(but1pin, 86, 1);
  Bs[2] = new MIDIbutton(but2pin, 87, 1);
  Bs[3] = new MIDIbutton(but3pin, 89, 1);
  Bs[4] = new MIDIbutton(but4pin, 90, 1);
  Bs[5] = new MIDIbutton(ringPin, EEPROM.read(17), EEPROM.read(19), EEPROM.read(21), EEPROM.read(23));
  Bs[6] = new MIDIbutton(tipPin,  EEPROM.read(18), EEPROM.read(20), EEPROM.read(22), EEPROM.read(24));
  Bs[5]->myButt->update(); // Will crash if not updated immediately after wake.
  Bs[6]->myButt->update(); // Will crash if not updated immediately after wake.
  for (int i=0; i<7; i++){Bs[i]->state = EEPROM.read(i+25);}

  
  Ps[0] = new MIDIpot(expPin, EEPROM.read(32), EEPROM.read(33), EEPROM.read(34), EEPROM.read(35));
//  Ps[0]->inputRange(EEPROM.read(40), EEPROM.read(42));
  
  Es[0] = new MIDIenc(encPinA, encPinB, 12);
  EEPROM.get(36, Es[0]->value);
  
  for(int i=1; i<9; i++){
    Ps[i] = new MIDIpot(muxPin0,16+i);  // 17~24 are pots
    Ps[i+8] = new MIDIpot(muxPin1,24+i);// 25~32 are sliders
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

Editor editor(editPin, Es[0]->myKnob); // Here because Es[0] must be constructed first

//PROGRAM ############################################################
void loop(){
  editor.myButt->update();
  //On Button Release ################################################
  if (editor.myButt->risingEdge()){
    DSP.clear();
    Es[0]->myKnob->write(0);
    if (editor.editing == true){
      editor.edit();
    }
    else {
      trackMode = !trackMode;
      EEPROM.put(6, trackMode);
    }
  }
  //On Button Press ##################################################
  else if (editor.myButt->fallingEdge()){
    DSP.clear();
  }
  //On Hold ##########################################################
  else if (editor.myButt->read() == LOW){
    int newChannel = editor.editChannel(*Es[0]->myKnob);
    if (MIDIchannel != newChannel){
      MIDIchannel = newChannel;
      EEPROM.put(0, MIDIchannel);
    }
    Bs[0]->myButt->update();
    if (Bs[0]->myButt->fallingEdge()){
      editor.editing = true;
      readFS = !readFS;
      EEPROM.put(1, readFS);
    }
    Bs[1]->myButt->update();
    if (Bs[1]->myButt->fallingEdge()){
      editor.editing = true;
      readEXP = !readEXP;
      EEPROM.put(2, readEXP);
    }
    Bs[2]->myButt->update();
    if (Bs[2]->myButt->fallingEdge()){
      editor.editing = true;
      readMUX = !readMUX;
      EEPROM.put(3, readMUX);
    }
    Bs[3]->myButt->update();
    if (Bs[3]->myButt->fallingEdge()){
      editor.editing = true;
      readMIDI = !readMIDI;
      EEPROM.put(4, readMIDI);
    }
    Bs[4]->myButt->update();
    if (Bs[4]->myButt->fallingEdge()){
      editor.editing = true;
      readUSB = !readUSB;
      EEPROM.put(5, readUSB);
    }
    if (readFS){
      for (uint8_t i=5;i<7;i++){
        if (Bs[i]->read() >= 0){
          editor.editing = true;
          editor.target = &Bs[i];
          editor.targetSize = sizeof(*Bs[i]);
        }
      }
    }
    if (readEXP && Ps[0]->read() >= 0){
      editor.editing = true;
      editor.target = &Ps[0];
      editor.targetSize = sizeof(*Ps[0]);
    }
    if (readMUX){// Read analog mux
      for (uint8_t i=1;i<(sizeof(Ps)/sizeof(Ps[i]));i++){ // the -1 is because the exp pedal is read separately
        if (Ps[i]->read() >= 0){
          editor.editing = true;
          editor.target = &Ps[i];
          editor.targetSize = sizeof(*Ps[i]);
        }
      }
    }
    DSP.value(MIDIchannel);
    DSP.states(false, false, false, readFS, readEXP, readMUX, readMIDI, readUSB);
  }
  //In Track Mode ####################################################
  else if (trackMode == true){
    int incdec = Es[0]->myKnob->read();
    for(int i=0; i<5; i++){
      int newState = Ts[i]->send();
      if(newState >= 0){
        EEPROM.put(i+12, Ts[i]->state);
        DSP.value(newState);
      }
      if (incdec){
        int newLevel = Ts[i]->vol(incdec);
        if(newLevel >= 0){
          EEPROM.put(i+7, newLevel);
          DSP.value(newLevel);
        }
      }
    }
    if (incdec){
      Es[0]->myKnob->write(0);
    }
//    record(Bs[5]->myButt, Bs[6]->myButt, *Ts[]);
    DSP.states(trackMode, Bs[5]->state, Bs[6]->state, Ts[0]->state, Ts[1]->state, Ts[2]->state, Ts[3]->state, Ts[4]->state);
  }
  //In Control Mode ##################################################
  else{
    int newEncVal = Es[0]->send();
      if(newEncVal >= 0 ){
        EEPROM.put(36, newEncVal);
        DSP.value(newEncVal);
        Es[0]->myKnob->write(0);
      }
    for(int i=0; i<5; i++){   // Send MIDI for on-board buttons
      int newState = Bs[i]->send();
      if (newState >= 0){
        EEPROM.put(i+25, Bs[i]->state);
        DSP.value(newState);
      }
    }
    if (readFS){              // Send MIDI for Foot Switches
      for(int i=5; i<7; i++){
        int newState = Bs[i]->send();
        if (newState >= 0){
          EEPROM.put(i+25, Bs[i]->state);
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
        DSP.value(Ps[i]->send());
        DSP.value(Ps[i+8]->send());
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

