#pragma once
// global parameters
#define BUTTON_PIN 12

extern const char * firingOrdersText[];
extern const float firingOrdersFactor[];
extern LiquidCrystal_I2C lcd;
void printAnim();
void initializeAnim();
void pulseParameter();
void saveState(bool smoothing);
void loadState(bool * smoothing, int * firingOrder);

void startScreen();
extern Button user;
extern double pulsePerRev;
void printAnim();
void saveParameter(int PPR);
extern double prevRecord[5];