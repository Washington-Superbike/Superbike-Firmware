/**
   @file Display.h
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The Display.h config file for the Display task for the bike's firmware. This defines the variables that are passed along to all other files as
          pointers. Then it runs the setup methods for all those
          files and then it sets up RTOS to run all the different files
          as individual tasks. These tasks are: datalogging,
          display, precharge, CAN, idle. These tasks will be further
          described in the documentation for their individual files.


    \note
      up all members to be able to use it without any trouble.

    \todo
      Goal 1.
      \n \n
      Goal 2.
      \n \n
      Goal 3.
      \n \n
      Final Goal.
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "FreeRTOS_TEENSY4.h"

#define DISPLAY_TASK_STACK_SIZE configMINIMAL_STACK_SIZE + 32368

// maximum time in ms it takes for the display task to run, used for spi mutex elsewhere
#define DISPLAY_UPDATE_TIME_MAX 50

#define TFT_CS 10 //The display chip select pin
#define TFT_DC 9 // the display
#define TFT_RST 8 //the display reset pin


#define TS_CS  10
// MOSI=11, MISO=12, SCK=13

// The TIRQ interrupt signal must be used for this example.
#define TIRQ_PIN  -1

// Number of PrintedData values (length of the array that contains all the printedData)
#define NUM_DATA 11

// Default Values
#define DEFAULT_FLOAT -1
#define DEFAULT_X_POS 235

// Multipliers and Formula Constants:
#define VERTICAL_SCALER 16

#define TS_MINX  327
#define TS_MAXX  3903
#define TS_MINY  243
#define TS_MAXY  3842

typedef enum {
  NUMBER, ARRAY, BOOL
} PRINT_TYPE;

typedef enum {
  DEBUG, SPEEDOMETER
} SCREEN_TYPE;

typedef struct PrintedDataStruct {
  int labelX, y, dataX;          // y is the same for data and label, but X isnt
  volatile float oldData;               // volatile for some printed data, not all
  PRINT_TYPE type;
  volatile float* currData;           // volatile for some printed data, not all
  int dataLen;
  char* labelPtr;                        // "labelPtr" is just the label itself. No String in C/.ino, so this is our best option.
  // (maybe add later) char* unitsPtr;
} PrintedData;

typedef struct PrintedDataThermStruct {
  int labelX, y, dataX;          // y is the same for data and label, but X isnt
  volatile float oldData[10];               // volatile for some printed data, not all
  volatile float* currData;           // volatile for some printed data, not all
  char* labelPtr;                        // "labelPtr" is just the label itself. No String in C/.ino, so this is our best option.
} PrintedDataTherm;

typedef struct PrintedDataTimeStuct {
  int labelX, y, dataX;          // y is the same for data and label, but X isnt
  volatile float oldData[10];               // volatile for some printed data, not all
  volatile float* currData;           // volatile for some printed data, not all
  char* labelPtr;                        // "labelPtr" is just the label itself. No String in C/.ino, so this is our best option.
} PrintedDataTimeStuct;


typedef struct ScreenInfo {
  SCREEN_TYPE screenType;
} Screen;

typedef struct MeasurementScreenDataStruct {
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
} MeasurementScreenData;

typedef struct displayTaskWrapper {
  MeasurementScreenData* msDataWrap;
  Screen* screenDataWrap;
} displayPointer;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); //the display controller
// All PrintedValue objects/structs
PrintedData printedVals[NUM_DATA];

PrintedData *batteryVoltage = &printedVals[0];
PrintedData *motorControllerVoltage = &printedVals[1];
PrintedData *auxBatteryVoltage = &printedVals[2];
PrintedData *rpm = &printedVals[3];
PrintedData *motorTemperature = &printedVals[4];
PrintedData *motorCurr = &printedVals[5];
PrintedData *errMessage = &printedVals[6];
PrintedData *chargerVolt = &printedVals[7];
PrintedData *chargerCurr = &printedVals[8];
PrintedData *bmsStatusFlag = &printedVals[9];
PrintedData *evccVolt = &printedVals[10];

PrintedDataTherm thermiData;
PrintedDataTimeStuct timeData;

// setup and update methods
void setupDisplay(MeasurementScreenData msData, Screen screen);
void displayUpdate(MeasurementScreenData msData, Screen screen);
void thermiDataPrint(bool thermiDataPrint);
void timePrint();
void setupMeasurementScreen(Screen screen);
void eraseThenPrint(int xPos, int yPos, String oldData, String newData);
void eraseThenPrintSPEEDO(int xPos, int yPos, String oldData, String newData);
//void screenEraser(int scaler, int i);
void manualScreenDataUpdater();
float aux_voltage_read();

#endif
