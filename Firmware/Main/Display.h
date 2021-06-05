
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#define TFT_CS 1 //The display chip select pin
#define TFT_DC 3 // the display
#define TFT_RST 2 //the display reset pin


#define TS_CS  9
// MOSI=11, MISO=12, SCK=13

#ifndef DISPLAY_H_
#define DISPLAY_H_

// The TIRQ interrupt signal must be used for this example.
#define TIRQ_PIN  -1


#define TS_MINX  327
#define TS_MAXX  3903
#define TS_MINY  243
#define TS_MAXY  3842

typedef struct MeasurementScreenDataStruct {
    float* mainBatteryVoltage;
    float* motorControllerVoltage;
    float* auxiliaryBatteryVoltage;
    float* RPM;
    float* motorTemp;
    float* motorCurrent;
    int* errorMessage;
    float *thermistorTemps;
    float *chargerCurrent;
    float *chargerVoltage;
    float prevMainBattVoltage;
    float prevAuxBattVoltage;
    int prevRPM;
    float prevMotorTemp;
    float prevMotorCurrent;
} MeasurementScreenData;

typedef struct ScreenInfo {
    int px;
    int py;
    int pz;
    bool recentlyChanged;
}Screen;

void drawMeasurementScreen(MeasurementScreenData msData);
void displayTask(MeasurementScreenData msData, Screen screen);

#endif
