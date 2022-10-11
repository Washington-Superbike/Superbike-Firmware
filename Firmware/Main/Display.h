
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

#define TFT_CS 1  // The display chip select pin
#define TFT_DC 3  // the display
#define TFT_RST 2 // the display reset pin

#define TS_CS 9
// MOSI=11, MISO=12, SCK=13

// The TIRQ interrupt signal must be used for this example.
#define TIRQ_PIN -1

// Number of PrintedData values (length of the array that contains all the printedData)
#define NUM_DATA 13

// Default Values
#define DEFAULT_FLOAT -1
#define DEFAULT_X_POS 235

#define TS_MINX 327
#define TS_MAXX 3903
#define TS_MINY 243
#define TS_MAXY 3842

typedef enum
{
  NUMBER,
  ARRAY,
  BOOL
} PRINT_TYPE;

typedef struct PrintedDataStruct
{
  int labelX, y, dataX;   // y is the same for data and label, but X isnt
  volatile float oldData; // volatile for some printed data, not all
  PRINT_TYPE type;
  volatile float *currData; // volatile for some printed data, not all
  int dataLen;
  char *labelPtr; // "labelPtr" is just the label itself. No String in C/.ino, so this is our best option.
                  // (maybe add later) char* unitsPtr;
} PrintedData;

typedef struct MeasurementScreenDataStruct
{
  float *mainBatteryVoltage;
  float *motorControllerVoltage;
  float *auxiliaryBatteryVoltage;
  float *RPM;
  float *motorTemp;
  float *motorCurrent;
  int *errorMessage;
  float *chargerCurrent;
  float *chargerVoltage;
  int *bmsStatusFlag;
  float *evccVoltage;
  float *thermistorTemps;
  // long *potentialLost; // represents the amount of data that is yet to be saved on the SD card (the potential amount of data lost
} MeasurementScreenData;

typedef struct ScreenInfo
{
  int px;
  int py;
  int pz;
  bool recentlyChanged;
} Screen;

void drawMeasurementScreen(MeasurementScreenData msData);
void displayTask(MeasurementScreenData msData);

// supporting methods for Display.ino
void drawMeasurementScreen(MeasurementScreenData msData, Screen screen);
void setupMeasurementScreen();
void screenEraser(int scaler, int i);

#endif //_DISPLAY_H
