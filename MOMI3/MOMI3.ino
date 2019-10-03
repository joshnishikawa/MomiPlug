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
bool readMIDIthru = true;
bool readAnalogMUX = false;
bool readDigitalMUX = false;
bool trackMode = false;
SevSeg DSP;
char DSPstring[5] = "    ";
int DP = 0;
Editor editor = Editor(encPinA, encPinB, editPin);
Track* Ts[3];
MIDIbutton* Bs[23];
MIDIpot* Ps[18];

// INITIALIZATION #######################################################
void setup(){
  Serial.begin(115200); //is this really necessary?
  delay(1500);
  MIDIchannel = EEPROM.read(0);
  readMIDIthru = EEPROM.read(1);
  readAnalogMUX = EEPROM.read(2);
  readDigitalMUX = EEPROM.read(3);
  DSP.Begin(COMMON_CATHODE, 4, digit1, digit2, digit3, digit4,
                               segA, segB, segC, segD, segE, segF, segG, segDP);
  DSP.SetBrightness(100); //Set the display to 100% brightness level
  
  Ts[0] = new Track(but1pin, 105, 2100);
  Ts[1] = new Track(but2pin, 106, 2100);
  Ts[2] = new Track(but3pin, 107, 2100);

  Bs[0] = new MIDIbutton(but1pin,  102, 1, 2100);
  Bs[1] = new MIDIbutton(but2pin,  103, 1, 2100);
  Bs[2] = new MIDIbutton(but3pin,  104, 1, 2100);
  Bs[3] = new MIDIbutton(ringPin,  80, EEPROM.read(4));
  Bs[4] = new MIDIbutton(tipPin,  81, EEPROM.read(8));
  Bs[5] = new MIDIbutton(ringPin, 107, 0);
  Bs[6] = new MIDIbutton(tipPin, 107, 0); //DELETE ME MAYBE???
  for(int i=7; i<23; i++){Bs[i] = new MIDIbutton(muxPin0,9+i, 1);} // CC 16~31

  Ps[0] = new MIDIpot(expPin, 85, EEPROM.read(12));
  Ps[0]->inputRange(8, 1015); // just cuz my expression pedal ain't great 
  Ps[1] = new MIDIpot(analog, 14);
  EEPROM.get(16, Ps[1]->inLo);
  EEPROM.get(20, Ps[1]->inHi);
  EEPROM.get(24, editor.touchLo);
  EEPROM.get(28, editor.touchHi);
  for(int i=2; i<18; i++){Ps[i] = new MIDIpot(muxPin1,14+i);} // CC 16~31

  pinMode(sel_a, OUTPUT);             //analog and digital mux selector 1
  pinMode(sel_b, OUTPUT);             //analog and digital mux selector 2
  pinMode(sel_c, OUTPUT);             //analog and digital mux selector 3
  pinMode(sel_d, OUTPUT);             //analog and digital mux selector 4
  pinMode(enableFS, INPUT_PULLUP);
  pinMode(enableEXP, INPUT_PULLUP);
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
  /*EEPROM.put(0, 3); EEPROM.put(1, true);
  EEPROM.put(2, false); EEPROM.put(3, false);
  EEPROM.put(4, 0); EEPROM.put(8, 1);
  EEPROM.put(12, 0); EEPROM.put(16, 0); EEPROM.put(20, 1023);*/
}

// PROGRAM ##############################################################
void loop(){
  editor.bounce->update();

  // On Button Release ##################################################
  if (editor.bounce->risingEdge()){
    strcpy(DSPstring, "    ");
    editor.encoder->write(0);
    if (editor.editing == true){
      if(editor.target == 0){EEPROM.put(0, MIDIchannel);}
      else if(editor.target == 1){EEPROM.put(1, readMIDIthru);}
      else if(editor.target == 2){EEPROM.put(2, readAnalogMUX);}
      else if(editor.target == 3){EEPROM.put(3, readDigitalMUX);}
      else if(editor.target == 4){EEPROM.put(4, Bs[3]->mode);}
      else if(editor.target == 8){EEPROM.put(8, Bs[4]->mode);}
      else if(editor.target == 12){EEPROM.put(12, Ps[0]->mode);}
      else if(editor.target == 16){
        Ps[1]->inLo = editor.newInLo;
        Ps[1]->inHi = editor.newInHi;
        EEPROM.put(16, Ps[1]->inLo);
        EEPROM.put(20, Ps[1]->inHi);
      }
      else if(editor.target == 24){
        EEPROM.put(24, editor.touchLo);
        EEPROM.put(28, editor.touchHi);
      }
      editor.editing = false;
    }
    else{
      trackMode = !trackMode;
      if(trackMode){strcpy(DSPstring, "trac");}
      else{strcpy(DSPstring, "ctrl");}
    }
  }

  // On Button Press ####################################################
  else if (editor.bounce->fallingEdge()){ //calibrate the analog header
    editor.newInLo = analogRead(analog);
    editor.newInHi = editor.newInLo;
    editor.touchLo = touchRead(cap0pin);
    editor.touchHi = editor.touchLo;
    sprintf(DSPstring, "%4d", MIDIchannel);
  }

  // On Hold ############################################################
  // TODO: 'editing' and 'target' could be reduced to 1 variable inside the sketch
  else if (editor.bounce->read() == LOW){
    int newChannel = editor.editChannel();
    if (newChannel != MIDIchannel){
      editor.editing = true;
      editor.target = 0;
      MIDIchannel = newChannel;
      sprintf(DSPstring, "%4d", MIDIchannel);
    }
    if (Bs[0]->read() == 127){
      editor.editing = true;
      editor.target = 1;
      readMIDIthru = !readMIDIthru;
    }
    if (Bs[1]->read() == 127){
      editor.editing = true;
      editor.target = 2;
      readAnalogMUX = !readAnalogMUX;
    }
    if (Bs[2]->read() == 127){
      editor.editing = true;
      editor.target = 3;
      readDigitalMUX = !readDigitalMUX;
    }
    if (digitalRead(enableFS)){ // TODO: use delay to prevent MIDI when hot-plugging
      if (Bs[3]->read() == 127){
        editor.editing = true;
        editor.target = 4;
        Bs[3]->mode = Bs[3]->mode == 1 ? 0 : 1;
        if(Bs[3]->mode == 1){strcpy(DSPstring, "lach");}
        else{strcpy(DSPstring, "hold");}
      }
      if (Bs[4]->read() == 127){
        editor.editing = true;
        editor.target = 8;
        Bs[4]->mode = Bs[4]->mode == 1 ? 0 : 1;
        if(Bs[4]->mode == 1){strcpy(DSPstring, "lach");}
        else{strcpy(DSPstring, "hold");}
      }
    }
    if (digitalRead(enableEXP) && Ps[0]->read() == 127){ // TODO: use delay to prevent MIDI when hot-plugging
      editor.editing = true;
      editor.target = 12;
      Ps[0]->mode = !Ps[0]->mode;
      if(Ps[0]->mode){strcpy(DSPstring, " cut");}
      else{strcpy(DSPstring, "-cut");}
    }
    
    if (editor.setAnalog(analog)){
      editor.editing = true;
      editor.target = 16;
      strcpy(DSPstring, "setH");
    }

    if (editor.setTouch(cap0pin)){
      editor.editing = true;
      editor.target = 24;
      strcpy(DSPstring, "setT");
    }

    digitalWrite(led1, readMIDIthru);
    digitalWrite(led2, readAnalogMUX);
    digitalWrite(led3, readDigitalMUX);
  }
  
  else{
  // In Track Mode Or Control Mode ######################################
    int newVal = 0;
    if (digitalRead(3) == HIGH){
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

    if (readMIDIthru){
      teensyUSBHost.Task();
      USBMIDI.read();
      MIDI.read();
      digitalWrite(led0, chaos(cap0pin, editor.touchLo, editor.touchHi*.8, editor.touchHi, 24, 96) + 1);
      // adding 1 to the returned value of -1 makes 0 which turns the LED off, otherwise on
    }

    if (readAnalogMUX || readDigitalMUX){
    for(int i=0; i<8; i++){               // USE i<16 FOR MUX16
//        digitalWrite(sel_d, (i&15)>>3); // UNCOMMENT FOR MUX16
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
      bool recState = record(Bs[5]->read() == 127, Bs[6]->read() == 127);
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
      for(int i=0;i<3;i++){                // Send MIDI for Onboard Buttons
        newVal = Bs[i]->send();
        if (newVal >= 0){
          sprintf(DSPstring, "%4d", newVal);
          DSPstring[0] = 'b';
        }
      }
      if (digitalRead(2) == HIGH){
        for(int i=3;i<5;i++){                // Send MIDI for Foot Switches
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

