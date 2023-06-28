/*
** Draws the display
** Someone else fill in current display capabilities
**/
#include "Display.h"
#include "Precharge.h"
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

void displayTask(void *displayTaskData) {
  while (1) {
    Context *context = *(Context **)displayTaskData;
    context->battery_voltages.aux_battery_voltage = aux_voltage_read();
    displayUpdate(context);
#ifdef CONFIG_TEST_SCREEN_DATA
    manualScreenDataUpdater();
#endif
    vTaskDelay((20 * configTICK_RATE_HZ) / 1000);
  }
}

void initDisplay(Context *context) {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextColor(PRINT_COLOR);

  /// Initializes all the PrintedDataStructs to set their position, values,
  /// and point to the correct pointer corresponding to the correct data.
#ifdef USE_DEBUGGING_SCREEN
  *batteryVoltage = {1, 10, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, &(context->battery_voltages.hv_series_voltage), 1, "Main Batt Voltage: "};
  *motorControllerVoltage = {1, 10 + VERTICAL_SCALER * 1, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, &(context->motor_stats.motor_controller_battery_voltage), 1, "Main Batt Voltage (Motor Controller): "};
  *auxBatteryVoltage = {1, 10 + 16 * 2, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, &(context->battery_voltages.aux_battery_voltage), 1, "Aux Batt Voltage: "};
  *rpm = {1, 10 + VERTICAL_SCALER * 3, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, &(context->motor_stats.RPM), 1, "RPM: "};
  *motorTemperature = {1, 10 + VERTICAL_SCALER * 4, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, &(context->motor_temps.motor_temperature), 1, "Motor Temp: "};
  *motorCurr = {1, 10 + VERTICAL_SCALER * 5, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, &(context->motor_stats.motor_current), 1, "Motor Current: "};
  *errMessage = {1, 10 + VERTICAL_SCALER * 6, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, (float*) &(context->motor_stats.error_message), 1, "Error Message: "};
  *chargerVolt = {1, 10 + VERTICAL_SCALER * 9, 100, DEFAULT_FLOAT, NUMBER, &(context->charger_stats.output_voltage), 1, "Charger Voltage: "};
  *chargerCurr = {1, 10 + VERTICAL_SCALER * 10, 100, DEFAULT_FLOAT, NUMBER, &(context->charger_stats.output_current), 1, "Charger Current: "};
  *bmsStatusFlag = {1, 10 + VERTICAL_SCALER * 11, 100, DEFAULT_FLOAT, NUMBER, &(context->bms_status.bms_status_flag), 1, "BMS Status Flag: "};
  *evccVolt = {1, 10 + VERTICAL_SCALER * 12, 100, DEFAULT_FLOAT, NUMBER, &(context->charge_controller_stats.charge_voltage), 1, "EVCC Voltage: "};
  *angleX = {150, 10 + VERTICAL_SCALER * 9, 210, DEFAULT_FLOAT, NUMBER, &(context->gyro_kalman.angle_X), 1, "X Angle: "};
  *angleY = {150, 10 + VERTICAL_SCALER * 10, 210, DEFAULT_FLOAT, NUMBER, &(context->gyro_kalman.angle_Y), 1, "Y Angle: "};
  *hvState = {150, 10 + VERTICAL_SCALER * 11, 210, DEFAULT_FLOAT, NUMBER, (float*) &(context->hv_state), 1, "State: "};
  *mcErrors = {150, 10 + VERTICAL_SCALER * 12, 210, DEFAULT_FLOAT, NUMBER, (float*) &(context->motor_stats.error_message), 1, "MC Errors: "};
#else
  *rpm = {1, 0, 0, DEFAULT_FLOAT, NUMBER, &(context->motor_stats.RPM), 1, "RPM: "};
  *batteryVoltage = {1, 215, 0, DEFAULT_FLOAT, NUMBER, &(context->battery_voltages.hv_series_voltage), 3, "Main Batt Voltage: "};
  *hvState = {160, 230, 200, DEFAULT_FLOAT, NUMBER, (float*) &(context->hv_state), 1, "State: "};
  *mcErrors = {160, 200, 225, DEFAULT_FLOAT, NUMBER, (float*) &(context->motor_stats.error_message), 1, "MC Errors: "};
  *bmsStatusFlag = {160, 185, 260, DEFAULT_FLOAT, NUMBER, &(context->bms_status.bms_status_flag), 1, "BMS Status Flag: "};
  *angleX = {160, 155, 180, DEFAULT_FLOAT, NUMBER, &(context->gyro_kalman.angle_X), 1, "X: "};
  *angleY = {160, 170, 180, DEFAULT_FLOAT, NUMBER, &(context->gyro_kalman.angle_Y), 1, "Y: "};
#endif
  thermiData = {1, 10 + VERTICAL_SCALER * 7, 90, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, context->thermistor_temps.temps, "Thermist Temp: "};

  /// Calls on the setupMeasurementScreen() to finish up the setup.
  setupMeasurementScreen();
}

void displayUpdate(Context *context) {
  // Printing out thermistor data
  thermiDataPrint();

#ifdef USE_DEBUGGING_SCREEN
  tft.setTextSize(1);
  for (int i = 0; i < NUM_DATA - 2; i++) {
    if (printedVals[i].type == NUMBER) {
      if ((*printedVals[i].currData) != (printedVals[i].oldData) || (printedVals[i].oldData) == DEFAULT_FLOAT) {
        float newData = *printedVals[i].currData;
        eraseThenPrintIfDiff(printedVals[i].dataX, printedVals[i].y, printedVals[i].oldData, newData);
        printedVals[i].oldData = newData;
      }
    }
  }
  
  HV_STATE newState = context->hv_state;
  if (eraseThenPrintIfDiff(printedVals[13].dataX, printedVals[13].y, state_name((int)printedVals[13].oldData), state_name(newState))) {
    printedVals[13].oldData = newState;
  }

  static int oldMsg = -1;
  int newMsg = context->motor_stats.error_message;
  if (oldMsg != newMsg) {
    tft.setCursor(printedVals[14].dataX, printedVals[14].y);
    tft.setTextColor(BACKGROUND_COLOR);
    tft.print(oldMsg,arduino::BIN);
    tft.setCursor(printedVals[14].dataX, printedVals[14].y);
    tft.setTextColor(PRINT_COLOR);
    tft.print(newMsg,arduino::BIN);
    oldMsg = newMsg;
  }
  
  // Printing out time data
  timePrint();

#else // (if speedometer screen)
  tft.setTextSize(8);
  // update screen if RPM changed
  if ((*printedVals[0].currData) != (printedVals[0].oldData) || (printedVals[0].oldData) == DEFAULT_FLOAT) {
    static int oldSpeed = -1;
    float newRPM = (*printedVals[0].currData);
    // Gear ratio, 48 teeth in the back wheel sprocoket. 16 on motor sprocket
    // Diameter = 0.522 m, divided by 60 converts it into per second, so the RPM is converted to a final
    // Speed of m/s
    int newSpeed = (int)(newRPM / GEAR_RATIO * PI * DIAMETER / 60 * MPH_CONVERT);
    if (eraseThenPrintIfDiff(175, 0, oldSpeed, newSpeed)) {
      oldSpeed = newSpeed;
    }
  }

  if ((*printedVals[1].currData) != (printedVals[1].oldData) || (printedVals[1].oldData) == DEFAULT_FLOAT) {
     tft.setTextSize(3);
     float newVoltage = *printedVals[0].currData;
     eraseThenPrintIfDiff(0, 215, (String)printedVals[0].oldData + " V", (String)newVoltage + " V");
     printedVals[1].oldData = newVoltage;
  }

  tft.setTextSize(1);
  HV_STATE newState = context->hv_state;
  if (eraseThenPrintIfDiff(printedVals[2].dataX, printedVals[2].y, state_name((int)printedVals[2].oldData), state_name(newState))) {
    printedVals[2].oldData = newState;
  }

  static int oldMsg = -1;
  int newMsg = context->motor_stats.error_message;
  if (oldMsg != newMsg) {
    tft.setCursor(printedVals[3].dataX, printedVals[3].y);
    tft.setTextColor(BACKGROUND_COLOR);
    tft.print(oldMsg,arduino::BIN);
    tft.setCursor(printedVals[3].dataX, printedVals[3].y);
    tft.setTextColor(PRINT_COLOR);
    tft.print(newMsg,arduino::BIN);
    oldMsg = newMsg;
  }
  
  for (int i = 4; i < NUM_DATA; i++) {
    if (printedVals[i].type == NUMBER) {
      if ((*printedVals[i].currData) != (printedVals[i].oldData) || (printedVals[i].oldData) == DEFAULT_FLOAT) {
        tft.setTextSize(printedVals[i].textSize);
        float newData = *printedVals[i].currData;
        eraseThenPrintIfDiff(printedVals[i].dataX, printedVals[i].y, printedVals[i].oldData, newData);
        printedVals[i].oldData = newData;
      }
    }
  }
#endif
}

void thermiDataPrint() {
#ifdef USE_DEBUGGING_SCREEN
  // number of thermistor values to print per line
  int incr = CONFIG_THERMISTOR_COUNT / 2; // ok there is really only room for 16 right now
  for (int i = 0; i < 2; i++) {
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
#else
  byte highestTemp = 0;
  for (int i = 0; i < CONFIG_THERMISTOR_COUNT; i++) {
    if ((byte)thermiData.currData[i] > highestTemp) {
      highestTemp = (byte)thermiData.currData[i];
      eraseThenPrintIfDiff(240, 215, thermiData.oldData[0], highestTemp);
      thermiData.oldData[0] = highestTemp;
    }
  }
#endif
}

void timePrint() {
  const int bufSize = 255;
  // date and time, update by erasing previous text then writing new
  static char previousTime[bufSize];
  char buf[bufSize];
  sprintf(buf, "%02u/%02u/%02u  %02u:%02u:%02u", month(), day(), year(), hour(), minute(), second());
  if (eraseThenPrintIfDiff(140, printedVals[10].y + VERTICAL_SCALER, previousTime, buf)) {
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
/// the data labels in their locations. This is achieved using a bunch of tft methods. Nice.
#else
  tft.setTextSize(5);

  tft.setCursor(0,0);
  tft.print("SPEED");

  tft.setCursor(220, 62);
  tft.print("mph");
  
  tft.setTextSize(1);
  tft.setCursor(160, 215);
  tft.print("Highest Temp: ");
  
  for (int i = 2; i < NUM_DATA; i++) {
    tft.setTextSize(printedVals[i].textSize);
    tft.setCursor(printedVals[i].labelX, printedVals[i].y);
    tft.print(printedVals[i].labelPtr);
  }
#endif
}

bool eraseThenPrintIfDiff(int xPos, int yPos, String oldData, String newData) {
  /// Erases the old value using the oldData parameter by writing it in
  /// the background color and then setting it back to the printing color to write the newData.
  if (newData != oldData) {
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

/* Leave in display task or move to precharge, don't move to CAN. *
 * There is no CAN bus functionality here.                        */
float aux_voltage_read() {
  float aux_voltage = 3.3 * analogRead(13) / 1024.0;
  aux_voltage *= 42.0 / 10.0;
  return  aux_voltage;
}
