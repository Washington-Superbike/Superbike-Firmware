// TODO:
// 1. Remove oldData field from the printedData struct.
// 2. Add more macros for default vals that are repeated in Display.ino
// 3. Remove all the extra macros and predefined methods
// 4. Remove everything that corresponds to things removed from Display.ino

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
#define NUM_DATA 12

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

typedef struct PrintedDataStruct {
  int labelX, y, dataX;          // y is the same for data and label, but X isnt
  volatile float oldData;               // volatile for some printed data, not all
  PRINT_TYPE type;
  volatile float* currData;           // volatile for some printed data, not all
  int dataLen;
  char* labelPtr;                        // "labelPtr" is just the label itself. No String in C/.ino, so this is our best option.
  // (maybe add later) char* unitsPtr;
} PrintedData;

typedef struct ScreenInfo {
  int px;
  int py;
  int pz;
  bool recentlyChanged;
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

// setup and update methods
void drawMeasurementScreen(MeasurementScreenData msData);
void displayTask(MeasurementScreenData msData, Screen screen);

// supporting methods for Display.ino
void drawMeasurementScreen(MeasurementScreenData msData, Screen screen);
void setupMeasurementScreen();
void screenEraser(int scaler, int i);

/// Checks for Auxiliary voltage reading from pin 13
float aux_voltage_read();

#endif
