
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#define TFT_CS 21 //The display chip select pin
#define TFT_DC 19 // the display
#define TFT_RST 20 //the display reset pin


#define TS_CS  15
// MOSI=11, MISO=12, SCK=13

#ifndef DISPLAY_H_
#define DISPLAY_H_

// The TIRQ interrupt signal must be used for this example.
#define TIRQ_PIN  2


#define TS_MINX  327
#define TS_MAXX  3903
#define TS_MINY  243
#define TS_MAXY  3842

typedef struct MeasurementScreenDataStruct {
    float* mainBatteryVoltage;
    float* auxiliaryBatteryVoltage;
    float* RPM;
    float* motorTemp;
    float* motorCurrent;
    int* errorMessage;
} MeasurementScreenData;



void drawMeasurementScreen(MeasurementScreenData msData);
void displayTask(MeasurementScreenData msData);

#endif
