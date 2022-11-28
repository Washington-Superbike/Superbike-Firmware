#include "Display.h"
#include "Main.h"
#include "FreeRTOS_TEENSY4.h"

//change final
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); //the display controller

XPT2046_Touchscreen ts(TS_CS);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

// Just for testing*******
int getMPH;
int getVoltage;
int getTemp;
//************************
bool ERROR_STATUS;
int screenMode;
int w;
int h;
//font width is fontsize*6 pixels


void displayTask(void *msData) {
    while (1) {
        MeasurementScreenData ms = *(MeasurementScreenData *)msData;
        if (get_SPI_control(DISPLAY_UPDATE_TIME_MAX)) {
          drawMeasurementScreen(ms);
          release_SPI_control();
        }
        // no delay task for display as it is the lowest priority task except for idle (which just delays)
        // this will allow us to update the display as fast as possible
    }
}

void drawMeasurementScreen(MeasurementScreenData msData) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
    int fromTop = 10;
    tft.setCursor(85, fromTop);
    tft.print(*msData.mainBatteryVoltage);
    tft.print(", "); tft.print(*msData.motorControllerVoltage);
    tft.setCursor(85, fromTop + 8*2);
    tft.println(*msData.auxiliaryBatteryVoltage);
    tft.setCursor(40, fromTop + 8*4);
    tft.println(*msData.RPM);
    tft.setCursor(85, fromTop + 8*6);
    tft.println(*msData.motorTemp);
    tft.setCursor(100, fromTop + 8*8);
    tft.println(*msData.motorCurrent);
    tft.setCursor(100, fromTop + 8*10);
    tft.println(*msData.errorMessage);
    tft.setCursor(135, fromTop + 8*12);
    String thermistors;
    for(int i=0;i<10;i++){
      thermistors.append((byte)msData.thermistorTemps[i]);
      if(i!=9){
        thermistors.append(", ");
      }
    }

    tft.println(thermistors);
    tft.setCursor(135, fromTop + 8*14);
    tft.println(*msData.chargerCurrent);
    tft.setCursor(135, fromTop + 8*16);
    tft.println(*msData.chargerVoltage);
    tft.setCursor(135, fromTop+8*18);
    tft.println(*msData.bmsStatusFlag, HEX);
    tft.setCursor(135, fromTop+8*20);
    tft.println(*msData.evccVoltage);
    tft.setCursor(135, fromTop+8*22);
    tft.print(month()); tft.print('/');
    tft.print(day()); tft.print('/');
    tft.print(year()); tft.print("  ");
    tft.print(hour()); tft.print(":");
    if(minute() < 10) tft.print('0');
    tft.print(minute()); tft.print(":");
    if(second() < 10) tft.print('0');
    tft.println(second());
}


void setupDisplay(Screen screen) {
    tft.begin();

    tft.setRotation(1);

    tft.fillScreen(ILI9341_WHITE);

    // eep touchscreen not found?
    if (!ts.begin()) {
        Serial.println("Couldn't start touchscreen controller");
        //while (1);
    }
    ts.setRotation(1);
    screenMode = 0;

    h = tft.height();
    w = tft.width();
    screen.recentlyChanged = true;

    setupMeasurementScreen(measurementData);
}

void setupMeasurementScreen(MeasurementScreenData msData) {
    tft.setCursor(0, 10);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    String measurementNames = " Main Batt V: \n\n Aux Batt V: \n\n RPM: \n\n Motor Temp: \n\n Motor Current: \n\n Error Message: \n\n Thermistor Temps:\n\n Charger Voltage:\n\n Charger Current\n\nBMS Status Flag: \n\n EVCC Voltage:\n\n Time: ";
    tft.print(measurementNames);
}
