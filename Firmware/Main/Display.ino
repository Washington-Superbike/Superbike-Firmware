/*
** Draws the display
** Someone else fill in current display capabilities
**/
#include "Display.h"
#include "Precharge.h"
#include "PrintedData.h"
#include "arduino_freertos.h"
#include "avr/pgmspace.h"

/// The global variable used to write to the display.
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); //the display controller

/// Array storing pointers to all PrintedData objects.
PrintedData *printedVals[NUM_DATA];

void displayTask(void *displayTaskData) {
  Context *context = *(Context **)displayTaskData;
  
  // Initialize all the PrintedDataStructs to set their position, values,
  // and point to the correct pointer corresponding to the correct data.
#ifdef USE_DEBUGGING_SCREEN
  // Allocate buffers for storing previously printed data.
  byte oldThermData[CONFIG_THERMISTOR_COUNT];
  const int timeBufSize = 255;
  char oldTimeData[timeBufSize];

  PrintedVal<float> batteryVoltage = {1, DEFAULT_X_POS, 10, "Main Batt Voltage: ", 1, &(context->battery_voltages.hv_series_voltage)};
  PrintedVal<bool> hvReady = {1, DEFAULT_X_POS, 10 + VERTICAL_SCALER * 1, "HV Cell Voltages Ready: ", 1, &(context->battery_voltages.hv_cell_voltages_ready)};
  PrintedVal<float> motorControllerVoltage = {1, DEFAULT_X_POS, 10 + VERTICAL_SCALER * 2, "Main Batt Voltage (Motor Controller): ", 1, &(context->motor_stats.motor_controller_battery_voltage)};
  PrintedVal<float> auxBatteryVoltage = {1, DEFAULT_X_POS, 10 + VERTICAL_SCALER * 3, "Aux Batt Voltage: ", 1, &(context->battery_voltages.aux_battery_voltage)};
  PrintedVal<int> rpm = {1, DEFAULT_X_POS, 10 + VERTICAL_SCALER * 4, "RPM: ", 1, &(context->motor_stats.RPM)};
  PrintedVal<float> motorTemperature = {1, DEFAULT_X_POS, 10 + VERTICAL_SCALER * 5, "Motor Temp: ", 1, &(context->motor_temps.motor_temperature)};
  PrintedVal<float> motorCurr = {1, DEFAULT_X_POS, 10 + VERTICAL_SCALER * 6, "Motor Current: ", 1, &(context->motor_stats.motor_current)};
  PrintedVal<float> chargerVolt = {1, 100, 10 + VERTICAL_SCALER * 9, "Charger Voltage: ", 1, &(context->charger_stats.output_voltage)};
  PrintedVal<float> chargerCurr = {1, 100, 10 + VERTICAL_SCALER * 10, "Charger Current: ", 1, &(context->charger_stats.output_current)};
  PrintedVal<int> bmsStatusFlag = {1, 100, 10 + VERTICAL_SCALER * 11, "BMS Status Flag: ", 1, &(context->bms_status.bms_status_flag)};
  PrintedVal<float> evccVolt = {1, 100, 10 + VERTICAL_SCALER * 12, "EVCC Voltage: ", 1, &(context->charge_controller_stats.charge_voltage)};
  PrintedVal<float> xAngle = {150, 210, 10 + VERTICAL_SCALER * 9, "X Angle: ", 1, &(context->gyro_kalman.angle_X)};
  PrintedVal<float> yAngle = {150, 210, 10 + VERTICAL_SCALER * 10, "Y Angle: ", 1, &(context->gyro_kalman.angle_Y)};
  PrintedVal<HV_STATE> hvState = {150, 210, 10 + VERTICAL_SCALER * 11, "State: ", 1, &(context->hv_state), printHVState};
  PrintedVal<int> errMessage = {150, 210, 10 + VERTICAL_SCALER * 12, "MC Errors: ", 1, &(context->motor_stats.error_message), printMCErrors};
  PrintedArr<byte> thermiData = {1, 90, 10 + VERTICAL_SCALER * 7, "Thermist Temp: ", 1, oldThermData, DEFAULT_VAL, context->thermistor_temps.temps, CONFIG_THERMISTOR_COUNT, printTempList};
  PrintedArr<char> timeData = {1, 35, 10 + VERTICAL_SCALER * 13, "Time: ", 1, oldTimeData, 0, NULL, timeBufSize, printTime};

  printedVals[0] = &batteryVoltage;
  printedVals[1] = &hvReady;
  printedVals[2] = &motorControllerVoltage;
  printedVals[3] = &auxBatteryVoltage;
  printedVals[4] = &rpm;
  printedVals[5] = &motorTemperature;
  printedVals[6] = &motorCurr;
  printedVals[7] = &chargerVolt;
  printedVals[8] = &chargerCurr;
  printedVals[9] = &bmsStatusFlag;
  printedVals[10] = &evccVolt;
  printedVals[11] = &xAngle;
  printedVals[12] = &yAngle;
  printedVals[13] = &hvState;
  printedVals[14] = &errMessage;
  printedVals[15] = &thermiData;
  printedVals[16] = &timeData;
#else
  byte oldThermData[1];
  
  PrintedVal<int> speedData = {1, 175, 0, "SPEED", 5, &(context->motor_stats.RPM), printSpeed};
  PrintedVal<float> batteryVoltage = {1, 0, 215, NULL, 3, &(context->battery_voltages.hv_series_voltage), printHV};
  PrintedVal<HV_STATE> hvState = {155, 195, 230, "State: ", 1, &(context->hv_state), printHVState};
  PrintedVal<int> errMessage = {155, 220, 200, "MC Errors: ", 1, &(context->motor_stats.error_message), printMCErrors};
  PrintedVal<int> bmsStatusFlag = {155, 255, 185, "BMS Status Flag: ", 1, &(context->bms_status.bms_status_flag)};
  PrintedVal<float> xAngle = {155, 170, 155, "X: ", 1, &(context->gyro_kalman.angle_X)};
  PrintedVal<float> yAngle = {155, 170, 170, "Y: ", 1, &(context->gyro_kalman.angle_Y)};
  PrintedArr<byte> thermiData = {155, 235, 215, "Highest Temp: ", 1, oldThermData, DEFAULT_VAL, context->thermistor_temps.temps, 1, printHighestTemp};

  printedVals[0] = &speedData;
  printedVals[1] = &batteryVoltage;
  printedVals[2] = &hvState;
  printedVals[3] = &errMessage;
  printedVals[4] = &bmsStatusFlag;
  printedVals[5] = &xAngle;
  printedVals[6] = &yAngle;
  printedVals[7] = &thermiData;
#endif
  setupDisplay();
  while (1) {
    context->battery_voltages.aux_battery_voltage = aux_voltage_read();
    updateDisplay();
#ifdef CONFIG_TEST_SCREEN_DATA
    manualScreenDataUpdater();
#endif
    vTaskDelay((20 * configTICK_RATE_HZ) / 1000);
  }
}

void setupDisplay() {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextColor(PRINT_COLOR);

  /// Print out the data labels.
  for (int i = 0; i < NUM_DATA; i++) {
    printedVals[i]->printLabel();
  }
}

void updateDisplay() {
  for (int i = 0; i < NUM_DATA; i++) {
    printedVals[i]->printData();
  }
}

// Prints the HV battery voltage reported by the BMS. With units.
void printHV(float *oldData, float currData, int xPos, int yPos) {
  if (*oldData != currData) {
     eraseThenPrint((String)(*oldData) + " V", (String)currData + " V", xPos, yPos);
     *oldData = currData;
  }
}

// Prints the RPM reported by the motor controller converted to speed in mph.
void printSpeed(int *oldData, int currData, int xPos, int yPos) {
  // Gear ratio, 48 teeth in the back wheel sprocoket. 16 on motor sprocket
  // Diameter = 0.522 m, divided by 60 converts it into per second, so the RPM is converted to a final
  // Speed of m/s
  int newSpeed = (int)(currData / GEAR_RATIO * PI * DIAMETER / 60 * MPH_CONVERT);
  if (*oldData != newSpeed) {
    tft.setTextSize(8);
    eraseThenPrint(*oldData, newSpeed, xPos, yPos);
    *oldData = newSpeed;
    tft.setTextSize(5);
    tft.setCursor(220, 62);
    tft.print("mph");
  }
}

// Prints the binary representation of the 2 bytes of motor controller error messages with leading 0s.
void printMCErrors(int *oldData, int currData, int xPos, int yPos) {
  if (*oldData != currData) {
    eraseThenPrint(binaryFormat(*oldData, 16), binaryFormat(currData, 16), xPos, yPos);
    *oldData = currData;
  }
}

void printHVState(HV_STATE *oldData, HV_STATE currData, int xPos, int yPos) {
  if (*oldData != currData) {
    eraseThenPrint(state_name(*oldData), state_name(currData), xPos, yPos);
    *oldData = currData;
  }
}

// Prints formatted date and time.
void printTime(char *oldDataArr, volatile char *currDataArr, int arrLength, int xPos, int yPos) {
  char buf[arrLength];
  sprintf(buf, "%02u/%02u/%02u  %02u:%02u:%02u", month(), day(), year(), hour(), minute(), second());
  if (strncmp(oldDataArr, buf, arrLength)) {
    eraseThenPrint(oldDataArr, buf, xPos, yPos);
    memcpy(oldDataArr, buf, arrLength);
  }
}

// Prints thermistor temperature values over 2 lines as a comma-separated list.
void printTempList(byte *oldDataArr, volatile byte *currDataArr, int arrLength, int xPos, int yPos) {
  int incr = CONFIG_THERMISTOR_COUNT / 2; // the printed line might go past the end of the screen if incr > 8
  for (int i = 0; i < 2; i++) {
    String sOld, sNew;
    bool newData = false;
    // process data/update display one line of therm values at a time
    for (int j = i * incr; j < incr + (i * incr); j++) {
      if (currDataArr[j] != oldDataArr[j]) {
        sNew.append(currDataArr[j]);
        sOld.append(oldDataArr[j]);
        oldDataArr[j] = currDataArr[j];
        newData = true;
      } else {
        sOld.append(oldDataArr[j]);
        sNew.append(oldDataArr[j]);
      }
      // if not the last value on a line, add a comma
      if (j != (incr + (i * incr)) - 1) {
        sOld.append(", ");
        sNew.append(", ");
      }
    }
    // if therm data has changed since the last print, update the display
    if (newData) {
      eraseThenPrint(sOld, sNew, xPos, yPos + (VERTICAL_SCALER * i));
    }
  }
}

// Cycles through the thermistor array once and print the highest temperature found.
void printHighestTemp(byte *oldDataArr, volatile byte *currDataArr, int arrLength, int xPos, int yPos) {
  byte highestTemp = 0;
  for (int i = 0; i < CONFIG_THERMISTOR_COUNT; i++) {
    byte temp = currDataArr[i];
    if (oldDataArr[0] == (byte)DEFAULT_VAL || temp > highestTemp) {
      highestTemp = temp;
      eraseThenPrint(oldDataArr[0], highestTemp, xPos, yPos);
      oldDataArr[0] = highestTemp;
    }
  }
}

void eraseThenPrint(String oldData, String newData, int xPos, int yPos) {
  /// Erases the old value using the oldData parameter by writing it in
  /// the background color and then setting it back to the printing color to write the newData.
  tft.setCursor(xPos, yPos);
  tft.setTextColor(BACKGROUND_COLOR);
  tft.print(oldData);
  tft.setCursor(xPos, yPos);
  tft.setTextColor(PRINT_COLOR);
  tft.print(newData);
}

// Returns a binary representation of n up to the given number of digits.
String binaryFormat(int n, int digits) {
  String bin_str;
  for (int i = digits - 1; i >= 0; i--) {
    bin_str.append((n >> i) & 1);
  }
  return bin_str;
}

void manualScreenDataUpdater() {
  /// Iterates through all the printedVals array and then
  /// does += 1 to make sure it's increasing. This change should be
  /// reflected in the display itself.
  for (int i = 0; i < NUM_DATA; i++) {
    printedVals[i]->incrementData();
  }
}

/* Leave in display task or move to precharge, don't move to CAN. *
 * There is no CAN bus functionality here.                        */
float aux_voltage_read() {
  float aux_voltage = 3.3 * analogRead(13) / 1024.0;
  aux_voltage *= 42.0 / 10.0;
  return  aux_voltage;
}
