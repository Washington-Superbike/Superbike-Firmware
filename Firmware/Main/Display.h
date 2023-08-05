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
#define DISPLAY_TASK_STACK_SIZE 15000
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

// Default Values:
/// The default starter value used as old data in PrintedData objects
#define DEFAULT_VAL -1
/// The default x position used for printing the data
#define DEFAULT_X_POS 235

// Multipliers and Formula Constants:
/// A Scaler for moving vertical data down. Each row of data is a factor of
/// 16 pixels away from each other, vertically.
#define VERTICAL_SCALER 16

/// Sets a default erasing(background)/writing(print) color based on screen type
/// and the number of PrintedData values (length of the array that contains all the PrintedData pointers)
#ifdef USE_DEBUGGING_SCREEN
#  define BACKGROUND_COLOR ILI9341_WHITE
#  define PRINT_COLOR ILI9341_BLACK
#  define NUM_DATA 17
#else // (if speedometer screen)
#  define BACKGROUND_COLOR ILI9341_BLACK
#  define PRINT_COLOR ILI9341_WHITE
#  define NUM_DATA 8
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

typedef struct _DisplayTaskData {
  Context *context;
} DisplayTaskData;

// These methods are not documented here because the internal documentation in
// Display.ino covers it properly.
void displayTask(void *displayTaskData);
void setupDisplay();
void updateDisplay();
void manualScreenDataUpdater();
void printHV(float *oldData, float currData, int xPos, int yPos);
void printSpeed(int *oldData, int currData, int xPos, int yPos);
void printMCErrors(int *oldData, int currData, int xPos, int yPos);
void printHVState(HV_STATE *oldData, HV_STATE currData, int xPos, int yPos);
void printTime(char *oldDataArr, volatile char *currDataArr, int arrLength, int xPos, int yPos);
void printTempList(float *oldDataArr, volatile float *currDataArr, int arrLength, int xPos, int yPos);
void printHighestTemp(float *oldDataArr, volatile float *currDataArr, int arrLength, int xPos, int yPos);
String binaryFormat(int n, int digits);
float aux_voltage_read();

#endif
