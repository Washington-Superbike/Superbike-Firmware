/**
   @file Display.ino
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The Display.ino file executes the Display task for the bike's firmware.
          This calls on the variables that are initialized in Display.h
          and then calls on various methods from the ILI9341's adafruit display library to update the display. Updating the display includes comparing old and new
          data to ensure data has changed, erasing the previous data if it's changed
          and writing in the new data.


    \note
      This works. Display does slow down a bit in the speedometer screen, but
      that is also due to often the numbers were changing in the manualScreenDataUpdater()method updating data too frequently. Just consider reducing the size of the
      text in case it looks like it is updating too slowly at speeds of ~60-80 mph.

    \todo
      Remove all unused vars and tighten up methods.
      \n \n
      There are WAY more macros (#define statements) that need to be written
      for defining some frequently used values in display. Can't think
      of them off the top of my head, but make and use those.
      \n \n
      In the speedometer screen, add code to show angles.
      If the angle is near the threshold (> ~30 degrees), show it in red.
      If the angle is near safe (< ~30 degrees), show it in green.
      Add an indicator that prints out preCharge statuses
        This variable is called hv_state in preCharge.h
        basically print out this variable if possbie.
        This is similar to how cars print out engine flags, etc.
*/

#include "Display.h"
#include "Main.h"
#include "FreeRTOS_TEENSY4.h"

void displayTask(void *measurementDataPtr) {
  while (1) {
    /// Parses the data from the void pointer to be processable? Idk if that's word.
    /// Passes the measurementScreenData to the displayUpdate()
    /// method. The displayUpdate() method then processes it and updates
    /// the screen accordingly.
    MeasurementScreenData msData = *(MeasurementScreenData *)measurementDataPtr;
    displayUpdate(msData);
    manualScreenDataUpdater();
    vTaskDelay((20 * configTICK_RATE_HZ) / 1000);
  }
}

void setupDisplay(MeasurementScreenData msData) {
  /// Calls on tft.begin() method and sets the orientation using
  /// tft.setRotatio() and also sets the screen to ILI9341_WHITE
  /// or ILI9341_BLACK based on Debugging or Speedometer screen.
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextColor(PRINT_COLOR);

  //PrintedData *thermTemps = &printedVals[7];
  //PrintedData *timeData = &printedVals[11];

  /// Initializes all the PrintedDataStructs to set their position, values,
  /// and point to the correct pointer corresponding to the correct data.
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

  thermiData = {1, 10 + VERTICAL_SCALER * 7, 90, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, (float*) msData.thermistorTemps, "Thermist Temp: "};

  /// Calls on the setupMeasurementScreen() to finish up the setup.
  setupMeasurementScreen();
}

void displayUpdate(MeasurementScreenData msData) {
#ifdef USE_DEBUGGING_SCREEN
  tft.setTextSize(1);
  for (int i = 0; i < NUM_DATA; i++) {
    if (printedVals[i].type == NUMBER) {
      if ((*printedVals[i].currData) != (printedVals[i].oldData) || (printedVals[i].oldData) == DEFAULT_FLOAT) {
        float newData = *printedVals[i].currData;
        eraseThenPrintIfDiff(printedVals[i].dataX, printedVals[i].y, printedVals[i].oldData, newData);
        printedVals[i].oldData = newData;
      }
    }
  }
  
  // Printing out thermistor data over 2 lines
  thermiDataPrint(2);
  
  // Printing out time data
  timePrint();

#else // (if speedometer screen)
  tft.setTextSize(8);
  // update screen if RPM changed
  if ((*printedVals[3].currData) != (printedVals[3].oldData) || (printedVals[3].oldData) == DEFAULT_FLOAT) {
    static int oldSpeed = -1;
    float newRPM = (*printedVals[3].currData);
    
    // Gear ratio, 48 teeth in the back wheel sprocoket. 16 on motor sprocket
    // Diameter = 0.522 m, divided by 60 converts it into per second, so the RPM is converted to a final
    // Speed of m/s
    int newSpeed = (int)(newRPM / GEAR_RATIO * PI * DIAMETER / 60 * MPH_CONVERT);
    if (eraseThenPrintIfDiff(175, 0, oldSpeed, newSpeed)) {
      oldSpeed = newSpeed;
    }
    tft.setTextSize(3);
    eraseThenPrintIfDiff(70, 180, (int)printedVals[3].oldData, (int)newRPM);
    printedVals[3].oldData = newRPM;
  }
  // update screen if main battery voltage changed
  if ((*printedVals[0].currData) != (printedVals[0].oldData) || (printedVals[0].oldData) == DEFAULT_FLOAT) {
     tft.setTextSize(3);
     float newVoltage = *printedVals[0].currData;
     eraseThenPrintIfDiff(0, 215, (String)printedVals[0].oldData + " V", (String)newVoltage + " V");
     printedVals[0].oldData = newVoltage;
  }
#endif
}

void thermiDataPrint(int numberOfLines) {
  // number of thermistor values to print per line
  int incr = 16 / numberOfLines; // ok there is really only room for 16 right now
  for (int i = 0; i < numberOfLines; i++) {
    String sOld, sNew;
    // process data/update display one line of therm values at a time
    for (int j = i * incr; j < incr + (i * incr); j++) {
      if (thermiData.currData[j] != thermiData.oldData[j]) {
        sNew.append((byte)thermiData.currData[j]);
        sOld.append((byte)thermiData.oldData[j]);
        thermiData.oldData[j] = thermiData.currData[j];
      } else {
        sOld.append((byte)thermiData.oldData[j]);
        sNew.append((byte)thermiData.oldData[j]);
      }
      // if not the last value on a line, add a comma
      if (j != (incr + (i * incr)) - 1) {
        sOld.append(", ");
        sNew.append(", ");
      }
    }
    // if therm data has changed since the last print, update the display
    eraseThenPrintIfDiff(thermiData.dataX, thermiData.y + (VERTICAL_SCALER * i), sOld, sNew);
  }
}

void timePrint() {
  const int bufSize = 255;
  // date and time, update by erasing previous text then writing new
  static char previousTime[bufSize];
  char buf[bufSize];
  sprintf(buf, "%02u/%02u/%02u  %02u:%02u:%02u", month(), day(), year(), hour(), minute(), second());
  if (eraseThenPrintIfDiff(140, 10 + VERTICAL_SCALER * (NUM_DATA + 2), previousTime, buf)) {
    memcpy(previousTime, buf, bufSize);
  }
}

void setupMeasurementScreen() {
/// If the screen is debugging type, it will print out all
/// the data labels in their data locations as a black text
/// This is achieved using a bunch of tft methods. Nice.
#ifdef USE_DEBUGGING_SCREEN
  tft.setTextSize(1);

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
  
/// If the screen is speedometer type, it will print out all
/// the data labels in their locations (SPEED up top and RPM
/// at the bottom). This is achieved using a bunch of tft methods. Nice.
#else
  tft.setTextSize(5);

  tft.setCursor(0,0);
  tft.print("SPEED");

  tft.setCursor(220, 62);
  tft.print("mph");

  tft.setTextSize(3);
  tft.setCursor(0, 180);
  tft.print("RPM");
#endif
}

bool eraseThenPrintIfDiff(int xPos, int yPos, String oldData, String newData) {
  /// Erases the old value using the oldData parameter by writing it in
  /// the background color and then setting it back to the printing color to write the newData.
  if (oldData != newData) {
    tft.setCursor(xPos, yPos);
    tft.setTextColor(BACKGROUND_COLOR);
    tft.print(oldData);
    tft.setCursor(xPos, yPos);
    tft.setTextColor(PRINT_COLOR);
    tft.print(newData);
    return true;
  }
  return false;
}

//void screenEraser(int scaler, int i) {
//  floor(log10(abs(*printedVals[i].currData))) + 1 = formula to return the number of digits in the int.
//  Attempting to scale the clearing rectangle based on int length, except, when int length = 1, it's kinda useless.
//  tft.fillRect(printedVals[i].dataX, printedVals[i].y, scaler * (floor(log10(abs(*printedVals[i].currData))) + 1), 8, ILI9341_WHITE);
//}

void manualScreenDataUpdater() {
  /// Iterates through all the printedVals array and then
  /// does += 1 to make sure it's increasing. This change should be
  /// reflected in the display itself.
  for (int i = 0; i < NUM_DATA; i++) {
    if (printedVals[i].type == NUMBER) {
      *printedVals[i].currData = *printedVals[i].currData + 1;
//      Serial.println(*printedVals[i].currData);
    }
  }
  for (int j = 0; j < 16; j++) {
    thermiData.currData[j]++;
  }
}

float aux_voltage_read() {
  /// Reads in the aux_voltage of the LV system and
  /// reads in the values. Would be better in CAN,
  /// but alas Chase said leave it here, it's here.
  float aux_voltage = 3.3 * analogRead(AUX_BAT_PIN) / 1024.0;
  aux_voltage *= 42.0 / 10.0;
  return  aux_voltage;
}
