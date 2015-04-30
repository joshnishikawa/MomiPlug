#ifndef display_h
#define display_h

#include "Arduino.h"
#include "button.h"
#include "Editor.h"

/*You will almost certainly not want to bother
  with this file or its corresponding CPP.
  There are many ways to drive a 7-segment
  display. This my attempt to drive a 3-digit
  7-segment display and 5 LEDs without
  exceeding the maximum current draw of 2
  74HC595 shift registers. This is done by
  displaying one digit at a time with each
  chip responsible for half the segments of
  the digit and then displaying the LEDs.
  It works but requires the display, LEDs
  and shift registers to be wired in a very
  specific way (see README.txt) which makes
  the code useless anywhere else.
*/


class Display{
  public:
    Display();
    Display(int serial, int clock, int latch);
    ~Display();
    int serial;
    int clock;
    int latch;
    byte static digit[3];
    void Value(int i);
    void Word(char w[4]);
    void Clear();
    void Info(Button* Bs[], Track* Ts[],
              Editor& Edit, Button& FS1, Button& FS2);
};

#endif
