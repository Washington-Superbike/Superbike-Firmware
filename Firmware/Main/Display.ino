#include "Display.h"

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


void displayTask(MeasurementScreenData msData, Screen screen) {
    drawMeasurementScreen(msData, screen);
}

void drawMeasurementScreen(MeasurementScreenData msData, Screen screen) {
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
    String measurementNames = " Main Batt V: \n\n Aux Batt V: \n\n RPM: \n\n Motor Temp: \n\n Motor Current: \n\n Error Message: \n\n Thermistor Temps:\n\n Charger Voltage:\n\n Charger Current\n\nBMS Status Flag: \n\n EVCC Voltage:";
    tft.print(measurementNames);
}



//2 is default
void updateMPH(int getMilesPerHour, int x, int y, int setSize, Screen screen) {

    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);


    tft.setCursor(x, y);
    tft.setTextSize(setSize);
    if (screen.recentlyChanged) {
        tft.println("MPH:");
    }
    if (setSize > 2) {
        tft.println();
    }
    int currentH = tft.getCursorY();
    tft.setCursor(x, currentH);
    tft.setTextSize(setSize + 3);

    if (getMPH < 100) {
        tft.print('0');
    }
    if (getMPH < 10) {
        tft.print('0');
    }
    tft.print(getMilesPerHour);

}

void errorScreen(bool error) {
/*
    if (error) {

        while (error) {
            errorDisplay();


            //***FOR TESTING******
            ERROR_STATUS = false;
            error = false;
            //********************
        }

        tft.fillScreen(ILI9341_WHITE);
        tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
        tft.setTextSize(2);
    }
*/
}

void errorDisplay() {
    tft.fillScreen(ILI9341_RED);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_BLACK, ILI9341_RED);
    tft.setCursor(0, 0);
    tft.println("SHIZ IS WRONG!!!");

    tft.println("");
}


void touchButton(Screen screen) {
    if (screen.px >= 2 * w / 3) { //In right third of screen
        //tft.fillScreen(ILI9341_WHITE);
        screenMode = mod(screenMode + 1, 3);
        Serial.println(screenMode);
    } else if (screen.px <= w / 3 && 0 <= screen.px) { //In left third of screen
        //tft.fillScreen(ILI9341_WHITE);
        screenMode = mod(screenMode - 1, 3);
        Serial.println(screenMode);
    }
}

int mod(int x, int m) {
    return (x % m + m) % m;
}


//Won't work with decimals
//mostSigDigit be the power of 10 for greatest digit of the max value\
// ex: 401 -> 4.01 * 10^2 -> mostSigDigit = 2
void updateNumbers(double num, double oldNum, int fontsize, int mostSigDigit, int digits) {
    int space = 6 * fontsize; //pixels
    tft.setTextSize(fontsize);
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);

    double avg = (num + oldNum) / 2;

    if (abs(num - oldNum) / avg > 0.05) { //percent diff of 5% or more

        for (int i =  mostSigDigit; i > mostSigDigit - digits; i--) {

            int oldDigit = (int) oldNum / pow(10, i);
            oldDigit = oldDigit % 10;
            int newDigit = (int) num / pow(10, i);
            newDigit = newDigit % 10;


            if (oldDigit != newDigit) {
                tft.print(newDigit);
            } else {
                tft.setCursor(tft.getCursorX() + space, tft.getCursorY());
            }
        }


    }
}
