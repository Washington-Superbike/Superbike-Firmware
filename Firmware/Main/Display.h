/**
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#include "CAN.h"

/// Uses the configMINIMAL_STACK_SIZE variable in Main.h to add up to the stack size used for the displayTask
#define DISPLAY_TASK_STACK_SIZE 10000
/* max time in ms it takes for display to update, should periodically measure this */
#define DISPLAY_UPDATE_TIME_MAX 50

/// The Teensy pin used for the display's chip select pin
#define TFT_CS 10
/// The Teensy pin used for the display data/command pin
#define TFT_DC 9
/// The Teensy pin used for the display reset pin
#define TFT_RST 8

/// The Teensy pin used for the display touch-screen chip select.
/// This is useless because we dont use the touch screen.
#define TS_CS  10
// MOSI=11, MISO=12, SCK=13

/// The TIRQ interrupt signal must be used for this example.
#define TIRQ_PIN  -1

/// Number of PrintedData values (length of the array that contains all the printedData)
#define NUM_DATA 12

// Default Values:
/// The default starter value used for floats in the PrintedDataStruct
#define DEFAULT_FLOAT -1
/// The default x position used the data in the PrintedDataStruct
#define DEFAULT_X_POS 235

// Multipliers and Formula Constants:
/// A Scaler for moving vertical data down. Each row of data is a factor of
/// 16 pixels away from each other, vertically.
#define VERTICAL_SCALER 16

/// Sets a default erasing(background)/writing(print) color based on screen type
#ifdef USE_DEBUGGING_SCREEN
#  define BACKGROUND_COLOR ILI9341_WHITE
#  define PRINT_COLOR ILI9341_BLACK
#else // (if speedometer screen)
#  define BACKGROUND_COLOR ILI9341_BLACK
#  define PRINT_COLOR ILI9341_WHITE
#  define RED ILI9341_RED
#  define GREEN ILI9341_GREEN
#endif

// Gear ratio, 48 teeth in the back wheel sprocoket. 16 on motor sprocket
// Diameter = 0.522 m, divided by 60 converts it into per second, so the RPM is converted to a final
// Speed of m/s
#define GEAR_RATIO (48.0 / 16.0)
#define DIAMETER 0.522
#define MPH_CONVERT 2.2369362920544

// Touch screen parameters.
/// Not sure why this is so high. Google it, but as of now it's unused.
#define TS_MINX  327
/// Not sure why this is so high. Google it, but as of now it's unused.
#define TS_MAXX  3903
/// Not sure why this is so high. Google it, but as of now it's unused.
#define TS_MINY  243
/// Not sure why this is so high. Google it, but as of now it's unused.
#define TS_MAXY  3842

/**
 * Just a basic enum to store the type of data being printed.
 */
typedef enum {
  NUMBER, ARRAY, BOOL
} PRINT_TYPE;

/**
 * The first complex struct. Stores all the info of a specific type of data we're
 * storing. This includes the x and y position of the label, the x and y position
 * of the data, the data type, a pointer to the current data, a float to store the
 * old data, the length of the data (TODO REMOVE), and a char* (String) with the
 * label for that data. Please look at the struct itself and how it is implemented
 * to fully understand how it works. The gist of how it's used is obvious, but the
 * details can often be the frustrating part.
 */
typedef struct PrintedDataStruct {
  int labelX, y, dataX;          // y is the same for data and label, but X isnt
  volatile float oldData;               // volatile for some printed data, not all
  PRINT_TYPE type;
  volatile float* currData;           // volatile for some printed data, not all
  int dataLen;
  const char* labelPtr;                        // "labelPtr" is just the label itself. No String in C/.ino, so this is our best option.
  // (maybe add later) char* unitsPtr;
} PrintedData;

/**
 * The second complex struct. Very similar to to PrintedDataStruct and now it's
 * adjusted for Thermistors, using a float array as the oldData instead of
 * a regular float.
 */
typedef struct PrintedDataThermStruct {
  int labelX, y, dataX;          // y is the same for data and label, but X isnt
  volatile float oldData[16];               // volatile for some printed data, not all
  volatile float* currData;           // volatile for some printed data, not all
  const char* labelPtr;                        // "labelPtr" is just the label itself. No String in C/.ino, so this is our best option.
} PrintedDataTherm;

/**
 * The third complex struct. Work in progress, make this better and integrate it into
 * the way time is updated, etc in the Display.ino file.
 */
typedef struct PrintedDataTimeStuct {
  int labelX, y, dataX;          // y is the same for data and label, but X isnt
  volatile float oldData[10];               // volatile for some printed data, not all
  volatile float* currData;           // volatile for some printed data, not all
  const char* labelPtr;                        // "labelPtr" is just the label itself. No String in C/.ino, so this is our best option.
} PrintedDataTimeStuct;

typedef struct _DisplayTaskData {
    Context *context;
} DisplayTaskData;

/// The global variable used to write to the display.
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); //the display controller
/// All PrintedValue objects/structs, just stored in the array
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
PrintedData *bkAngle = &printedVals[11];


/// The thermistor data.
PrintedDataTherm thermiData;
/// The time data.
PrintedDataTimeStuct timeData;

// These methods are not documented here because the internal documentation in
// Display.ino covers it properly.
void initDisplay(Context *context);
void displayUpdate(Context *context);
void thermiDataPrint(int numberOfLines);
void timePrint();
void setupMeasurementScreen();
void drawRectangle();
bool eraseThenPrintIfDiff(int xPos, int yPos, String oldData, String newData);
//void screenEraser(int scaler, int i);
void manualScreenDataUpdater();
float aux_voltage_read();

#endif
