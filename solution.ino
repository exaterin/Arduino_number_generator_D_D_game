#include "funshield.h"

constexpr int leds[] = { led1_pin, led2_pin, led3_pin, led4_pin };
constexpr int ledsSize = sizeof(leds)/sizeof(leds[0]);
constexpr int buttons[] = { button1_pin, button2_pin, button3_pin };
constexpr int dices[] = {4, 6, 8, 10, 12, 20, 100};
constexpr int dicesSize = sizeof(dices)/sizeof(dices[0]);
constexpr byte d = 0b10100001;
constexpr byte empty = 0b11111111;
constexpr int displaySize = 4;
constexpr int period = 50;

enum Mode { NORMAL, CONFIGURATION };
Mode mode;

long unsigned ledMillis, startMillis, processMillis, buttonMillis;
int CurLed, CurDice, CurSegment, count, result;


struct Initialization {
  
  void initLeds() {
    for (int i = 0; i < displaySize; ++i) {
      pinMode(leds[i], OUTPUT);
      digitalWrite(leds[i], OFF);
    }
  }
  
  void initButtons() {
    for (int button : buttons) 
      pinMode(button, INPUT);
  }
};

struct Display{

  bool state, stateLast, ifReading, ifAnimating;
  
  char CurDisplay[4];
  byte glyph;
  
  void writeOnDisplay(char ch, byte pos) {
    
    if ( isDigit(ch) ) 
      glyph = digits[ch -'0']; 
      
    else if (ch =='d') 
      glyph = d;
      
    else 
      glyph = empty;
      
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, glyph);
    shiftOut(data_pin, clock_pin, MSBFIRST, 1 << pos);
    digitalWrite(latch_pin, HIGH);
    
    CurSegment = (CurSegment + 1) % 4;
  }
  
  void configurateDisplay(int counterNow, int typeNow) {
    
    CurDisplay[0] = '0' + counterNow;
    CurDisplay[1] = 'd';
    
    if (dices[typeNow]/10 != 0) {
      CurDisplay[2] = ((dices[typeNow]/10)%10)+'0';
      CurDisplay[3] = (dices[typeNow]%10)+'0';
    }
    
    else {
      CurDisplay[2] = (dices[typeNow]%10)+'0';
      CurDisplay[3] = ' ';
    }
  }


};

Display display = Display();

void createNumbers() {

  buttonMillis = millis();
  
  randomSeed(processMillis - buttonMillis);
  
  for (int i = 0; i < count; ++i) 
    result += random(1,dices[CurDice] + 1);
}

void generateNumbers() {
  display.ifAnimating = false;
  CurLed = 0;
  
  for (int i = 0; i < ledsSize; i++) 
    digitalWrite(leds[i], OFF);

  createNumbers();

  int k = 1000;
  int m = 1;
  for (int i = displaySize - 1; i >= 0; --i) {
    if (result/k != 0) {
      display.CurDisplay[i] = ((result % (10*m))/m) + '0';
      m *= 10;
    }
    else display.CurDisplay[i] = ' ';
    
    k = k/10;
  }
  result = 0;
}



struct Button{

  void setNormalMode() {
    mode = NORMAL;
    display.ifAnimating = true;
    
    for (int i = 0; i < displaySize; i++) 
      display.CurDisplay[i] = ' ';
    
    ledMillis = millis();
    processMillis = millis();
  }
  
  void incrementThrow() {
    
    if (mode == NORMAL) {
      mode = CONFIGURATION;
      count -= 1;
    }
    count = (count + 1) % 9;
    
    if (count == 0)
      count = 9;
    
    display.configurateDisplay(count, CurDice);
    
  }
  
  void changeDiceType() {
  
    if (mode == NORMAL) {
      mode = CONFIGURATION;
      CurDice -= 1;
    }
    
    mode = CONFIGURATION;
    CurDice = (CurDice + 1) % dicesSize;
    display.configurateDisplay(count, CurDice);
  }
  
  void processButtons(long unsigned startMillis){
    
      if (millis() - startMillis >= period) {
        
      if (display.ifReading != display.state) {
        display.state = display.ifReading;
        if (display.state) {
               
          if (!digitalRead(button1_pin) && !display.ifAnimating) 
              setNormalMode();
                      
          if (!digitalRead(button2_pin)) 
              incrementThrow();
                   
          if (!digitalRead(button3_pin)) 
              changeDiceType();
        }    
        else if (mode == NORMAL && display.ifAnimating)
            generateNumbers();
      }
    }
  }
};

  
void LedsAnimation() {

  unsigned long time;
  unsigned int pausa = 300;
  unsigned int cyclus = 1800;
  unsigned int numOfLeds = 4;

  time = millis();

  for (unsigned int i = 1; i < numOfLeds; ++i)    
    if (time % cyclus == i * pausa ){
        digitalWrite(leds[i-1], OFF);
        digitalWrite(leds[i], ON);
      }
      
  for(unsigned int i = numOfLeds - 1; i > 1; --i)
    if (time % cyclus == ((2*numOfLeds - 1) - i) * pausa) {
      digitalWrite(leds[i], OFF);
      digitalWrite(leds[i-1], ON);
    }

    if (time % cyclus == 0 ) {
    digitalWrite(leds[1], OFF);
    digitalWrite(leds[0], ON);
    }

}
  

Button button = Button();
Initialization initialization = Initialization();

void setup() {
  
  initialization.initLeds();
  initialization.initButtons();
  
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  
  result = -1;
  count = 1; 
  
  display.configurateDisplay(count, CurDice); 
}

void loop() {
  
  display.writeOnDisplay(display.CurDisplay[CurSegment], CurSegment);
  
  if (display.ifAnimating) 
    LedsAnimation();
    
  display.ifReading = !digitalRead(button1_pin) || !digitalRead(button2_pin) || !digitalRead(button3_pin);

  if (display.ifReading != display.stateLast) 
    startMillis = millis();

  button.processButtons(startMillis);
  
  display.stateLast = display.ifReading;
  
}
