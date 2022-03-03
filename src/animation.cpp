#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;
int pos = 0;
// define the animation chars
byte botL[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11000,
  B11000,
  B11000
};

byte midL[] = {
  B00000,
  B00000,
  B11000,
  B11000,
  B11000,
  B00000,
  B00000,
  B00000
};

byte topL[] = {
  B11100,
  B11100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte topR[] = {
  B00011,
  B00011,
  B00011,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte midR[] = {
  B00000,
  B00000,
  B00000,
  B00011,
  B00011,
  B00011,
  B00000,
  B00000
};

byte botR[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00111,
  B00111
};

void initializeAnim(){// load special processing animation chars into the 1602 LCD
lcd.createChar(0,botL);
lcd.createChar(1,midL);
lcd.createChar(2,topL);
lcd.createChar(3,topR);
lcd.createChar(4,midR);
lcd.createChar(5,botR);
}
void printAnim(){// ensure position is correct before printing the char
if(pos > 5) pos = 0;// return to beginning if finished already
lcd.setCursor(15,0);
lcd.write(pos);// write the current animated frame in
++pos; // increment position in animationd
}