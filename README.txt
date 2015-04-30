TEENSY 3.1 PIN ASSIGNMENTS
             ______
        ____|      |____
  GND -|-GND|      | 5V |- VBUS on USB host module
 FS 1 -| 0  |______AGND-|- AGND (sleve and Mux16 GND)
 FS 2 -| 1        _3.3v-|- Voltage
SEL_A  |-2   AREF(_) 23 |- 1/4" analog return (ring)
SEL_B  |-3    A-1(_) 22 |
SEL_C  |-4    A-2(_) 21-|  Encoder Pin B
SEL_D  |-5           20-|  Encoder Pin A
 MIDI  |-6           19-| 'Edit' Button
 SRCK -| 7           18 |- Button 0
 RCLK -| 8           17 |- Button 1
  SER -| 9           16 |- Button 2
   SS  |-10          15 |- Button 3
 MOSI  |-11          14 |- Button 4
 MISO  |-12          13 |- SCK
       |________________|

  * '-' indicates which direction the wires run.



74HC595 (x2) PIN ASSIGNMENTS
             __ __
 Butn4 LED -| (_) |- 5V
 Butn3 LED -|     |- Butn5 LED
 Butn2 LED -|     |- SER from Teensy
 G Segment -|     |-
 F Segment -|  #1 |- RCLK
 E Segment -|     |- SRCLK
 D Segment -|     |- 5V
       GND -|_____|- SER to #2
             __ __
 B Segment -| (_) |- 5V
 A Segment -|     |- C Segment
DP Segment -|     |- SER from #1
 Cathode 1 -|     |-
 Cathode 2 -|  #2 |- RCLK
 Cathode 3 -|     |- SRCLK
 Butn1 LED -|     |- 5V
       GND -|_____|-

 * The 7 segments are split between
   the 2 chips so that a maximum of
   5 pins are HIGH at any given time.



7-SEGMENT LED PIN ASSIGNMENTS

      G  F  DP A  B
 _____|__|__|__|__|_
|O ___ O ___ O ___  |
| |   | |   | |   | |
| |___| |___| |___| |
| |   | |   | |   | |
| |___| |___| |___| |
|___________________|
   |  |  |  |  |  |
   E  1  2  C  D  3

 *It's wired up-side down so
  ignore the datasheet.



USB HOST MODULE PIN ASSIGNMENTS (from circuits@home)
          __________
         |          |
         |          |
       __|          |__
      |  |          |  |
      |  |__________|  |
      | _    5V -(_) _ |- (Be sure to break the VBUS jumper.)
  SS -|(_)          (_)|
MOSI -|(_)          (_)|
MISO -|(_)          (_)|
 SCK -|(_)          (_)|
      |(_)          (_)|
      |(_)          (_)|
      |(_)          (_)|
      |(_)          (_)|
3.3V -|(_)          (_)|- GND
      |(_)          (_)|- 3.3V
 GND -|(_)          (_)|
      |(_)          (_)|
      |________________|



5-PIN MIDI IN DIN PIN ASSIGNMENTS
_______________________
                       \
________________________\
      _____________      |
     /_____________\     |     ______________
    //      _2     \\    |    /     2    \   \__
   // 4_   (_)   _5 \\   |   / 5   o===  4\   \ \___
  ||  (_)       (_)  ||  |  |  o===     o==|        \
  ||  _           _  ||  |  |              |         |__
  || (_)         (_) ||  |  | o===       o=|         |_ \
  || 1             3 ||  |  | 3          1 |         | \ \
  ||                 ||  |  |              |     ___/   \ \
   \\       _       //   |   \      _     /   /_/       / /
    \\_____| |_____//    |    \____| |___/___/         / /
     \-------------/     |                            / / 
_________________________|                            \ \

Pin 1 - Not Connected
Pin 2 - Not Connected
Pin 3 - Not Connected
Pin 4 - Through a 220 Ohm resistor and into 6N138 Pin 2
Pin 5 - From 6N138 Pin 3 

 * A diode and 270 Ohm resistor are also needed.
   See schematic below.



MIDI IN SCHEMATIC (6N138)        3.3V
                                  |
                           __ __  |   270 Ohm
            220 Ohm      -|1(_)8|~'~~~\/\/\/\_
DIN Pin 4~~~\/\/\/\~~~_~~~|2   7|-            |
DIN Pin 5~~~~~~~~~~~~~^~~~|3   6|~~~~~~~~~~~~~'~~~MIDI
                      D  -|4___5|~~GND          (Teensy)
                      i
                      o
                      d
                      e
