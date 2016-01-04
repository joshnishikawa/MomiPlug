#include "display.h"

// constructors
Display::Display(){};

Display::Display(int s, int c, int l){
  serialPin = s;
  clockPin = c;
  latchPin = l;
  pinMode(serialPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  timer = 0;
};

// destructor
Display::~Display(){};


word Display::words[] = {
  //BCCCF7777777BBBB
  0b0011000000000000, // digit 0 & FS 0
  0b0101000000000000, // digit 1 & FS 1
  0b0110000000000000, // digit 2 & 'Edit button' state
  0b0000000000000000  // button states
};

void Display::value(int v){
  byte temp = 0;
  byte d[] {
    //*ABCDEFG
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
    switch(v[i]){          //ABCDEFG
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

void Display::clear(){
  for (int i=0; i<3; i++){
    for (int j=4; j<11; j++){
      words[i] &= ~(1 << j);
    }
  }
};

void Display::states(bool a, bool b, bool c, bool d, bool e, bool f, bool g, bool h){
//SET BITS   State on  ?      YES = Set bit to 1.      NO = Set bit to 0.
  words[0] = a == true ?      words[0] | (1 << 11):    words[0] & ~(1 << 11);
  words[1] = b == true ?      words[1] | (1 << 11):    words[1] & ~(1 << 11);
  words[2] = c == true ?      words[2] | (1 << 11):    words[2] & ~(1 << 11);
  words[3] = d == true ?      words[3] | (1 << 15):    words[3] & ~(1 << 15);
  words[3] = e == true ?      words[3] | (1 << 3):     words[3] & ~(1 << 3);
  words[3] = f == true ?      words[3] | (1 << 2):     words[3] & ~(1 << 2);
  words[3] = g == true ?      words[3] | (1 << 1):     words[3] & ~(1 << 1);
  words[3] = h == true ?      words[3] | 1:            words[3] & ~1;
};

void Display::print(){
  static int counter = 0;
  if(counter == 4){ // Using a counter instead of a for loop allows each digit
    counter = 0;    // stay lit for an entire loop cycle increasing brightness (I hope).
  }
  digitalWrite(latchPin, LOW);
  shiftOut(serialPin, clockPin, MSBFIRST, byte(words[counter]>>8));// 8 MSBs of a word
  shiftOut(serialPin, clockPin, MSBFIRST, byte(words[counter]));   // 8 LSBs of a word
  digitalWrite(latchPin, HIGH);
  counter++;
  if (millis() - timer > 3000){
    clear();
  }
};

