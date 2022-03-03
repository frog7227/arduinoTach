// this file contains all of the functions needed to change parameters for the unit and load/save then to/from eeprom
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <buttonDebounce.h>
extern LiquidCrystal_I2C lcd;
extern void startScreen();
extern Button user;
extern double pulsePerRev;

void loadParameter(){

}

void pulseParameter(){
    lcd.clear();
    lcd.home();
    lcd.print("1P1R");
    if(user.debounce()){
        delay(100);
        if(user.debounce()){
            pulsePerRev=2;
        }
        
    }
}
void sampleTimeParameter(){
    
}
