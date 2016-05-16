#include "MOMIPLUG.h"
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
bool trackMode = false;
bool readFS = true;
bool readEXP = true;
bool readMUX = true;
bool readMIDI = true;
bool readUSB = true;
int MIDIchannel = 2;
int* MC = &MIDIchannel;

MIDIenc* Es[1];
MIDIbutton* Bs[7];
MIDIpot* Ps[17];
MIDInote* Ns[0];
MIDIcapSens* Cs[0];
Track* Ts[5];
Display DSP(serPin, clockPin, latchPin);

//INITIALIZATION #####################################################
void setup(){
  Es[0] = new MIDIenc(encPinA, encPinB, 12);

  Bs[0] = new MIDIbutton(but0pin, 85, 1);
  Bs[1] = new MIDIbutton(but1pin, 86, 1);
  Bs[2] = new MIDIbutton(but2pin, 87, 1);
  Bs[3] = new MIDIbutton(but3pin, 89, 1);
  Bs[4] = new MIDIbutton(but4pin, 90, 1);
  Bs[5] = new MIDIbutton(ringPin, 14, 1);
  Bs[6] = new MIDIbutton(tipPin, 15, 1);

  Ts[0] = new Track(but0pin, 80);
  Ts[1] = new Track(but1pin, 81);
  Ts[2] = new Track(but2pin, 82);
  Ts[3] = new Track(but3pin, 83);
  Ts[4] = new Track(but4pin, 84);

  Ps[0] = new MIDIpot(expPin, 9, 1);
  for(int i=1; i<9; i++){
    Ps[i] = new MIDIpot(muxPin0,16+i);  // 16~23 are pots
    Ps[i+8] = new MIDIpot(muxPin1,24+i);// 24~31 are sliders
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

Editor editor(editPin, Es[0]->myKnob);

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
    else {trackMode = !trackMode;}
  }
  //On Button Press ##################################################
  else if (editor.myButt->fallingEdge()){
    DSP.clear();
  }
  //On Hold ##########################################################
  else if (editor.myButt->read() == LOW){
    MIDIchannel = editor.editChannel(*Es[0]->myKnob, MIDIchannel);
    Bs[0]->myButt->update();
    if (Bs[0]->myButt->fallingEdge()){
      editor.editing = true;
      readFS = !readFS;
    }
    Bs[1]->myButt->update();
    if (Bs[1]->myButt->fallingEdge()){
      editor.editing = true;
      readEXP = !readEXP;
    }
    Bs[2]->myButt->update();
    if (Bs[2]->myButt->fallingEdge()){
      editor.editing = true;
      readMUX = !readMUX;
    }
    Bs[3]->myButt->update();
    if (Bs[3]->myButt->fallingEdge()){
      editor.editing = true;
      readMIDI = !readMIDI;
    }
    Bs[4]->myButt->update();
    if (Bs[4]->myButt->fallingEdge()){
      editor.editing = true;
      readUSB = !readUSB;
    }
    for (uint8_t i=7;i<(sizeof(Bs)/sizeof(Bs[i]));i++){
      if (Bs[i]->read() >= 0){
        editor.editing = true;
        editor.target = &Bs[i];
        editor.targetSize = sizeof(*Bs[i]);
      }
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
      for (uint8_t i=0;i<(sizeof(Ps)/sizeof(Ps[i]))-1;i++){ // the -1 is because the exp pedal is read separately
        if (Ps[i]->read() >= 0){
          editor.editing = true;
          editor.target = &Ps[i];
          editor.targetSize = sizeof(*Ps[i]);
        }
      }
    }
    for (uint8_t i=0;i<(sizeof(Ns)/sizeof(Ns[i]));i++){
      if (Ns[i]->read() >= 0){
        editor.editing = true;
        editor.target = &Ns[i];
        editor.targetSize = sizeof(*Ns[i]);
      }
    }
    DSP.value(MIDIchannel);
    DSP.states(0, 0, 0, readFS, readEXP, readMUX, readMIDI, readUSB);
  }
  //In Track Mode ####################################################
  else if (trackMode == true){
    int incdec = Es[0]->myKnob->read();
    for(int i=0; i<5; i++){
      DSP.value(Ts[i]->read());
      if (incdec){
        DSP.value(Ts[i]->vol(incdec, i));
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
    DSP.value(Es[0]->send()); // Read on board encoder
    for(int i=0; i<7; i++){   // Send MIDI for on-board buttons & foot switches
      DSP.value(Bs[i]->send());
    }
    if (readFS){
      DSP.value(Bs[5]->send());
      DSP.value(Bs[6]->send());
    }
    if (readEXP){
      DSP.value(Ps[0]->send());
    }
    if (readMUX){
      for(int i=0; i<8; i++){
        digitalWrite(sel_a, (i&7)>>2);
        digitalWrite(sel_b, (i&3)>>1);
        digitalWrite(sel_c, (i&1));
        DSP.value(Ps[i]->send());
        DSP.value(Ps[i+8]->send());
      }
    }
    if (readMIDI){
      MIDI.read();    
    }
    if (readUSB){
      // TODO: set this up
    }
    DSP.states(trackMode, Bs[5]->state, Bs[6]->state, Bs[0]->state, Bs[1]->state, Bs[2]->state, Bs[3]->state, Bs[4]->state);
  }
  DSP.print(); // Display most recently updated CC value
}

