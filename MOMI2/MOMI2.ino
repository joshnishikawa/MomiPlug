#include "MOMIPLUG.h"
MIDI_CREATE_DEFAULT_INSTANCE();
//#include <usb_keyboard.h> // if also using for keyboard input

// PIN ASSIGNMENTS ##################################################
//MIDI in is pin 0 by default
//MIDI out is pin 1 by default
const int enableFS = 2; //enable footswitches
const int enableEXP = 3;//enable expression pedal
const int sel_a = 4;    //selectors for analog and digital mux inputs
const int sel_b = 5;    //
const int sel_c = 6;    //
const int sel_d = 7;    //
const int led3 = 8;     //LED on center right
const int fs1led = 9;   //LED for FS1
const int led2 = 10;    //LED on upper right
const int fs0led = 11;  //LED for FS0
const int editPin = 12; //edit button (push switch on encoder)
const int led0 = 13;    //the onboard (orange) LED
const int expPin = 14;  //analog input for expression pedal
const int ringPin = 15; //footswitch 1
const int tipPin = 16;  //footswitch 0
const int but1pin = 17; //onboard buttons
const int but2pin = 18; //
const int but3pin = 19; //
const int muxPin1 = 20; //muxed analog input
const int audioVol = 21;//volume for 3.5mm jack
const int cap0pin = 22; //onboard Capacitive touch sensor
const int muxPin0 = 23; //muxed digital input
const int encPinA = 24; //pin A of encoder
const int encPinB = 25; //pin B of encoder
const int led1 = 26;    //LED on upper left

const int segG = 27;    //Pin 7
const int segD = 28;    //Pin 8
const int segF = 29;    //Pin 9
const int digit2 = 30;  //Pin 10
const int segB = 31;    //Pin 11
const int segA = 32;    //Pin 12
const int digit1 = 33;  //Pin 1
const int segE = 34;    //Pin 2
const int segC = 35;    //Pin 3
const int digit3 = 36;  //Pin 4
const int segDP= 37;    //Pin 5
const int digit4 = 38;  //Pin 6

const int analog = 39;  //2-pin female header for variable resistors
const int audioR = A21; //right headphone
const int audioL = A22; //left headphone

// DECLARATOINS #########################################################
byte MIDIchannel;
bool readMUX;
bool readMIDI;
bool readUSB;
bool trackMode;
bool pauseMUXread;// keeps MUX selectors stable until the selected
                  // pot is edited or non-MUXed input is triggered       
SevSeg DSP;
char DSPstring[5] = "    ";
int DP = 0;

Editor editor = Editor(encPinA, encPinB, editPin);
Track* Ts[3];
MIDIbutton* Bs[5];
MIDIpot* Ps[18];

// INITIALIZATION #######################################################
void setup(){
  DSP.Begin(COMMON_CATHODE, 4, digit1, digit2, digit3, digit4, segA, segB, segC, segD, segE, segF, segG, segDP);
  DSP.SetBrightness(100); //Set the display to 100% brightness level
  
//restoreDefaultSettings(); // UNCOMMENT ONLY TO RESTORE ALL DEFAULT SETTINGS!
                            // THEN IMMEDIATELY COMMENT OUT AND REUPLOAD!

  pauseMUXread = false;
  EEPROM.get(0, MIDIchannel);
  EEPROM.get(1, readMUX);
  EEPROM.get(2, readMIDI);
  EEPROM.get(3, readUSB);
  EEPROM.get(4, trackMode);
  EEPROM.get(5, editor.level);

  Ts[0] = new Track(but1pin, 105, eeprom_read_word(384));
  Ts[1] = new Track(but2pin, 106, eeprom_read_word(386));
  Ts[2] = new Track(but3pin, 107, eeprom_read_word(388));
  for (int i=0; i<3; i++){
    Ts[i]->level = EEPROM.read(i+6);
    Ts[i]->state = EEPROM.read(i+9);
  }

  Bs[0] = new MIDIbutton(but1pin,  EEPROM.read(128), EEPROM.read(129), eeprom_read_word(384));
  EEPROM.get(130, Bs[0]->outLo);
  EEPROM.get(131, Bs[0]->outHi);
  EEPROM.get(132, Bs[0]->state);
  Bs[1] = new MIDIbutton(but2pin,  EEPROM.read(133), EEPROM.read(134), eeprom_read_word(386));
  EEPROM.get(135, Bs[1]->outLo);
  EEPROM.get(136, Bs[1]->outHi);
  EEPROM.get(137, Bs[1]->state);
  Bs[2] = new MIDIbutton(but3pin,  EEPROM.read(138), EEPROM.read(139), eeprom_read_word(388));
  EEPROM.get(140, Bs[2]->outLo);
  EEPROM.get(141, Bs[2]->outHi);
  EEPROM.get(142, Bs[2]->state);
  Bs[3] = new MIDIbutton(ringPin,  EEPROM.read(143), EEPROM.read(144));
  EEPROM.get(145, Bs[3]->outLo);
  EEPROM.get(146, Bs[3]->outHi);
  EEPROM.get(147, Bs[3]->state);
  Bs[3]->read(); // Will crash if not updated immediately after wake.
  Bs[4] = new MIDIbutton(tipPin,  EEPROM.read(148), EEPROM.read(149));
  EEPROM.get(150, Bs[4]->outLo);
  EEPROM.get(151, Bs[4]->outHi);
  EEPROM.get(152, Bs[4]->state);
  Bs[4]->read(); // Will crash if not updated immediately after wake.

  Ps[0] = new MIDIpot(expPin, EEPROM.read(256), EEPROM.read(257), EEPROM.read(258), EEPROM.read(259));
  Ps[0]->inputRange(eeprom_read_word(512), eeprom_read_word(514));
  Ps[1] = new MIDIpot(analog, EEPROM.read(260), EEPROM.read(261), EEPROM.read(262), EEPROM.read(263));
  Ps[1]->inputRange(eeprom_read_word(516), eeprom_read_word(518));

  for(int i=2; i<18; i++){
    Ps[i] = new MIDIpot(muxPin1,14+i);  // CC 16~31 // TODO: selectable MIDInote, MIDIpot, MIDIbutton
  }

  pinMode(sel_a, OUTPUT);             //analog mux selector 1
  pinMode(sel_b, OUTPUT);             //analog mux selector 2
  pinMode(sel_c, OUTPUT);             //analog mux selector 3
  pinMode(sel_d, OUTPUT);             //analog mux selector 4 (if MUX16)
  pinMode(enableFS, INPUT_PULLUP);
  pinMode(enableEXP, INPUT_PULLUP);
  pinMode(fs0led, OUTPUT);
  pinMode(fs1led, OUTPUT);
  pinMode(led0, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOff(onNoteOff);
  MIDI.setHandleNoteOn(onNoteOn);
  MIDI.setHandleAfterTouchPoly(onPolyPressure);
  MIDI.setHandleControlChange(onControl);
  MIDI.setHandleProgramChange(onProgram);
  MIDI.setHandleAfterTouchChannel(onAfterTouch);
  MIDI.setHandlePitchBend(onPitchBend);
}

// PROGRAM ##############################################################
void loop(){
  editor.bounce->update();
  // On Button Release ##################################################
  if (editor.bounce->risingEdge()){
    strcpy(DSPstring, "    ");
    editor.encoder->write(0);
    if (editor.editing == true){
      if (editor.storageIndex == 0){
        editor.editing = false;
        pauseMUXread = false; // resume now that we're finished editing
      }
/*    else if(editor.storageIndex >= 384){
        editor.editCapSens(*editor.cTarget);
        editor.editing = false;
        pauseMUXread = false; // resume now that we're finished editing
      }*/
      else if(editor.storageIndex >= 256){
        editor.editPot(*editor.pTarget);
        editor.editing = false;
        pauseMUXread = false; // resume now that we're finished editing
      }
      else if(editor.storageIndex >= 128){
        editor.editButton(*editor.bTarget);
        editor.editing = false;
        pauseMUXread = false; // resume now that we're finished editing
      }
      else{
        editor.editing = false;
        pauseMUXread = false;
      }
    }
    else{
      trackMode = !trackMode;
      EEPROM.put(4, trackMode); Serial.println("trackmode");
      if(trackMode){
        strcpy(DSPstring, "trac");
      }
      else{
        strcpy(DSPstring, "ctrl");
      }
    }
  }
  // On Button Press ####################################################
  else if (editor.bounce->fallingEdge()){
    DSP.DisplayString((char*)"    ", 0);
  }
  // On Hold ############################################################
  else if (editor.bounce->read() == LOW){
    int newChannel = editor.editChannel();
    if (newChannel != MIDIchannel){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      MIDIchannel = newChannel;
      editor.storageIndex = 0;
      EEPROM.put(0, MIDIchannel); Serial.println("MIDIchannel");
    }
    if (Bs[0]->read() == Bs[0]->outHi){ //falling edge
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      editor.storageIndex = 0;
      readMUX = !readMUX;
      EEPROM.put(1, readMUX); Serial.println("readMUX");
    }
    if (Bs[1]->read() == Bs[1]->outHi){ //falling edge
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      editor.storageIndex = 0;
      readMIDI = !readMIDI;
      EEPROM.put(2, readMIDI); Serial.println("readMIDI");
    }
    if (Bs[2]->read() == Bs[2]->outHi){ //falling edge
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      editor.storageIndex = 0;
      readUSB = !readUSB;
      EEPROM.put(3, readUSB); Serial.println("readUSB");
    }
    if (digitalRead(enableFS)){
      for (uint8_t i=3;i<5;i++){
        if (Bs[i]->read() >= 0){
          pauseMUXread = false; // now that we're editing something else
          editor.editing = true;
          editor.storageIndex = i*5+128;
          //Button's index * number of bytes stored for each button + adress where button storage begins
          editor.bTarget = Bs[i];
        }
      }
    }
    if (digitalRead(enableEXP) && Ps[0]->read() >= 0){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      editor.storageIndex = 256; //Pot storage begins at EEPROM address 256
      editor.pTarget = Ps[0];
    }
    if (Ps[1]->read() >= 0){
      pauseMUXread = false; // now that we're editing something else
      editor.editing = true;
      editor.storageIndex = 260; //Header storage begins at EEPROM address 260
      editor.pTarget = Ps[1];
    }
    if (readMUX && !pauseMUXread){// Read analog mux
      for(int i=0; i<16; i++){
        digitalWrite(sel_a, (i&15)>>3);
        digitalWrite(sel_b, (i&7)>>2);
        digitalWrite(sel_c, (i&3)>>1);
        digitalWrite(sel_d, (i&1));
        if (Ps[i+2]->read() >= 0){
          editor.editing = true;
          editor.storageIndex = (i+2)*4+256;
          //Pot's index * number of bytes stored for each pot + adress where pot storage begins
          editor.pTarget = Ps[i+2];
          pauseMUXread = true; //until finished editing
          break;
        }
      }
    }
    sprintf(DSPstring, "%4d", MIDIchannel);
    digitalWrite(led1, readMUX);
    digitalWrite(led2, readMIDI);
    digitalWrite(led3, readUSB);
  }
  // In Track Mode ######################################################
  else if (trackMode == true){
    int incdec = editor.encoder->read();
    for(int i=0; i<3; i++){
      int newState = Ts[i]->send();
      if(newState >= 0){
        EEPROM.put(i+6, Ts[i]->state); Serial.println("Tr state");
        sprintf(DSPstring, "%4d", newState);
      }
      if (incdec){
        int newLevel = Ts[i]->vol(incdec);
        if(newLevel >= 0){
          EEPROM.put(i+9, newLevel); Serial.println("Tr level");
          sprintf(DSPstring, "%4d", newLevel);
        }
      }
    }
    if (incdec){
      editor.encoder->write(0);
    }
//    record(Bs[5]->bounce, Bs[6]->bounce, *Ts[]);
    DP = 15;
    digitalWrite(fs0led, Bs[0]->state);
    digitalWrite(fs1led, Bs[1]->state);
    digitalWrite(led1, Ts[0]->state);
    digitalWrite(led2, Ts[1]->state);
    digitalWrite(led3, Ts[2]->state);
  }
  // In Control Mode ####################################################
  else{
    int newVal = editor.send();
    if(newVal >= 0){
      EEPROM.put(5, editor.level); Serial.println("Edit level");
      sprintf(DSPstring, "%4d", editor.level);
      DSPstring[0] = 'r';
    }

    newVal = Bs[0]->send();            // Send MIDI for Onboard Buttons
    if (newVal >= 0){
      EEPROM.put(132, Bs[0]->state); Serial.println("Butn state");
      sprintf(DSPstring, "%4d", newVal);
      DSPstring[0] = 'b';
    }
    newVal = Bs[1]->send();
    if (newVal >= 0){
      EEPROM.put(137, Bs[1]->state); Serial.println("Butn state");
      sprintf(DSPstring, "%4d", newVal);
      DSPstring[0] = 'b';
    }
    newVal = Bs[2]->send();
    if (newVal >= 0){
      EEPROM.put(142, Bs[2]->state); Serial.println("Butn state");
      sprintf(DSPstring, "%4d", newVal);
      DSPstring[0] = 'b';
    }

    if (digitalRead(enableFS)){              // Send MIDI for Foot Switches
      newVal = Bs[3]->send();
      if (newVal >= 0){
        EEPROM.put(147, Bs[3]->state); Serial.println("FS0 state");
        sprintf(DSPstring, "%4d", newVal);
        DSPstring[0] = 'f';
      }
      newVal = Bs[4]->send();
      if (newVal >= 0){
        EEPROM.put(152, Bs[4]->state); Serial.println("FS1 state");
        sprintf(DSPstring, "%4d", newVal);
        DSPstring[0] = 'f';
      }
    }

    newVal = Ps[1]->send();
    if (newVal >= 0){
      sprintf(DSPstring, "%4d", newVal);      
      DSPstring[0] = 'h';
    }

    if (digitalRead(enableEXP)){             // Send MIDI for EXP Pedal
      newVal = Ps[0]->send();
      if (newVal >= 0){
        sprintf(DSPstring, "%4d", newVal);      
        DSPstring[0] = 'E';
      }
    }

    if (readMUX){                            // Send MIDI for Muxed input
      for(int i=0; i<16; i++){
        digitalWrite(sel_a, (i&15)>>3);
        digitalWrite(sel_b, (i&7)>>2);
        digitalWrite(sel_c, (i&3)>>1);
        digitalWrite(sel_d, (i&1));
        newVal = Ps[i+2]->send();
        if (newVal >= 0){
          sprintf(DSPstring, "%4d", newVal);
        }
      }
    }
    if (readMIDI){                           // MIDI to usbMIDI
      MIDI.read();    
    }
    if (readUSB){                            // read USB input
      // TODO: set this up
    }
//    digitalWrite(led0, chaos(cap0pin, 2000, 1500, 1400, 107, 20) + 1); // effective only if readMIDI == true
    // adding 1 to the returned value of -1 makes 0 which turns the LED off, otherwise on
    DP = 0;
    digitalWrite(led1, Bs[0]->state);
    digitalWrite(led2, Bs[1]->state);
    digitalWrite(led3, Bs[2]->state);
    digitalWrite(fs0led, Bs[3]->state);
    digitalWrite(fs1led, Bs[4]->state);
  }
  DSP.DisplayString(DSPstring, DP);          // Display most recently stored information
}

