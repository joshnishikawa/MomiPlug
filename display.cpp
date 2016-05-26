#include "display.h"

// constructors
Display::Display(){};

Display::Display(const int s, const int c, const int l){
  serial = s;
  clock = c;
  latch = l;
  pinMode(serial, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(latch, OUTPUT);
  timer = 0;
  blinkTimer = 0;
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
      case 'A': words[i] += 0b1110111<<4; break; //A
      case 'a': words[i] += 0b1110111<<4; break; //A
      case 'b': words[i] += 0b0011111<<4; break; //b
      case 'C': words[i] += 0b1001110<<4; break; //C
      case 'c': words[i] += 0b0001101<<4; break; //c
      case 'd': words[i] += 0b0111101<<4; break; //d
      case 'E': words[i] += 0b1001111<<4; break; //E
      case 'e': words[i] += 0b1001111<<4; break; //E
      case 'F': words[i] += 0b1000111<<4; break; //E
      case 'f': words[i] += 0b1000111<<4; break; //E
      case 'H': words[i] += 0b0110111<<4; break; //H
      case 'h': words[i] += 0b0010111<<4; break; //h
      case 'I': words[i] += 0b0110000<<4; break; //I
      case 'i': words[i] += 0b0010000<<4; break; //i
      case 'J': words[i] += 0b0111100<<4; break; //J
      case 'j': words[i] += 0b0111100<<4; break; //J
      case 'L': words[i] += 0b0001110<<4; break; //L
      case 'l': words[i] += 0b0110000<<4; break; //l
      case 'N': words[i] += 0b1110110<<4; break; //N
      case 'n': words[i] += 0b0010101<<4; break; //n
      case 'O': words[i] += 0b1111110<<4; break; //O
      case 'o': words[i] += 0b0011101<<4; break; //o
      case 'P': words[i] += 0b1100111<<4; break; //P
      case 'p': words[i] += 0b1100111<<4; break; //P
      case 'q': words[i] += 0b1110011<<4; break; //q
      case 'r': words[i] += 0b0000101<<4; break; //r
      case 'S': words[i] += 0b1011011<<4; break; //S
      case 's': words[i] += 0b1011011<<4; break; //S
      case 't': words[i] += 0b0001111<<4; break; //t
      case 'U': words[i] += 0b0111110<<4; break; //U
      case 'u': words[i] += 0b0011100<<4; break; //u
      case 'Y': words[i] += 0b0111011<<4; break; //Y
      case 'y': words[i] += 0b0111011<<4; break; //y
      case '-': words[i] += 0b0000001<<4; break; //-
      case '_': words[i] += 0b0001000<<4; break; //_
      case '[': words[i] += 0b1000010<<4; break; //[
      case ']': words[i] += 0b0011000<<4; break; //]
      case ' ': words[i] += 0b0000000<<4; break; //' '
      case '0': words[i] += 0b1111110<<4; break; //0
      case '1': words[i] += 0b0110000<<4; break; //1
      case '2': words[i] += 0b1101101<<4; break; //2
      case '3': words[i] += 0b1111001<<4; break; //3
      case '4': words[i] += 0b0110011<<4; break; //4
      case '5': words[i] += 0b1011011<<4; break; //5
      case '6': words[i] += 0b1011111<<4; break; //6
      case '7': words[i] += 0b1110000<<4; break; //7
      case '8': words[i] += 0b1111111<<4; break; //8
      case '9': words[i] += 0b1111011<<4; break; //9
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

void Display::blink(int i){
  static bool toggle = false;
  if (millis() - blinkTimer > 200){
    toggle = !toggle;
    bitWrite(words[i], 11, toggle);
    blinkTimer = millis();
  }
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
    counter = 0;          // which boosts the brightness a bit.
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

