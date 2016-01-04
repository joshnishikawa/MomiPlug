#include "MOMIPLUG.h"
//#include <usb_keyboard.h> // if also using for keyboard input

int MIDIchannel = 3;

const int fstippin = 0;    //footswitch 1
const int fsringpin = 1;   //footswitch 2
const int sel_a = 2;       //selectors for analog mux
const int sel_b = 3;       //
const int sel_c = 4;       //
const int sel_d = 5;       //
const int midipin = 6;     //MIDI input from 5-pin DIN
const int clockpin = 7;    //clock pin for shift registers
const int latchpin = 8;    //latch pin for shift registers
const int but0pin = 14;    //onboard buttons
const int but1pin = 15;    //
const int but2pin = 16;    //
const int but3pin = 17;    //
const int but4pin = 18;    //
const int editpin = 19;    //edit button (push switch on encoder)
const int encpinA = 20;    //pin A of encoder
const int encpinB = 21;    //pin B of encoder
const int serialpin = 22;  //serial output to shift registers 
const int expedpin = A9;   //analog input for expression pedal
const int muxpin0 = A10;   //analog input from muxes
const int muxpin1 = A11;   //

Editor Edit(editpin, encpinA, encpinB);
MIDIbutton *Bs[5];
Track *Ts[5];
MIDIbutton FS0(fsringpin, 14, 1);
MIDIbutton FS1(fstippin, 15, 1);
MIDIpot EXP(expedpin, 9, 1);
MIDIpot *Ps[16];
Display DSP(serialpin, clockpin, latchpin);

void setup(){
  Bs[0] = new MIDIbutton(but0pin, 85, 1);
  Bs[1] = new MIDIbutton(but1pin, 86, 1);
  Bs[2] = new MIDIbutton(but2pin, 87, 1);
  Bs[3] = new MIDIbutton(but3pin, 89, 1);
  Bs[4] = new MIDIbutton(but4pin, 90, 1);
  
  Ts[0] = new Track(but0pin);
  Ts[1] = new Track(but1pin);
  Ts[2] = new Track(but2pin);
  Ts[3] = new Track(but3pin);
  Ts[4] = new Track(but4pin);
  
  for(int i=0; i<8; i++){
    Ps[i] = new MIDIpot(muxpin0,16+i); // 16~23 are pots
  }
  for(int i=0; i<8; i++){
    Ps[i] = new MIDIpot(muxpin1,24+i); // 24~31 are sliders
  }

  pinMode(muxpin0, INPUT);     //analog input from muxes
  pinMode(muxpin1, INPUT);
  pinMode(sel_a, OUTPUT);      //analog mux selector 1
  pinMode(sel_b, OUTPUT);      //analog mux selector 2
  pinMode(sel_c, OUTPUT);      //analog mux selector 3
//  pinMode(sel_d, OUTPUT);      //analog mux selector 4
//  pinMode(midipin, INPUT);     //pin for standard MIDI input
  MIDI.begin();
}

void loop(){
  
  Edit.check(Bs, Ps, FS1, FS0);

  if(Edit.state == true){
    DSP.value(Edit.read(Ts, FS1, FS0));
  }
  else{
    for(int i=0;i<5;i++){
      Bs[i]->read();
    }
    FS1.read();
    FS0.read();
    EXP.read();
    DSP.clear();
  }
  DSP.info(Bs, Ts, Edit, FS1, FS0);
  
/*  UNCOMMENT WHEN THERE ARE MORE BUTTONS
    for(int i=5;i<sizeof(Bs)/sizeof(Bs[0]);i++){ 
    Bs[i]->Read();
  }
*/

/*  UNCOMMENT WHEN THERE ARE MORE POTS
    for (int i=0; i<8; i++){
    digitalWrite(sel_a, (i&7)>>2);
    digitalWrite(sel_b, (i&3)>>1);
    digitalWrite(sel_c, (i&1));
    Pots[i]->read();
    Pots[i+8]->read();
  }
*/

}

