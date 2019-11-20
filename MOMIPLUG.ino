#include "MOMIPLUG.h"

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
const int enableFS = 2; //enable footswitches
const int enableEXP = 3;//enable expression pedal
const int sel_a = 4;    //selectors for analog and digital mux inputs
const int sel_b = 55;   //55 because pin 5 got fried. Oops!
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
byte MIDIchannel = 3;
uint8_t readMIDIthru = true;
uint8_t readAnalogMUX = false;
uint8_t readDigitalMUX = false;
uint8_t trackMode = false;
SevSeg DSP;
char DSPstring[5] = "    ";
int DP = 0;
Editor editor = Editor(encPinA, encPinB, editPin);
Track* Ts[3];
MIDIbutton* Bs[23];
MIDIpot* Ps[19];
MIDItouch blackSpongeTouch(23, 9, TOUCH); // TESTING: DELETE ME

Bounce FSenable(enableFS, 500);
Bounce EXPenable(enableEXP, 500);
TouchVariable paperClip(cap0pin, 0, 127);

void setup(){ // INITIALIZATION #########################################
  MIDIchannel = EEPROM.read(255);
  readMIDIthru = EEPROM.read(1);
  readAnalogMUX = EEPROM.read(2);
  readDigitalMUX = EEPROM.read(3);
  DSP.Begin(COMMON_CATHODE, 4, digit1, digit2, digit3, digit4,
                               segA, segB, segC, segD, segE, segF, segG, segDP);
  DSP.SetBrightness(100); //Set the display to 100% brightness level
  
  Ts[0] = new Track(but1pin, 105);
  Ts[1] = new Track(but2pin, 106);
  Ts[2] = new Track(but3pin, 107);

  Bs[0] = new MIDIbutton(but1pin, 102, 1, TOUCH);
  Bs[1] = new MIDIbutton(but2pin, 103, 1, TOUCH);
  Bs[2] = new MIDIbutton(but3pin, 104, 1, TOUCH);
  Bs[3] = new MIDIbutton(ringPin, 80, 0);
  EEPROM.get(4, Bs[3]->mode);
  Bs[4] = new MIDIbutton(tipPin, 81, 0);
  EEPROM.get(8, Bs[4]->mode);
  Bs[5] = new MIDIbutton(ringPin, 107, 0);
  Bs[6] = new MIDIbutton(tipPin, 107, 0); //DELETE ME MAYBE???
  for(int i=7; i<23; i++){Bs[i] = new MIDIbutton(muxPin0,9+i, 1);} // CC 16~31

  Ps[0] = new MIDIpot(expPin, 85);
  EEPROM.get(12, Ps[0]->mode);
  Ps[0]->inputRange(8, 1015); // just cuz my expression pedal ain't great 
  Ps[1] = new MIDIpot(analog, 14);
  EEPROM.get(16, Ps[1]->inLo);
  EEPROM.get(20, Ps[1]->inHi);
  for(int i=2; i<18; i++){Ps[i] = new MIDIpot(muxPin1,14+i);} // CC 16~31

  Ps[18] = new MIDIpot(audioVol, 90, 15, 85); // Until I have something better for this pin
  Ps[18]->inputRange(300, 700); // TESTING: DELETE ME
  Ps[18]->outputRange(70, 18); // TESTING: DELETE ME
  Ps[18]->setControlNumber(9); // TESTING: DELETE ME
  blackSpongeTouch.setControlNumber(90); // TESTING: DELETE ME
  blackSpongeTouch.setOutputRange(22, 70); // TESTING: DELETE ME
  blackSpongeTouch.inputRange(); // TESTING: DELETE ME

  paperClip.setInputRange();

  pinMode(sel_a, OUTPUT); //analog and digital mux selector 1
  pinMode(sel_b, OUTPUT); //analog and digital mux selector 2
  pinMode(sel_c, OUTPUT); //analog and digital mux selector 3
  pinMode(sel_d, OUTPUT); //analog and digital mux selector 4
  pinMode(enableFS, INPUT);
  pinMode(enableEXP, INPUT);
  pinMode(fs0led, OUTPUT);
  pinMode(fs1led, OUTPUT);
  pinMode(led0, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

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
/*  EEPROM.put(255, 3); EEPROM.put(1, true);  // MIDIchannel, readMIDIthru
  EEPROM.put(2, false); EEPROM.put(3, false); // readAnalogMUX, readDigitalMux
  EEPROM.put(4, 0); EEPROM.put(8, 1);         // FS0 mode, FS1 mode
  EEPROM.put(12, 0);                          // EXP killSwitch #
  EEPROM.put(16, 0); EEPROM.put(20, 1023);    // Header min, max 
  */
}

void loop(){ // PROGRAM #################################################
  editor.bounce->update();
  FSenable.update();
  EXPenable.update();

  // On Button Release ##################################################
  if (editor.bounce->risingEdge()){
    strcpy(DSPstring, "    ");
    editor.encoder->write(0);
    paperClip.setInputRange();

    switch (editor.editing){
      case 255: EEPROM.put(255, MIDIchannel); break;
      case 1: EEPROM.put(1, readMIDIthru); break;
      case 2: EEPROM.put(2, readAnalogMUX); break;
      case 3: EEPROM.put(3, readDigitalMUX); break;
      case 4: EEPROM.put(4, Bs[3]->mode); break;
      case 8: EEPROM.put(8, Bs[4]->mode); break;
      case 12:EEPROM.put(12, Ps[0]->killSwitch); break;
      case 16: 
        Ps[1]->inLo = editor.newInLo;
        Ps[1]->inHi = editor.newInHi;
        EEPROM.put(16, Ps[1]->inLo);
        EEPROM.put(20, Ps[1]->inHi);
        break;
      default:
        trackMode = !trackMode;
    }
    editor.editing = 0;

    if(trackMode){
      for(int i=0; i<3; i++){
        usbMIDI.sendControlChange(Ts[i]->number, Ts[i]->level, MIDIchannel);
      }
      strcpy(DSPstring, "trac");
    }
    else{
      if(readAnalogMUX){
        for(int i=2; i<18; i++){
          usbMIDI.sendControlChange(Ps[i]->number, Ps[i]->value, MIDIchannel);          
        }
      }
      strcpy(DSPstring, "ctrl");
    }
  }

  // On Button Press ####################################################
  else if (editor.bounce->fallingEdge()){ //calibrate the analog header
    editor.newInLo = analogRead(analog);
    editor.newInHi = editor.newInLo;
    sprintf(DSPstring, "%4d", MIDIchannel);
  }

  // On Hold ############################################################
  else if (editor.bounce->read() == LOW){
    int newChannel = editor.editChannel();
    if (newChannel != MIDIchannel){
      editor.editing = 255;
      MIDIchannel = newChannel;
      sprintf(DSPstring, "%4d", MIDIchannel);
    }
    if (Bs[0]->read() == 127){
      editor.editing = 1;
      readMIDIthru = !readMIDIthru;
    }
    if (Bs[1]->read() == 127){
      editor.editing = 2;
      readAnalogMUX = !readAnalogMUX;
    }
    if (Bs[2]->read() == 127){
      editor.editing = 3;
      readDigitalMUX = !readDigitalMUX;
    }
    if (FSenable.read()){
      if (Bs[3]->read() == 127){
        editor.editing = 4;
        Bs[3]->mode = Bs[3]->mode == 1 ? 0 : 1;
        if(Bs[3]->mode == 1){strcpy(DSPstring, "lach");}
        else {strcpy(DSPstring, "hold");}
      }
      if (Bs[4]->read() == 127){
        editor.editing = 8;
        Bs[4]->mode = Bs[4]->mode == 1 ? 0 : 1;
        if(Bs[4]->mode == 1){strcpy(DSPstring, "lach");}
        else {strcpy(DSPstring, "hold");}
      }
    }
    if (EXPenable.read() && Ps[0]->read() == 127){
      editor.editing = 12;
      Ps[0]->killSwitch = !Ps[0]->killSwitch;
      if(Ps[0]->killSwitch){strcpy(DSPstring, " cut");}
      else{strcpy(DSPstring, "-cut");}
    }
    
    if (editor.setAnalog(analog)){
      editor.editing = 16;
      strcpy(DSPstring, "setH");
    }

    digitalWrite(led1, readMIDIthru);
    digitalWrite(led2, readAnalogMUX);
    digitalWrite(led3, readDigitalMUX);
  }
  
  else{
  // In Track Mode Or Control Mode ######################################
    int newVal = 0;
    if (EXPenable.read()){
      newVal = Ps[0]->send();              // Send MIDI for EXP Pedal
      if (newVal >= 0){
        sprintf(DSPstring, "%4d", newVal);      
        DSPstring[0] = 'E';
      }
    }
    newVal = Ps[1]->send();              // Send MIDI for Analog Header
    if (newVal >= 0){
      sprintf(DSPstring, "%4d", newVal);      
      DSPstring[0] = 'h';
    }
    Ps[18]->send(); // TESTING: DELETE ME
    
    if (readMIDIthru){
      teensyUSBHost.Task();
      USBMIDI.read();
      MIDI.read();
      int newValue = paperClip.read();
      chaos(newValue); // TODO: Other cool MIDI functions
      digitalWrite(led0, newValue);
    }
    else{ // Ignore (don't buffer) incoming MIDI.
      while(MIDI1.read()){}
      // I wish there were a way to ignore USB host MIDI.
    }

    //blackSpongeTouch.send(); // TESTING: DELETE ME
    
    if (readAnalogMUX || readDigitalMUX){
    for(int i=0; i<8; i++){               // USE i<8 FOR MUX8, i<16 FOR MUX16
//        digitalWrite(sel_d, (i&15)>>3); // COMMENT FOR MUX8, UN- FOR MUX16
        digitalWrite(sel_c, (i&7)>>2);
        digitalWrite(sel_b, (i&3)>>1);
        digitalWrite(sel_a, (i&1));

        if (readAnalogMUX){
          newVal = Ps[i+2]->send();
          if (newVal >= 0){
            sprintf(DSPstring, "%4d", newVal);
            DSPstring[0] = 'A';
          }
        }
        if (readDigitalMUX){
          newVal = Bs[i+7]->send();
          if (newVal >= 0){
            sprintf(DSPstring, "%4d", newVal);
            DSPstring[0] = 'd';
          }
        }
      }
    }  
    // In Track Mode ######################################################
    if (trackMode){
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
      uint8_t recState = record(Bs[5]->read() == 127, Bs[6]->read() == 127);
      DP = 15;
      digitalWrite(fs0led, recState);
      digitalWrite(fs1led, !digitalRead(tipPin));
      digitalWrite(led1, Ts[0]->state);
      digitalWrite(led2, Ts[1]->state);
      digitalWrite(led3, Ts[2]->state);
    }

    // In Control Mode ####################################################
    else{
      newVal = editor.send();
      if(newVal >= 0){
        sprintf(DSPstring, "%4d", editor.level);
        DSPstring[0] = 'r';
      }
      for(int i=0;i<3;i++){ // Send MIDI for Onboard Buttons
        newVal = Bs[i]->send();
        if (newVal >= 0){
          sprintf(DSPstring, "%4d", newVal);
          DSPstring[0] = 'b';
        }
      }
      if (FSenable.read()){
        for(int i=3;i<5;i++){ // Send MIDI for Foot Switches
          newVal = Bs[i]->send();
          if (newVal >= 0){
            sprintf(DSPstring, "%4d", newVal);
            DSPstring[0] = 'f';
          }
        }
      }
      
      DP = 0;
      digitalWrite(led1, Bs[0]->state);
      digitalWrite(led2, Bs[1]->state);
      digitalWrite(led3, Bs[2]->state);
      digitalWrite(fs0led, Bs[3]->state);
      digitalWrite(fs1led, Bs[4]->state);
    }
  }
  DSP.DisplayString(DSPstring, DP); // Display most recently stored information
}
