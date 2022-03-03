// dependencies
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
//#include <customChars.h>
//#include <EEPROM.h>
#include "Arduino.h"
#include<HardwareSerial.h>
#include <buttonDebounce.h>

// functions in other files
extern void printAnim();
extern void initializeAnim();
extern void loadParameter();
extern void pulseParameter();
extern void sampleTimeParameter();

LiquidCrystal_I2C lcd(0x27,16,2); // define I2C LCD
Button user;

// 2 volatiles used to track overflow of the counters
volatile bool ovrflw = false;
volatile int currPos = 0;
double pulsePerRev=2; // called r on the page
double RPM = 0;
bool smooth = false;
int overflows = 50;
/*
int centerPos(int strLen){
  strLen = (16-strLen)/2; 
  if(strLen > 0) return strLen;
  else return -1;
}
*/

void calculateRPM(){
 // lcd.setCursor(0,1);
 // lcd.print(TCCR1B);
  noInterrupts();// pause interrupts so settings can be changed during calculation
  TCCR1B = 0; //stop timer 1 and 2
  TCCR2B = 0;
  
  // calculate RPM
  unsigned int tCount1 = TCNT1;
  unsigned int tCount2 = TCNT2;
  int oldPos = currPos;
  if(!ovrflw){
    RPM = 937500.*pulsePerRev*(double)tCount1/((double)currPos*256. + (double)tCount2);// refer to notes for how 937500 is calculated
    //RPM = tCount1;
  }else{
  RPM = -1;
  ovrflw = false;
  } 
  // reset everything to zero for next aquisition
  TCNT1 = 0;
  
  currPos = 0;
  TCNT2 = 0;
    
  // renable counters
  TCCR1B |= (1 << CS12) + (1 << CS11) +  (1 << CS10); // CS12,11,10 = 1, sets counter source to external rising edge on T1 pin, aka D5 on arduino uno
  TCCR2B |= (1 << CS22) + (1 << CS21) + (1 << CS20); // set clock prescaler to /1024
   
interrupts(); // allow ISRs to run
/*
lcd.setCursor(0,1);
lcd.print("        ");
lcd.setCursor(0,1);
lcd.print(tCount1);
lcd.print("|");
lcd.print(tCount2+256*oldPos);
*/
}

void startScreen(){
lcd.clear();
lcd.setCursor(0,0);
  lcd.print("Curr RPM:       ");
  lcd.setCursor(0,1);
  lcd.print("S: Off   M: 1P1R");
  initializeAnim();
}

void updateScreen(){
lcd.setCursor(10,0);
lcd.print("     ");
lcd.setCursor(10,0);
if(RPM >= 0) lcd.print(RPM);
else lcd.print("OVFLW");
printAnim();
}

void setup() {
  Serial.begin(115200);
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
TIFR1 |= (1 << TOV1); // clear overflow flag
TIMSK1 |= (1 << TOIE1); // enable overflow ISR
// timer 2 controls the overall timing of counting, this is due to it not having an external interrupt input, so it cannot be used for actual counting
TCCR2A = 0; // clear all settings
TCCR2B = 0;
TCNT2  = 0;
TCCR2B |= (1 << CS22) + (1 << CS21) + (1 << CS20); // set clock prescaler to /1024
TIMSK2 |= (1 << TOIE2); // enable overflow ISR
interrupts(); // renable interrupts after settings are changed
user.begin(12);
}

void loop() {
if(currPos >= overflows){ 
  calculateRPM();
  updateScreen();
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