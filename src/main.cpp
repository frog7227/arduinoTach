// dependencies
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include <buttonDebounce.h>
#include <globalHeader.h>

// functions in other files

#define SMOOTH_RECORD 10 // number of values to smooth

// 2 volatiles used to track overflow of the counters
volatile bool ovrflw = false;
volatile int currPos = 0;
LiquidCrystal_I2C lcd(0x27,16,2); // define I2C LCD
Button user;
double pulsePerRev = 2;
double RPM = 0, prevRPM[SMOOTH_RECORD];
bool smooth = false;
int overflows = 50, smoothPos = 0, firingOrderNum = 0;
/*
int centerPos(int strLen){
  strLen = (16-strLen)/2; 
  if(strLen > 0) return strLen;
  else return -1;
}
*/
void stopTimers(){
  noInterrupts();// pause interrupts so settings can be changed during calculation
  TCCR1B = 0; //stop timer 1 and 2
  TCCR2B = 0;
  interrupts();
}
void startTimers(){
  noInterrupts();
  TCNT1 = 0; // clear timer params
  currPos = 0;
  TCNT2 = 0; 
  // renable counters
  TCCR1B |= (1 << CS12) + (1 << CS11) +  (1 << CS10); // CS12,11,10 = 1, sets counter source to external rising edge on T1 pin, aka D5 on arduino uno
  TCCR2B |= (1 << CS22) + (1 << CS21) + (1 << CS20); // set clock prescaler to /1024 
  interrupts(); // allow ISRs to run
}
void calculateRPM(){// calculate RPM
  stopTimers();
  
  unsigned int tCount1 = TCNT1;
  unsigned int tCount2 = TCNT2;

  if(!ovrflw){// account for possible overflow condition
    RPM = 937500.*pulsePerRev*(double)tCount1/((double)currPos*256. + (double)tCount2);// refer to notes for how 937500 is calculated
    //RPM = tCount1;
  }else{
  RPM = -1;
  ovrflw = false;
  } 
  // reset everything to zero for next aquisition
  startTimers();
}

void startScreen(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Curr RPM:       ");
  lcd.setCursor(0,1);
  lcd.print("S: ");
  if(smooth) lcd.print("On    M: ");
  else lcd.print("Off   M: ");
  lcd.print(firingOrdersText[firingOrderNum]);
  initializeAnim();
}
double averageRPM(){
    double sum = 0;
    for(int i = 0; i < SMOOTH_RECORD; ++i) sum += prevRPM[i];
    return sum/SMOOTH_RECORD;
}
void updateScreen(double RPM1){
  lcd.setCursor(10,0);
  lcd.print("     ");
  lcd.setCursor(10,0);
  if(RPM1 >= 0 || RPM1 < 100000) lcd.print(RPM1);
  else lcd.print("OVFLW");
  printAnim();
}

void setup() {
  //Serial.begin(115200);
  user.begin(BUTTON_PIN);
  //configure LCD and add loading message
  lcd.init();
  lcd.clear();
  lcd.backlight();
  // loading text
  lcd.setCursor(2,0);
  lcd.print("Initializing");
  lcd.setCursor(0,1);
  lcd.print("(c) Andrew O '22");
  delay(2000);
  if(!digitalRead(BUTTON_PIN)) pulseParameter();
  loadState(&smooth, &firingOrderNum);
  pulsePerRev = firingOrdersFactor[firingOrderNum];
  startScreen();
  pinMode(5,INPUT);
noInterrupts(); //disable interrupts while timer settings are changed

//configure timer 1 (max of 65536)  for tach use
TCCR1A = 0; // clear all settings 
TCCR1B = 0;
TCNT1  = 0;
TIMSK1 = 0;
TIMSK2 = 0;

TCCR1B |= (1 << CS12) + (1 << CS11) +  (1 << CS10); // CS12,11,10 = 1, sets counter source to external rising edge on T1 pin, aka D5 on arduino uno
TCCR1B |= (1 << ICNC1); // enable noise filter
TIFR1 |= (1 << TOV1); // clear overflow flag, sometimes it gets set at startup
TIMSK1 |= (1 << TOIE1); // enable overflow ISR

// timer 2 controls the overall timing of counting, this is due to it not having an external interrupt input, so it cannot be used for actual counting
TCCR2A = 0; // clear all settings
TCCR2B = 0;
TCNT2  = 0;

TCCR2B |= (1 << CS22) + (1 << CS21) + (1 << CS20); // set clock prescaler to /1024
TIMSK2 |= (1 << TOIE2); // enable overflow ISR

interrupts(); // renable interrupts after settings are changed
}

void loop() {
  if(user.debounce()){ // if user wants to enable smoothing
    if(!smooth){// if smoothing is off currently
        stopTimers();
        smoothPos = 0;
        for(int i = 0; i < SMOOTH_RECORD; ++i) prevRPM[i] = RPM;//set all of the array to last rpm value
        smooth = true;
        saveState(smooth);// save while it cannot be interrupted
        lcd.setCursor(3,1);
        lcd.print("On ");
        startTimers();
      }else{// if smoothing is on
        stopTimers();
        smooth = false;
        saveState(smooth);// save while it cannot be interrupted
        lcd.setCursor(3,1);
        lcd.print("Off");
        startTimers();
      }
      
  }

if(currPos >= overflows){// time to calculate rpm and update screen
  calculateRPM();
  if(smooth){
    prevRPM[smoothPos] = RPM;
    updateScreen(averageRPM());
    if(++smoothPos > SMOOTH_RECORD - 1) smoothPos = 0;
  }else updateScreen(RPM);
}


}
ISR(TIMER2_OVF_vect)        // interrupt service routine for timer overflow
{
    currPos++; // increment overflows, this counts within +-.064ms which is within one 1 rpm for < 10k rpm
}
ISR(TIMER1_OVF_vect)        // interrupt service routine for timer overflow
{
    ovrflw = true;
}