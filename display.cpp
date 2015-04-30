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
};


// destructor
Display::~Display(){};

byte Display::digit[3];

// Loads 'digit' with up to a 3-digit integer.
// (Aligned right with no leading zeroes.)
void Display::Value(int v){
    byte d[] {
      //*ABCDEFG
      0b11111110, //0
      0b10110000, //1
      0b11101101, //2
      0b11111001, //3
      0b10110011, //4
      0b11011011, //5
      0b11011111, //6
      0b11110000, //7
      0b11111111, //8
      0b11111011, //9
      0b10000000, //' '
    };
    digit[2] = d[v%10];
    v/=10;
    digit[1] = v%10;
    v/=10;
    digit[0] = v==0 ? d[10] : d[v];
    digit[1] = digit[1]==0 && v==0 ? d[10] : d[digit[1]];
};


// Loads digit with up to 3 of these supported characters.
// (Aligned left. Use leading spaces if needed.)
void Display::Word(char w[]){
  for(int i=0; i<3; i++){
    switch(w[i]){          //*ABCDEFG
      case 'a': digit[i] = 0b11110111; break; //A
      case 'c': digit[i] = 0b11001110; break; //C
      case 'd': digit[i] = 0b10111101; break; //d
      case 'e': digit[i] = 0b11001111; break; //E
      case 'h': digit[i] = 0b10010111; break; //h
      case 'i': digit[i] = 0b10110000; break; //I
      case 'l': digit[i] = 0b10001110; break; //L
      case 'n': digit[i] = 0b11110110; break; //N
      case 'r': digit[i] = 0b11100110; break; //r
      case 's': digit[i] = 0b11011011; break; //S
      case 't': digit[i] = 0b10001111; break; //t
      case ' ': digit[i] = 0b10000000; break; //' '
      default : digit[i] = 0b10000000; break; 
    }
  }
};


void Display::Clear(){
  digit[0] = 0b10000000;
  digit[1] = 0b10000000;
  digit[2] = 0b10000000;
};


/*This rather long function is an attempt to drive a 7-segment display and
5 LEDs without exceeding the maximum current draw of the 74HC595 chips.
This is done by displaying one digit at a time with each chip responsible
for half the segments of the digit and then displaying the LEDs.*/
void Display::Info(Button* Bs[], Track* Ts[],
                   Editor& Edit, Button& FS1, Button& FS2){
  int pinState;
  digitalWrite(latch, LOW);

  // DISPLAY FIRST DIGIT
  digitalWrite(serial, LOW);        //button 1 LED off
  digitalWrite(clock, HIGH);

  for(int i=0;i<3;i++){
    digitalWrite(clock, LOW);
    if(i==0){
      digitalWrite(serial, LOW);
      digitalWrite(clock, HIGH);
    }
    else{
      digitalWrite(serial, HIGH);
      digitalWrite(clock, HIGH);
    }
  }
  digitalWrite(clock, LOW);
  digitalWrite(serial, Edit.state);  // the 'Edit' Button state
  digitalWrite(clock, HIGH);
  for(int i=6;i>=0;i--){             //digit 1 segments
    digitalWrite(clock, LOW);
    pinState = digit[0] & (1<<i) ? 1 : 0;
    digitalWrite(serial, pinState);
    digitalWrite(clock, HIGH);
  }
  for(int i=0;i<4;i++){              //all other button LEDs off
    digitalWrite(clock, LOW);
    digitalWrite(serial, LOW);
    digitalWrite(clock, HIGH);
  }
  digitalWrite(clock, LOW);
  digitalWrite(latch, HIGH);

  // DISPLAY SECOND DIGIT
  digitalWrite(latch, LOW);

  digitalWrite(clock, LOW);
  digitalWrite(serial, LOW);        //button 1 LED off
  digitalWrite(clock, HIGH);

  for(int i=0;i<3;i++){
    digitalWrite(clock, LOW);
    if(i==1){
      digitalWrite(serial, LOW);
      digitalWrite(clock, HIGH);
    }
    else{
      digitalWrite(serial, HIGH);
      digitalWrite(clock, HIGH);
    }
  }
  digitalWrite(clock, LOW);
  digitalWrite(serial, FS1.state);   //FS1 state
  digitalWrite(clock, HIGH);
  for(int i=6;i>=0;i--){             //digit 2 segments
    digitalWrite(clock, LOW);
    pinState = digit[1] & (1<<i) ? 1 : 0;
    digitalWrite(serial, pinState);
    digitalWrite(clock, HIGH);
  }
  for(int i=0;i<4;i++){              //all other button LEDs off
    digitalWrite(clock, LOW);
    digitalWrite(serial, LOW);
    digitalWrite(clock, HIGH);
  }
  digitalWrite(clock, LOW);
  digitalWrite(latch, HIGH);

  // DISPLAY THIRD DIGIT
  digitalWrite(latch, LOW);

  digitalWrite(clock, LOW);
  digitalWrite(serial, LOW);         //button 1 LED off
  digitalWrite(clock, HIGH);

  for(int i=0;i<3;i++){
    digitalWrite(clock, LOW);
    if(i==2){
      digitalWrite(serial, LOW);
      digitalWrite(clock, HIGH);
    }
    else{
      digitalWrite(serial, HIGH);
      digitalWrite(clock, HIGH);
    }
  }
  digitalWrite(clock, LOW);
  digitalWrite(serial, FS2.state);    //FS2 state
  digitalWrite(clock, HIGH);
  for(int i=6;i>=0;i--){              //digit 3 segments
    digitalWrite(clock, LOW);
    pinState = digit[2] & (1<<i) ? 1 : 0;
    digitalWrite(serial, pinState);
    digitalWrite(clock, HIGH);
  }
  for(int i=0;i<4;i++){               //all other button LEDs off
    digitalWrite(clock, LOW);
    digitalWrite(serial, LOW);
    digitalWrite(clock, HIGH);
  }
  digitalWrite(clock, LOW);
  digitalWrite(latch, HIGH);

  //DISPLAY BUTTON LEDS
  digitalWrite(latch, LOW);
  if(Edit.state == false){              //button 1 LED
    digitalWrite(serial, Ts[0]->state);}
  else digitalWrite(serial, Bs[0]->state);
  digitalWrite(clock, HIGH);
  
  digitalWrite(serial, LOW);
  for(int i=0;i<11;i++){
    digitalWrite(clock, LOW);
    digitalWrite(clock, HIGH);
  }
  for(int i=1;i<5;i++){
    digitalWrite(clock, LOW);
    if(Edit.state == false){              //other button LEDs
      digitalWrite(serial, Ts[i]->state);}
    else digitalWrite(serial, Bs[i]->state);
      digitalWrite(clock, HIGH);
    }
  digitalWrite(clock, LOW);
  digitalWrite(latch, HIGH);
};
