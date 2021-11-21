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
             ___|      |___
            |GND|      | 5V|
    MIDI in |0  |______AGND|
   MIDI out |1 (_)     3.3v|
       Edit |2 (_)    (_)23| Touch Button 5
SCL2/CAN0TX |3 (_)    (_)22| Touch Button 4 
SDA2/CAN0RX |4 (_)    (_)21| MUX1 return
    FS1 LED |5 (_)       20| MUX0 return
      LED 3 |6           19| Touch Button 3
      LED 2 |7           18| Touch Button 2
      LED 1 |8           17| Touch Button 1
    FS0 LED |9           16| MIC
       SEL0 |10          15| FS 2 (ring)
 SEL1/MOSI0 |11          14| SEL3/SCK0 ( use SPI.setSCK(14) )
 SEL2/MISO0 |12          13| LED_BUILTIN 
            |3.3v       GND|
      Enc B |24         A22| Phones R (ring)
      Enc A |25         A21| Phones L (tip)
 FS 1 (tip) |26          39| EXP return (ring)
  Segment G |27          38| Cathode 4
  Segment D |28          37| Segment DP
  Segment F |29          36| Cathode 3
  Cathode 2 |30          35| Segment C
  Segment B |31          34| Segment E
  Segment A |32          33| Cathode 1
            |______________|

HEADER PIN ASSIGNMENTS
 __________________________________________________
|                         |                        |
|  SIG0  3.3V  GND    EN  | SIG1  3.3V  GND    EN  |
|                         |                        |
|  SEL3  SEL2  SEL1  SEL0 | SEL3  SEL2  SEL1  SEL0 |
|_________________________|________________________|

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


USB HOST PORT PIN ASSIGNMENTS

   5V D- D+ GND
   |  |  |  |
  ____________
 | U  U  U  U |
 |____________|


TODO: Find a way to choose whether muxed input should be buttons, pots or drums.
*/
