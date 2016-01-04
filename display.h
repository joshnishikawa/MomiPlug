#ifndef display_h
#define display_h

#include "Arduino.h"

class Display{
  public:
    Display();
    Display(int serial, int clock, int latch);
    ~Display();
    int timer;
    int serialPin;
    int clockPin;
    int latchPin;
    word static words[4];
    void value(int i);
    void verbal(const char v[]);
    void states(bool a, bool b, bool c, bool d, bool e, bool f, bool g, bool h);
    void clear();
    void print();
};

#endif


