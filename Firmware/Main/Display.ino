#include "Display.h"
#include "Main.h"
#include "FreeRTOS_TEENSY4.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); // the display controller

XPT2046_Touchscreen ts(TS_CS); // Param 2 - Touch IRQ Pin - interrupt enabled polling

// TODO: Below = unused, remove?
// Just for testing*******
int getMPH;
int getVoltage;
int getTemp;
//************************
bool ERROR_STATUS;
int screenMode;
int w;
int h;

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
PrintedData *sdDataLost = &printedVals[12];

// Setups the display with
// TODO: DISCUSS screen function:
// Do we truly need a multiple screen setup?
// We only really need the testing screen?
// Unless we want some kind of cool Odometer?
// Potential implementation of screen-based swapping:
// if testingScreen ...
// if drivingScreen ... (load an odometer, etc.)
// font width is fontsize*6 pixels

void setupDisplay(MeasurementScreenData msData, Screen screen)
{
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_WHITE);

  // eep touchscreen not found?
  if (!ts.begin())
  {
    Serial.println("Couldn't start touchscreen controller");
    // while (1);
  }
  ts.setRotation(1);
  screenMode = 0;

  // TODO: Currently unused. Remove?
  h = tft.height();
  w = tft.width();
  screen.recentlyChanged = true;

  *batteryVoltage = {1, 10, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.mainBatteryVoltage, 1, "Main Batt Voltage: "};
  *motorControllerVoltage = {1, 10 + VERTICAL_SCALER * 1, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorControllerVoltage, 1, "Main Batt Voltage (Motor Controller): "};
  *auxBatteryVoltage = {1, 10 + 16 * 2, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.auxiliaryBatteryVoltage, 1, "Aux Batt Voltage: "};
  *rpm = {1, 10 + VERTICAL_SCALER * 3, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.RPM, 1, "RPM: "};
  *motorTemperature = {1, 10 + VERTICAL_SCALER * 4, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorTemp, 1, "Motor Temp: "};
  *motorCurr = {1, 10 + VERTICAL_SCALER * 5, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorCurrent, 1, "Motor Current: "};
  *errMessage = {1, 10 + VERTICAL_SCALER * 6, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, (float *)msData.errorMessage, 1, "Error Message: "};
  *thermTemps = {1, 10 + VERTICAL_SCALER * 7, 90, DEFAULT_FLOAT, ARRAY, (float *)msData.thermistorTemps, 1, "Thermist Temp: "};
  *chargerVolt = {1, 10 + VERTICAL_SCALER * 8, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.chargerVoltage, 1, "Charger Voltage: "};
  *chargerCurr = {1, 10 + VERTICAL_SCALER * 9, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.chargerCurrent, 1, "Charger Current: "};
  *bmsStatusFlag = {1, 10 + VERTICAL_SCALER * 10, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorTemp, 1, "BMS Status Flag: "};
  *evccVolt = {1, 10 + VERTICAL_SCALER * 11, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, msData.motorTemp, 1, "EVCC Voltage: "};
  *sdDataLost = {1, 10 + VERTICAL_SCALER * 12, DEFAULT_X_POS, DEFAULT_FLOAT, NUMBER, (float *)msData.potentialLost, 1, "kb lost: "};
  //    Unsure if this messes with the code, or reduces the potentialLost value beyond being usable

  setupMeasurementScreen();
}

// Update function for display.
// TODO: Is there a point to the whole

// TODO: DISCUSS screen function:
// Again, what point does the Screen serve?
// Do we truly need a multiple screen setup?
// We only really need the testing screen?
// Unless we want some kind of cool Odometer?
// Potential implementation of screen-based swapping:
// if testingScreen ...
// if drivingScreen ... (load an odometer, etc.)
void displayTask(MeasurementScreenData msData, Screen screen)
{
  drawMeasurementScreen(msData, screen);
}

void drawMeasurementScreen(MeasurementScreenData msData, Screen screen)
{
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK);

  for (int i = 0; i < NUM_DATA; i++)
  {
    tft.setCursor(printedVals[i].dataX, printedVals[i].y);

    if (printedVals[i].type == ARRAY)
    {
      screenEraser(17 * 10, i);
      String thermistors;
      for (int j = 0; j < 10; j++)
      {
        thermistors.append((byte)printedVals[i].currData[j]);
        if (j != 9)
        {
          thermistors.append(", ");
        }
      }
      tft.print(thermistors);
    }
    else
    {
      screenEraser(23, i);
      tft.print(*printedVals[i].currData);
    }
  }
}

void setupMeasurementScreen()
{
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK);

  for (int i = 0; i < NUM_DATA; i++)
  {
    tft.setCursor(printedVals[i].labelX, printedVals[i].y);
    tft.print(printedVals[i].labelPtr);
  }
}

void screenEraser(int scaler, int i)
{
  // TODO: get rid of this comment
  // floor(log10(abs(*printedVals[i].currData))) + 1 = formula to return the number of digits in the int.
  // Attempting to scale the clearing rectangle based on int length, except, when int length = 1, it's kinda useless.
  tft.fillRect(printedVals[i].dataX, printedVals[i].y, scaler * (floor(log10(abs(*printedVals[i].currData))) + 1), 8, ILI9341_WHITE);
}
