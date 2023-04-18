#ifndef _SOC_H_
#define _SOC_H_

#include "DataLogging.h"
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "FreeRTOS_TEENSY4.h"
#include <fstream>
#include <iostream>

//Estimate of the milliAmp hour capacity of the bike
#define RATED_CAPCITY = 3000;

double integralOfCharge = 0; //Represents the total integral of the current
double lastSavedTime = 0; //Represents the last time the integral was added to
float initDOD = 0; //Represents the initial depth of Discharge of the bike (how much battery has been used)
float initSOC; // Represents the initial SOC of the bike (usually 100%)
int lastSavedTime = epochTime;



#endif //_SOC_H_