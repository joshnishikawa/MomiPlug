//LIBRARIES THAT MUST BE INCLUDED FOR MOMIPLUG TO WORK
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

/*TEENSY 3.6 PIN ASSIGNMENTS
                 ______
            ____|      |____
      GND -|-GND|      | 5V |
  MIDI in  |-0  |______AGND-|-
 MIDI out  |-1 (_)     3.3v-|
Enable FS  |-2 (_)    (_)23 |- Digital MUX return
Enable EXP |-3 (_)    (_)22 |- Touch Expression
 Select A  |-4 (_)    (_)21 |- Audio Volume
 Select B  |-5 (_)       20 |- Analog MUX return
 Select C  |-6           19 |- Touch Button 2
 Select D  |-7           18 |- Touch Button 1
    LED 3 -| 8           17 |- Touch Button 0
  LED FS1 -| 9           16 |- FS 1 (tip)
    LED 2 -| 10          15 |- FS 0 (ring)
  LED FS0 -| 11          14 |- EXP return (ring)
     EDIT -| 12          13 |  LED 0 
          -| 3.3v       GND-|-
    Enc A -| 24         A22-|  Phones L
    Enc B -| 25         A21-|  Phones R
    LED 1 -| 26          39 |- Analog Header return
Segment G  |-27          38-|  Cathode 4
Segment D  |-28          37-|  Segment DP
Segment F  |-29          36-|  Cathode 3
Cathode 2  |-30          35-|  Segment C
Segment B  |-31          34-|  Segment E
Segment A  |-32          33-|  Cathode 1
           |________________|
dashes indicate which direction wires run
                           _
            ______________| |______________
MUX PINOUT /     o (_)    |_|     o  (_)  /|
          /////           o o    o  (_)  / |
         /______________________________/  |
        |   __     __                   | _|_
        |  /  \   /  \   _______________||___)
        |  \__/   \__/  |. . . . . . . .| /<-----Analog Mux
        |_______________|. . . . . . . .|/ <-----Digital Mux (Cap Touch)
                         G D 3 3 S S S S
                         N A V V E E E E
                         D T     L L L L 
                           A     | | | |
                                 A B C D 
                             _________________________   
                            |      _____________                                 
         ______________     |     /_____________\   
      __/   /    2     \    |    //      _2     \\  
   __/ /   /5  ===0    4\   |   // 4_   (_)   _5 \\ 
  /       |==0     ===0  |  |  ||  (_)       (_)  ||
_|        |              |  |  ||  _           _  ||
_         |=0       ===0 |  |  || (_)         (_) ||
 |        | 3          1 |  |  || 1             3 ||
  \__     |              |  |  ||                 ||
     \_\   \      _     /   |   \\       _       // 
        \___\____| |___/    |    \\_____| |_____//  
                            |     \-------------/   
                            |__________________________

5-PIN MIDI IN DIN PIN ASSIGNMENTS
  Pin 1 - Not Connected
  Pin 2 - Not Connected
  Pin 3 - Not Connected
  Pin 4 - Through a 220 Ohm resistor and into PC900 Pin 1
  Pin 5 - From PC900 Pin 2 (a diode is also needed)

  SCHEMATIC                          3.3V
                                      |
              220 Ohm        __ __    |   270 Ohm
  DIN Pin 4~~~\/\/\/\~~~_~~~|1(_)6|~~~'~~~\/\/\/\_
  DIN Pin 5~~~~~~~~~~~~~^~~~|2   5|~~~GND         |
                           -|3___4|~~~~~~~~~~~~~~~'~~~ to Teensy Pin 0
                             PC900                        (MIDI in)
                          Opto-Coupler              

5-PIN MIDI OUT DIN PIN ASSIGNMENTS
  Pin 1 - Not Connected
  Pin 2 - to Ground
  Pin 3 - Not Connected
  Pin 4 - Connected to 5.5v through a 220 Ohm resistor
  Pin 5 - Connected to Teensy Pin 1 (MIDI out) through a 220 Ohm resistor


USB HOST PORT PIN ASSIGNMENTS

   5V D- D+ GND
   |  |  |  |
  ____________
 | U  U  U  U |
 |____________|


TODO: Find a way to choose whether muxed input should be buttons, pots or drums.
*/
