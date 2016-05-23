#ifndef display_h
#define display_h

#include "Arduino.h"

class Display{
    unsigned long int timer;
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
    void states(bool a, bool b, bool c, bool d, bool e, bool f, bool g, bool h);
    void clear();
    void print();
};

#endif


