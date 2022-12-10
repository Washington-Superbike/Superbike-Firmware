#include "Display.h"
#include "Main.h"
#include "FreeRTOS_TEENSY4.h"

float x = 0;
// TODO:
// 1. Make internal methods private (really annoying to do, requires messing with overall structure of code to use classes, etc. Not worth)
// 2. Remove all vars that are completley unused
// 3. Figure out how to fix the updateNumbers method (remove it) such that it doesn't break
//    precharge and main methods.
// 4. Write out a header comment for this file and all other files.
// 5. Write out comments for big blocks that are hard to interpret.
// 6. Write out comments for all merhods explaining them.

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
PrintedData *thermTemps = &printedVals[7];
PrintedData *chargerVolt = &printedVals[8];
PrintedData *chargerCurr = &printedVals[9];
PrintedData *bmsStatusFlag = &printedVals[10];
PrintedData *evccVolt = &printedVals[11];
PrintedData *timeData = &printedVals[12];

void displayTask(void *displayTaskWrap) {
    while (1) {
        displayPointer displayWrapper = *(displayPointer *)displayTaskWrap;
        MeasurementScreenData test = *(displayWrapper.msDataWrap);
        drawMeasurementScreen(*(displayWrapper.msDataWrap), *(displayWrapper.screenDataWrap));
        *(test.mainBatteryVoltage)++;
        vTaskDelay((20 * configTICK_RATE_HZ) / 1000);
    }
}

void setupDisplay(MeasurementScreenData msData, Screen screen) {
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(ILI9341_WHITE);

    *batteryVoltage = {1, 10, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.mainBatteryVoltage, 1, "Main Batt Voltage: "};
    *motorControllerVoltage = {1, 10+VERTICAL_SCALER*1, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorControllerVoltage, 1, "Main Batt Voltage (Motor Controller): "};
    float auxVolt = aux_voltage_read();
    *auxBatteryVoltage = {1, 10 + 16*2, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, &auxVolt, 1, "Aux Batt Voltage: "};
    *rpm = {1, 10 + VERTICAL_SCALER*3, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER,msData.RPM, 1, "RPM: "};
    *motorTemperature = {1, 10 + VERTICAL_SCALER*4, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorTemp, 1, "Motor Temp: "};
    *motorCurr = {1, 10 + VERTICAL_SCALER*5, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorCurrent, 1, "Motor Current: "};
    *errMessage = {1, 10 + VERTICAL_SCALER*6, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, (float*) msData.errorMessage, 1, "Error Message: "};
    *thermTemps = {1, 10 + VERTICAL_SCALER*7, 90, DEFAULT_FLOAT, ARRAY, (float*) msData.thermistorTemps, 10, "Thermist Temp: "};
    *chargerVolt = {1, 10 + VERTICAL_SCALER*8, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.chargerVoltage, 1, "Charger Voltage: "};
    *chargerCurr = {1, 10 + VERTICAL_SCALER*9, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.chargerCurrent, 1, "Charger Current: "};
    *bmsStatusFlag = {1, 10 + VERTICAL_SCALER*10, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorTemp, 1, "BMS Status Flag: "};
    *evccVolt = {1, 10 + VERTICAL_SCALER*11, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorTemp, 1, "EVCC Voltage: "};
//    Unsure if this messes with the code, or reduces the potentialLost value beyond being usable
    
    setupMeasurementScreen();
}

// Update function for display.

// TODO: DISCUSS screen function:
// Again, what point does the Screen serve?
// Do we truly need a multiple screen setup?
// We only really need the testing screen?
// Unless we want some kind of cool Odometer?
// Potential implementation of screen-based swapping:
// if testingScreen ...
// if drivingScreen ... (load an odometer, etc.)
void displayTask(MeasurementScreenData msData, Screen screen) {
    drawMeasurementScreen(msData, screen);
}

void drawMeasurementScreen(MeasurementScreenData msData, Screen screen) {
    tft.setTextSize(1);

    int bufSize = 255;
    // date and time, update by erasing previous text then writing new
    static char previousTime[bufSize] = "01/01/1970 00:00:00";
    char buf[bufSize];
    sprintf(buf, "%02u/%02u/%02u  %02u:%02u:%02u", month(), day(), year(), hour(), minute(), second());
    tft.setCursor(140, 10+VERTICAL_SCALER*12);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(previousTime);
    tft.setCursor(140, 10+VERTICAL_SCALER*12);
    tft.setTextColor(ILI9341_BLACK);
    tft.println(buf);
    memcpy(previousTime,buf, bufSize);
    
    for (int i = 0; i < NUM_DATA; i++) {
      tft.setCursor(printedVals[i].dataX, printedVals[i].y);

      // array does not have the "don't print if data hasn't been updated" quality yet
      if (printedVals[i].type == ARRAY) {
        screenEraser(17*10, i);
        String s;
        for (int j = 0; j < printedVals[i].dataLen; j++) {
          s.append((byte)printedVals[i].currData[j]);
          if(j!=9){
            s.append(", ");
          }
        }
        tft.print(s);
      } else if(printedVals[i].type == NUMBER) {
        if((*printedVals[i].currData) != (printedVals[i].oldData) || (printedVals[i].oldData) == DEFAULT_FLOAT) {
          tft.setCursor(printedVals[i].dataX, printedVals[i].y);
          screenEraser(23, i);
          tft.setTextColor(ILI9341_BLACK);
          tft.print(*printedVals[i].currData);
          printedVals[i].oldData = *printedVals[i].currData;
        }
      }
    }
}

void setupMeasurementScreen() {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    
    for (int i = 0; i < NUM_DATA; i++) {
      tft.setCursor(printedVals[i].labelX, printedVals[i].y);
      tft.print(printedVals[i].labelPtr);
    }
}

void screenEraser(int scaler, int i){
  // floor(log10(abs(*printedVals[i].currData))) + 1 = formula to return the number of digits in the int.
  // Attempting to scale the clearing rectangle based on int length, except, when int length = 1, it's kinda useless.
  tft.fillRect(printedVals[i].dataX, printedVals[i].y, scaler * (floor(log10(abs(*printedVals[i].currData))) + 1), 8, ILI9341_WHITE);
}

float aux_voltage_read(){
  float aux_voltage = 3.3*analogRead(13)/1024.0;
  aux_voltage *= 42.0/10.0;
  return  aux_voltage;
}
