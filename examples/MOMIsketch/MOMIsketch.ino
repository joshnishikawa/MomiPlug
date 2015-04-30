#include <MIDI.h>
#include <Bounce.h>
#include <Encoder.h>
#include "button.h"
#include "pot.h"
#include "display.h"
#include "editor.h"

#define fstippin 0    //footswitch 1
#define fsringpin 1   //footswitch 2
#define sel_a 2       //selectors for analog mux
#define sel_b 3       //
#define sel_c 4       //
#define sel_d 5       //
#define midipin 6     //MIDI input from 5-pin DIN
#define clockpin 7    //clock pin for shift registers
#define latchpin 8    //latch pin for shift registers
#define intpin 9      //interrupt pin for USB host
#define usbsspin 10   //SS pin for USB host
#define mosipin 11    //MOSI to USB host
#define misopin 12    //MISO from USB host
#define usbsckpin 13  //serial clock for USB host
#define but0pin 14    //onboard buttons
#define but1pin 15    //
#define but2pin 16    //
#define but3pin 17    //
#define but4pin 18    //
#define editpin 19    //edit button (push switch on encoder)
#define encpinA 20    //pin A of encoder
#define encpinB 21    //pin B of encoder
#define serialpin 22  //serial output to shift registers 
#define expedpin A9   //analog input for expression pedal
#define muxpin0 A10   //analog input from muxes
#define muxpin1 A11   //

Editor Edit(editpin, encpinA, encpinB);
Button *Bs[5];
Track *Ts[5];
Button FS1(fstippin, 25);
Button FS2(fsringpin, 26);
Pot EXP(expedpin, 27);
Pot *Ps[16];
Display DSP(serialpin, clockpin, latchpin);

void setup(){
  Bs[0] = new Button(but0pin, 20, 1);
  Bs[1] = new Button(but1pin, 21, 1);
  Bs[2] = new Button(but2pin, 22, 1);
  Bs[3] = new Button(but3pin, 23, 1);
  Bs[4] = new Button(but4pin, 24, 1);
  
  Ts[0] = new Track(but0pin);
  Ts[1] = new Track(but1pin);
  Ts[2] = new Track(but2pin);
  Ts[3] = new Track(but3pin);
  Ts[4] = new Track(but4pin);
  
  for(int i=0; i<8; i++){
    Ps[i] = new Pot(muxpin0,28+i);
  }
  
  for(int i=8; i<16; i++){
    Ps[i] = new Pot(muxpin1,36+i);
  }

  pinMode(muxpin0, INPUT);     //analog input from muxes
  pinMode(muxpin1, INPUT);
//  pinMode(usbsspin, OUTPUT);   //SS pin for USB host module
//  pinMode(mosipin, OUTPUT);    //MOSI to USB host module
//  pinMode(misopin, INPUT);     //MISO from USB host module
//  pinMode(usbsckpin, OUTPUT);  //serial clock for USB host module
//  pinMode(intpin, INPUT);      //DON'T REALLY KNOW YET
  pinMode(sel_a, OUTPUT);      //analog mux selector 1
  pinMode(sel_b, OUTPUT);      //analog mux selector 2
  pinMode(sel_c, OUTPUT);      //analog mux selector 3
//  pinMode(sel_d, OUTPUT);      //analog mux selector 4
//  pinMode(midipin, INPUT);     //pin for standard MIDI input
  MIDI.begin();
}

void loop(){
  Edit.Read(Bs, Ps, FS1, FS2);

  if(Edit.state == false){
    DSP.Value(Edit.Tracks(Ts, FS1));
  }
  else{
    for(int i=0;i<5;i++){
      Bs[i]->Read();
    }
    FS1.Read();
  }
 
/*  UNCOMMENT WHEN THERE ARE MORE BUTTONS
    for(int i=5;i<sizeof(Bs)/sizeof(Bs[0]);i++){ 
    Bs[i]->Read();
  }
*/

  EXP.Read();
  FS2.Read();
  
/*  for (int i=0; i<8; i++){
    digitalWrite(sel_a, (i&7)>>2);
    digitalWrite(sel_b, (i&3)>>1);
    digitalWrite(sel_c, (i&1));
    Pots[i]->Read();
    Pots[i+8]->Read();
  }
*/

  while(usbMIDI.read()){};
  DSP.Info(Bs, Ts, Edit, FS1, FS2);
}

