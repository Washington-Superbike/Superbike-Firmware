/**
   @file Display.ino
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The DataLogging.h config file for CAN bus for the bike's firmware. This initializes
          all variables that are passed along to all other files as
          pointers. Then it runs the setup methods for all those
          files and then it sets up RTOS to run all the different files
          as individual tasks. These tasks are: datalogging,
          display, precharge, CAN, idle. These tasks will be further
          described in the documentation for their individual files.


    \note
      up all members to be able to use it without any trouble.

    \todo
      PLEASE remove the whole screen parameter situation. Change the whole
      value in the
      \n \n
      Goal 2.
      \n \n
      Goal 3.
      \n \n
      Change the thermistor code to use "NUM_THERMI" from Main.h
      Final Goal.
*/

#include "Display.h"
#include "Main.h"
#include "FreeRTOS_TEENSY4.h"

// TODO:
//  Remove all vars that are completley unused
//  Write out a header comment for this file and all other files.
//  Write out comments for big blocks that are hard to interpret.
//  Write out comments for all functions explaining them.
// 1. Remove oldData field from the printedData struct.
// 2. Add more macros for default vals that are repeated in Display.ino
// 3. Remove all the extra macros and predefined methods
// 4. Remove everything that corresponds to things removed from Display.ino


void displayTask(void *displayTaskWrap) {
  while (1) {
    displayPointer displayWrapper = *(displayPointer *)displayTaskWrap;
    MeasurementScreenData test = *(displayWrapper.msDataWrap);
    displayUpdate(*(displayWrapper.msDataWrap), *(displayWrapper.screenDataWrap));
    manualScreenDataUpdater();
    vTaskDelay((20 * configTICK_RATE_HZ) / 1000);
  }
}

void setupDisplay(MeasurementScreenData msData, Screen screen) {
  tft.begin();
  tft.setRotation(1);
  if (screen.screenType == DEBUG){
    tft.fillScreen(ILI9341_WHITE);
  }

  if (screen.screenType == SPEEDOMETER){
    tft.fillScreen(ILI9341_BLACK);
  }

  //PrintedData *thermTemps = &printedVals[7];
  //PrintedData *timeData = &printedVals[11];

  *batteryVoltage = {1, 10, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.mainBatteryVoltage, 1, "Main Batt Voltage: "};
  *motorControllerVoltage = {1, 10 + VERTICAL_SCALER * 1, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorControllerVoltage, 1, "Main Batt Voltage (Motor Controller): "};
  float auxVolt = aux_voltage_read();
  *auxBatteryVoltage = {1, 10 + 16 * 2, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, &auxVolt, 1, "Aux Batt Voltage: "};
  *rpm = {1, 10 + VERTICAL_SCALER * 3, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.RPM, 1, "RPM: "};
  *motorTemperature = {1, 10 + VERTICAL_SCALER * 4, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorTemp, 1, "Motor Temp: "};
  *motorCurr = {1, 10 + VERTICAL_SCALER * 5, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorCurrent, 1, "Motor Current: "};
  *errMessage = {1, 10 + VERTICAL_SCALER * 6, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, (float*) msData.errorMessage, 1, "Error Message: "};
  *chargerVolt = {1, 10 + VERTICAL_SCALER * 9, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.chargerVoltage, 1, "Charger Voltage: "};
  *chargerCurr = {1, 10 + VERTICAL_SCALER * 10, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.chargerCurrent, 1, "Charger Current: "};
  *bmsStatusFlag = {1, 10 + VERTICAL_SCALER * 11, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorTemp, 1, "BMS Status Flag: "};
  *evccVolt = {1, 10 + VERTICAL_SCALER * 12, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorTemp, 1, "EVCC Voltage: "};

  thermiData = {1, 10 + VERTICAL_SCALER * 7, 180, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, (float*) msData.thermistorTemps, "Thermist Temp: "};

  setupMeasurementScreen(screen);
}

void displayUpdate(MeasurementScreenData msData, Screen screen) {
  if (screen.screenType == DEBUG) {
    tft.setTextSize(1);
    for (int i = 0; i < NUM_DATA; i++) {
      if (printedVals[i].type == NUMBER) {
        if ((*printedVals[i].currData) != (printedVals[i].oldData) || (printedVals[i].oldData) == DEFAULT_FLOAT) {
          eraseThenPrint(printedVals[i].dataX, printedVals[i].y, printedVals[i].oldData, *printedVals[i].currData);
          printedVals[i].oldData = *printedVals[i].currData;
        }
      }
    }

    // Printing out thermistor data
    thermiDataPrint(true);

    // Printing out time data
    timePrint();
  }

  if (screen.screenType == SPEEDOMETER){
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(8);
    String newString = "";
    String oldString = "";
    if ((*printedVals[3].currData) != (printedVals[3].oldData) || (printedVals[3].oldData) == DEFAULT_FLOAT) {
//    Gear ratio, 48 teeth in the back wheel sprocoket. 16 on motor sprocket
//    Diameter = 0.522 m, divided by 60 converts it into per second, so the RPM is converted to a final
//    Speed of m/s
      float gearRatio = 48/16;
      float diameterPerSecond = 0.522/60;
      float pi = 3.14159;
      float mphConvert = 2.2369362920544;

      float newSpeed = *printedVals[3].currData / gearRatio * pi * diameterPerSecond * mphConvert;
      float oldSpeed = printedVals[3].oldData / gearRatio * pi * diameterPerSecond * mphConvert;
      newString = (String) (int) newSpeed;
      oldString = (String) (int) oldSpeed;
//      Serial.println(newString);
//      Serial.println(oldString);
      eraseThenPrintSPEEDO(175, 0, oldString, newString);
      newString = (String) (int) *printedVals[3].currData;
      oldString = (String) (int) printedVals[3].oldData;
      eraseThenPrintSPEEDO(120, 180, oldString, newString);
      printedVals[3].oldData = *printedVals[3].currData;
    }
  }
}

void thermiDataPrint(bool thermiDataPrint) {
  if (thermiDataPrint) {

    bool sameData = true;
    String sOld, sNew;

    for (int j = 0; j < 5; j++) {
      if (thermiData.currData[j] != thermiData.oldData[j]) {
        sameData = false;
        sNew.append((byte)thermiData.currData[j]);
        sOld.append((byte)thermiData.oldData[j]);
        thermiData.oldData[j] = thermiData.currData[j];
      } else {
        sOld.append((byte)thermiData.oldData[j]);
        sNew.append((byte)thermiData.oldData[j]);
      }
      if (j != 4) {
        sOld.append(", ");
        sNew.append(", ");
      }
    }
    if (!sameData) {
      eraseThenPrint(thermiData.dataX, thermiData.y, sOld, sNew);
    }
    sameData = true;
    sOld = "";
    sNew = "";

    for (int j = 5; j < 10; j++) {
      if (thermiData.currData[j] != thermiData.oldData[j]) {
        sameData = false;
        sNew.append((byte)thermiData.currData[j]);
        sOld.append((byte)thermiData.oldData[j]);
        thermiData.oldData[j] = thermiData.currData[j];
      } else {
        sOld.append((byte)thermiData.oldData[j]);
        sNew.append((byte)thermiData.oldData[j]);
      }
      if (j != 9) {
        sOld.append(", ");
        sNew.append(", ");
      }
    }
    if (!sameData) {
      eraseThenPrint(thermiData.dataX, thermiData.y + VERTICAL_SCALER, sOld, sNew);
    }

  }
}

void timePrint() {
  const int bufSize = 255;
  // date and time, update by erasing previous text then writing new
  static char previousTime[bufSize];
  char buf[bufSize];
  sprintf(buf, "%02u/%02u/%02u  %02u:%02u:%02u", month(), day(), year(), hour(), minute(), second());
  eraseThenPrint(140, 10 + VERTICAL_SCALER * (NUM_DATA + 2), previousTime, buf);
  memcpy(previousTime, buf, bufSize);

}

void setupMeasurementScreen(Screen screen) {
  if (screen.screenType == DEBUG) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);

    for (int i = 0; i < NUM_DATA; i++) {
      tft.setCursor(printedVals[i].labelX, printedVals[i].y);
      tft.print(printedVals[i].labelPtr);
    }

    // Printing out time label
    tft.setCursor(printedVals[10].labelX, printedVals[10].y + VERTICAL_SCALER);
    tft.print("Time: ");

    // Printing out thermistor temps label
    tft.setCursor(thermiData.labelX, thermiData.y);
    tft.print(thermiData.labelPtr);
  }
  if (screen.screenType == SPEEDOMETER){
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(5);

    tft.setCursor(0,0);
    tft.print("SPEED");

    tft.setCursor(220, 62);
    tft.print("mph");

    tft.setCursor(0, 205);
    tft.print("RPM");
  }
}

void eraseThenPrint(int xPos, int yPos, String oldData, String newData) {
  tft.setCursor(xPos, yPos);
  tft.setTextColor(ILI9341_WHITE);
  tft.print(oldData);
  tft.setCursor(xPos, yPos);
  tft.setTextColor(ILI9341_BLACK);
  tft.print(newData);
}

void eraseThenPrintSPEEDO(int xPos, int yPos, String oldData, String newData) {
  tft.setCursor(xPos, yPos);
  tft.setTextColor(ILI9341_BLACK);
  tft.print(oldData);
  tft.setCursor(xPos, yPos);
  tft.setTextColor(ILI9341_WHITE);
  tft.print(newData);
}

//void screenEraser(int scaler, int i) {
//  floor(log10(abs(*printedVals[i].currData))) + 1 = formula to return the number of digits in the int.
//  Attempting to scale the clearing rectangle based on int length, except, when int length = 1, it's kinda useless.
//  tft.fillRect(printedVals[i].dataX, printedVals[i].y, scaler * (floor(log10(abs(*printedVals[i].currData))) + 1), 8, ILI9341_WHITE);
//}

void manualScreenDataUpdater() {
  for (int i = 0; i < NUM_DATA; i++) {
    if (printedVals[i].type == NUMBER) {
      *printedVals[i].currData = *printedVals[i].currData + 1;
//      Serial.println(*printedVals[i].currData);
    }
  }
  for (int j = 0; j < 10; j++) {
    thermiData.currData[j]++;
  }
}

float aux_voltage_read() {
  float aux_voltage = 3.3 * analogRead(13) / 1024.0;
  aux_voltage *= 42.0 / 10.0;
  return  aux_voltage;
}
