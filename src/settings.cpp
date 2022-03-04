// this file contains all of the functions needed to change parameters for the unit and load/save then to/from eeprom
/*
* EEPROM Layout
* Location 0 -> int containing previous 
* Location 1 -> bool containing smoothing on/off
*
*
*/
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <buttonDebounce.h>
#include <EEPROM.h>
#include <string.h>
#include <globalHeader.h>
extern LiquidCrystal_I2C lcd;
double prevRecord[5];
void loadParameter(double *PPR){

}

const char * firingOrdersText[] = {"1P2R","1P1R","2P1R","3P1R","4P1R","6P1R","8P1R","3P2R","5P2R"}; // len = 9
const float firingOrdersFactor[] ={2,1,.5,1./3,.25,1./6,1./8,2./3,2./5};

void pulseParameter(){
    lcd.clear();
    lcd.home();
    lcd.print(" Firing Pattern");
    lcd.setCursor(4,1);
    lcd.print("Settings");
    delay(5000);
    lcd.clear();
    lcd.print("Mode: ");
    lcd.setCursor(0,1);
    lcd.print("Confirm -> Hold");// hold button to confirm
    
    for(int i = 0; i < 9; ++i){
            //lcd.setCursor(15,0);
            //lcd.print(i);
            //lcd.home();
            lcd.setCursor(6,0);
            lcd.print(firingOrdersText[i]);// e.g. 1P2R
            while(true){// wait for button to be pressed
            if(user.debounce()){// if the button is pressed
                unsigned long prevTime = millis();// record the start time
                while((millis() < prevTime + 100 ) && !digitalRead(BUTTON_PIN));// wait to see if time is either > 100ms held or was released
                delay(100);// debounce cannot be used here, so a delay is introduced
                if(!digitalRead(BUTTON_PIN)){// if it is still held, left the for loop due to time condition
                    lcd.setCursor(0,1);
                    lcd.print("Mode Saved");
                    delay(5000); // wait 1 second to show it's been saved
                    EEPROM.update(0,i);// save to EEPROM           
                    i = 10; // force out of outer for loop
                    break;
                }else break;// button was released first, therefore break from the while loop and return to outer for loop to select another value
            }
            if(i == 8) i = -1; // loop over options until selection is made
        }

    }
}

void saveState(bool smoothing){
    EEPROM.update(sizeof(int), smoothing);// move over the int into the next byte where our bool will be saved
}

void loadState(bool * smoothing, int * firingOrder){
    EEPROM.get(0, *firingOrder);
    EEPROM.get(sizeof(int), *smoothing);
}