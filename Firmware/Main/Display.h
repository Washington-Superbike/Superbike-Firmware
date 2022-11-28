
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "FreeRTOS_TEENSY4.h"

#define DISPLAY_TASK_STACK_SIZE configMINIMAL_STACK_SIZE + 8096

// maximum time in ms it takes for the display task to run, used for spi mutex elsewhere
#define DISPLAY_UPDATE_TIME_MAX 50

#define TFT_CS 10 //The display chip select pin
#define TFT_DC 9 // the display
#define TFT_RST 8 //the display reset pin


#define TS_CS  10
// MOSI=11, MISO=12, SCK=13

// The TIRQ interrupt signal must be used for this example.
#define TIRQ_PIN  -1


#define TS_MINX  327
#define TS_MAXX  3903
#define TS_MINY  243
#define TS_MAXY  3842

typedef struct {
  int px;
  int py;
  int pz;
  bool recentlyChanged;
} Screen;

typedef struct {
  float* mainBatteryVoltage;
  float* motorControllerVoltage;
  float* auxiliaryBatteryVoltage;
  float* RPM;
  float* motorTemp;
  float* motorCurrent;
  int* errorMessage;
  float* chargerCurrent;
  float* chargerVoltage;
  int* bmsStatusFlag;
  float* evccVoltage;
  float* thermistorTemps;
  float prevMainBattVoltage;
  float prevAuxBattVoltage;
  int prevRPM;
  float prevMotorTemp;
  float prevMotorCurrent;
  Screen screen;
} MeasurementScreenData;

void drawMeasurementScreen(MeasurementScreenData msData);
void displayTask(MeasurementScreenData msData);

#endif //_DISPLAY_H
