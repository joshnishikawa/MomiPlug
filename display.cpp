#include "display.h"

// constructors
Display::Display(){};

Display::Display(int s, int c, int l){
  serial = s;
  clock = c;
  latch = l;
  pinMode(serial, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(latch, OUTPUT);
  timer = 0;
};

// destructor
Display::~Display(){};


word Display::words[] = { // Stores the info to be shifted out to display.
  // B = button LED, C = cathode, D = decimal, A = anode for one of the segments
  //BCCCDAAAAAAABBBB
  0b0011000000000000, // digit 0 & FS 0
  0b0101000000000000, // digit 1 & FS 1
  0b0110000000000000, // digit 2 & 'Edit button' state
  0b0000000000000000  // button states
};

void Display::value(int v){
  byte temp = 0;
  byte d[] {
    //ABCDEFG
    0b1111110, //0
    0b0110000, //1
    0b1101101, //2
    0b1111001, //3
    0b0110011, //4
    0b1011011, //5
    0b1011111, //6
    0b1110000, //7
    0b1111111, //8
    0b1111011, //9
    0b0000000, //' '
  };
  
  if (v >= 0){
    clear();
    words[2] += d[v%10]<<4;                              // write least significant digit
    v/=10;                                               // reduce to most significant digits
    temp = v%10;                                         // calculate new LSD
    v/=10;                                               // 2 or 3 digit number?
    words[0] += v==0 ? d[10]<<4 : d[v]<<4;               // if 2-dig, leave left digit blank
    words[1] += temp==0 && v==0 ? d[10]<<4 : d[temp]<<4; // if 1-dig, leave middle blank too
    timer = millis();
  }
};

void Display::verbal(const char v[]){
  clear();
  for(int i=0; i<3; i++){
    switch(v[i]){           //ABCDEFG
      case 'a': words[i] += 0b1110111<<4; break; //A
      case 'c': words[i] += 0b1001110<<4; break; //C
      case 'd': words[i] += 0b0111101<<4; break; //d
      case 'e': words[i] += 0b1001111<<4; break; //E
      case 'h': words[i] += 0b0010111<<4; break; //h
      case 'i': words[i] += 0b0110000<<4; break; //I
      case 'l': words[i] += 0b0001110<<4; break; //L
      case 'n': words[i] += 0b1110110<<4; break; //N
      case 'r': words[i] += 0b1100110<<4; break; //r
      case 's': words[i] += 0b1011011<<4; break; //S
      case 't': words[i] += 0b0001111<<4; break; //t
      case ' ': words[i] += 0b0000000<<4; break; //' '
      default : words[i] += 0b0000000<<4; break; 
    }
  }
  timer = millis();
};

void Display::states(bool a, bool b, bool c, bool d, bool e, bool f, bool g, bool h){
  bitWrite(words[0], 11, a);
  bitWrite(words[1], 11, b);
  bitWrite(words[2], 11, c);
  bitWrite(words[3], 15, d);
  bitWrite(words[3], 3, e);
  bitWrite(words[3], 2, f);
  bitWrite(words[3], 1, g);
  bitWrite(words[3], 0, h);
};

void Display::clear(){
  for (int i=0; i<3; i++){
    for (int j=4; j<11; j++){
      words[i] &= ~(1 << j);
    }
  }
};

void Display::print(){
  static int counter = 0; // Using a counter instead of a for loop allows
  if(counter == 4){       // each digit to stay lit for an entire loop cycle 
  counter = 0;            // which boosts the brightness a bit.
  }
  digitalWrite(latch, LOW);
  shiftOut(serial, clock, MSBFIRST, words[counter]>>8); // 8 MSBs of a word
  shiftOut(serial, clock, MSBFIRST, words[counter]);    // 8 LSBs of a word
  digitalWrite(latch, HIGH);
  counter++;
  if (millis() - timer > 3000){
    clear();
  }
};

