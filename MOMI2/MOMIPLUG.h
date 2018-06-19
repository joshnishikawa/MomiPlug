//LIBRARIES THAT MUST BE INCLUDED FOR MOMIPLUG TO WORK==========================
//Bounce, Flicker and Encoder libraries are also required
#include "Arduino.h"
#include <USBHost_t36.h>
#include <MIDI.h>
#include <EEPROM.h>
#include "SevSeg.h"
#include "MIDIcontroller.h"
#include "MIDIinput.h"
#include "track.h"
#include "editor.h"

//DEFAULT SETTINGS==============================================================
uint8_t defaults[12]{         //Start at EEPROM address 0
  3,                          //default MIDIchannel 
  0, 0, 0,                    //readMUX, readMIDI, readUSB
  0,                          //Track mode
  0,                          //Onboard Encoder level
  0, 0, 0,                    //Track states
  0, 0, 0,                    //Track levels
};
uint8_t buttonDefaults[25]{   //Start at EEPROM address 128
  102, 1, 0, 127, 0,
  103, 1, 0, 127, 0,
  104, 1, 0, 127, 0,
  80, 1, 0, 127, 0,           //FS0 CC#, mode, min, max, state
  81, 0, 0, 127, 0,           //FS1 CC#, mode, min, max, state
};
uint8_t potDefaults[8]{       //Start at EEPROM address 256
  85, 1, 0, 127,              //EXP CC#, kill, min out, max out
	14, 0, 0, 127  	            //analog header CC#, kill, min out, max out
};

uint16_t buttonThresholds[5]{ //Start at EEPROM address 384
  2000, 2000, 2000, 0, 0,     //Button Thresholds
};

uint16_t potRanges[36]{       //Start at EEPROM address 512
  0, 1023,                    //EXP
  0, 1023,                    //header
                              //analog MUX input ranges
  0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023,
  0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023, 0, 1023
};

//FUNCTIONS USED FOR RESTORING DEFAULT SETTINGS=================================
void eeprom_put_word(int p_address, int p_value){
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);
  EEPROM.put(p_address, lowByte);
  EEPROM.put(p_address + 1, highByte);
}

unsigned int eeprom_read_word(int p_address){
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);
  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

unsigned int eeprom_get_word(int p_address){
  byte lowByte;
  EEPROM.get(p_address, lowByte);
  byte highByte;
  EEPROM.get(p_address + 1,highByte);
  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void restoreDefaultSettings(){
  for(uint8_t i=0;i<sizeof(defaults);i++){
    EEPROM.put(i, defaults[i]);
  }
  for(uint8_t i=0;i<sizeof(buttonDefaults);i++){
    EEPROM.put(i+128, buttonDefaults[i]);
  }
  for(uint8_t i=0;i<sizeof(potDefaults);i++){
    EEPROM.put(i+256, potDefaults[i]);
  }
  for(uint8_t i=0;i<sizeof(buttonThresholds)/2;i++){
  eeprom_put_word(i*2+384, buttonThresholds[i]);    //*2 for 2-byte values
  }
  for(uint8_t i=0;i<sizeof(potRanges)/2;i++){
    eeprom_put_word(i*2+512, potRanges[i]);  //*2 for 2-byte values
  }
}

