#ifndef display_h
#define display_h

#include "Arduino.h"

class Display{
    unsigned long int timer;
    unsigned long int blinkTimer;
    int serial;
    int clock;
    int latch;
    word static words[4];
  public:
    Display();
    Display(const int serial, const int clock, const int latch);
    ~Display();
    void value(int i);
    void verbal(const char v[]);
    void states(bool d, bool e, bool f, bool g, bool h);
    void states(bool a, bool b, bool c, bool d, bool e, bool f, bool g, bool h);
    void blink(int i);
    void clear();
    void print();
};

#endif


