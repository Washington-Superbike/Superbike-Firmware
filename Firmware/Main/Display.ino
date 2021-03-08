#include "Display.h"

//change final
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); //the display controller

XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

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



IntervalTimer updateDisplayTimer;
volatile signed char updateDisplayFlag;
void updateDisplayISR() {
    updateDisplayFlag = 1;
}

void displayTask(MeasurementScreenData msData, Screen screen) {
    drawMeasurementScreen(msData, screen);
}

void drawMeasurementScreen(MeasurementScreenData msData, Screen screen) {
    //makeBattery("name", voltage, maxVoltage, startX, startY, width, height);
    makeBattery("Battery", *msData.mainBatteryVoltage, msData.prevMainBattVoltage, 100, w / 16, 0, 0, 0, screen);
    makeBattery("Aux Battery", *msData.auxiliaryBatteryVoltage, msData.prevAuxBattVoltage, 100, w / 2, 0, 0, 0, screen);

    updateTemp(getTemp, screen);

    updateMPH(getMPH, 6 * w / 8, 5 * h / 8, 2, screen);
}


void setupDisplay(Screen screen) {
    tft.begin();

    tft.setRotation(1);

    tft.fillScreen(ILI9341_WHITE);

    // eep touchscreen not found?
    if (!ts.begin()) {
        Serial.println("Couldn't start touchscreen controller");
        while (1);
    }
    ts.setRotation(1);
    screenMode = 0;

    h = tft.height();
    w = tft.width();
    screen.recentlyChanged = true;
}

void makeBattery(String battName, float getVolt, float prevVolt, float maxVolt, int startX, int startY, int width, int height, Screen screen) {

    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);  tft.setTextSize(2);

    if (screen.recentlyChanged) {
        tft.setCursor(startX, startY);
        tft.print(battName + ":");
    }

    if (abs(width) > 0 && abs(height) > 0) {
        tft.fillRect(startX + w / 16, startY + h / 8, width, height, ILI9341_WHITE); //new
        if ((getVolt * 100) / maxVolt > 60) {
            tft.fillRect(startX + w / 16, startY + height + h / 8, width, -height * getVolt / maxVolt, ILI9341_GREEN);
        } else {
            tft.fillRect(startX + w / 16 , startY + height + h / 8, width, -height * getVolt / maxVolt, ILI9341_RED);
        }
        tft.drawRect(startX + w / 16, startY + h / 8, width, height, ILI9341_BLACK);
    }

    tft.setCursor(startX + width + w / 16, startY + height / 2 + h / 8);
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);  tft.setTextSize(2);

    if (getVolt < 100) {
        tft.print('0');
    }
    if (getVolt < 10) {
        tft.print('0');
    }
    tft.print(getVolt);
}

void updateTemp(int getTemperature, Screen screen) {


    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);


    tft.setCursor(w / 16, 5 * h / 8);
    if (screen.recentlyChanged) {
        tft.println("Motor Temp:");
    }
    int currentH = tft.getCursorY();
    tft.setCursor(w / 8, currentH);
    tft.setTextSize(4);

    if (getTemp < 100) {
        tft.print('0');
    }
    if (getTemp < 10) {
        tft.print('0');
    }
    tft.print(getTemperature);
    tft.print(" F");

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

    if (error) {

        while (error) {
            errorDisplay();


            //***FOR TESTING******
            delay(500);
            ERROR_STATUS = false;
            error = false;
            //********************
        }

        tft.fillScreen(ILI9341_WHITE);
        tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
        tft.setTextSize(2);
    }

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
        tft.fillScreen(ILI9341_WHITE);
        screenMode = (screenMode + 1) % 3;
    } else if (screen.px <= w / 3 && 0 <= screen.px) { //In left third of screen
        tft.fillScreen(ILI9341_WHITE);
        screenMode = (screenMode - 1) % 3;
    }
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
