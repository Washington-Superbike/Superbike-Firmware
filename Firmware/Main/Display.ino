#include "Display.h"

//change final
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); //the display controller

XPT2046_Touchscreen ts(TS_CS, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

// Just for testing*******
int getMPH;
int getVoltage;
int getTemp;
//************************
bool ERROR_STATUS;
int screen_Mode;
int WIDTH;
int HEIGHT;

void displayTask(MeasurementScreenData msData) {
    drawMeasurementScreen(msData);
}

void drawMeasurementScreen(MeasurementScreenData msData){
    
}


/*
  void setup() {
    Serial.begin(9600);

    tft.begin();

    // Just for testing*******
    getMPH = 0;
    getVoltage = 100;
    getTemp = 90;
    ERROR_STATUS = false; //*****Make True To Test Error Screen********
    //************************


    tft.setRotation(1);

    tft.fillScreen(ILI9341_WHITE);

    // eep touchscreen not found?
    if (!ts.begin()) {
      Serial.println("Couldn't start touchscreen controller");
      while (1);
    }
    ts.setRotation(1);
    screen_Mode = 0;

    HEIGHT = tft.height();
    WIDTH = tft.width();



  }


  void loop() {
  int h = HEIGHT;
  int w = WIDTH;




  errorScreen(ERROR_STATUS);

  //Just for testing**********
  if (getVoltage > 0) {
    getMPH = getMPH + 10;
    getVoltage = getVoltage - 10;
    getTemp = getTemp + 10;

  } else {
    getMPH = 0;
    getVoltage = 100;
    getTemp = 90;
  }
  //**************************

    if (ts.tirqTouched()) {
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      touchButton(p);
    }
  }



  if(screen_Mode == 0) {
     //makeBattery("name", voltage, maxVoltage, startX, startY, width, height);
     makeBattery("Battery", getVoltage, 100, w/16, 0, 3*w/16, h/4);
     makeBattery("Aux Battery", getVoltage, 100, w/2, 0, 3*w/16, h/4);


     updateTemp(getTemp);

     updateMPH(getMPH, 6*w/8, 5*h/8);
   }else if(screen_Mode == 1) {
      updateMPH(getMPH, 0, 0);
   }else {
      errorDisplay();
   }

   delay(70);


  }

*/


void makeBattery(String battName, int getVolt, int maxVolt, int startX, int startY, int width, int height) {

    int w = WIDTH;
    int h = HEIGHT;

    tft.setCursor(startX, startY);
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print(battName + ":");

    tft.fillRect(startX + w / 16, startY + h / 8, width, height, ILI9341_WHITE); //new
    if ((getVolt * 100) / maxVolt > 60) {
        tft.fillRect(startX + w / 16, startY + height + h / 8, width, -height * getVolt / maxVolt, ILI9341_GREEN);
    } else {
        tft.fillRect(startX + w / 16 , startY + height + h / 8, width, -height * getVolt / maxVolt, ILI9341_RED);
    }
    tft.drawRect(startX + w / 16, startY + h / 8, width, height, ILI9341_BLACK);
    tft.setCursor(startX + width + w / 16, startY + height / 2);
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
    tft.setTextSize(2);

    if (getVolt < 100) {
        tft.print('0');
    }
    if (getVolt < 10) {
        tft.print('0');
    }
    tft.print(getVolt);
}

void updateTemp(int getTemperature) {

    int w = WIDTH;
    int h = HEIGHT;

    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);


    tft.setCursor(w / 16, 5 * h / 8);
    tft.println("Motor Temp:");
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

void updateMPH(int getMilesPerHour, int x, int y) {

    int w = WIDTH;
    int h = HEIGHT;

    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);


    tft.setCursor(x, y);
    tft.setTextSize(2);
    tft.println("MPH:");
    int currentH = tft.getCursorY();
    tft.setCursor(x, currentH);
    tft.setTextSize(4);

    if (getMPH < 100) {
        tft.print('0');
    }
    if (getMPH < 10) {
        tft.print('0');
    }
    tft.print(getMilesPerHour);

}

void errorScreen(bool error) {

    int w = WIDTH;
    int h = HEIGHT;

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

}

void errorDisplay() {
    tft.fillScreen(ILI9341_RED);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_BLACK, ILI9341_RED);
    tft.setCursor(0, 0);
    tft.println("SHIZ IS WRONG!!!");

    tft.println("");
}


String touchButton(TS_Point p) {

    p.x = map(p.x, TS_MAXX, TS_MINX, 0, tft.width());
    p.y = map(p.y, TS_MAXY, TS_MINY, 0, tft.height());

    if (p.x >= 2 * WIDTH / 3) { //In right third of screen
        while (ts.touched()) {
            //loop - only acts when it's released
        }
        tft.fillScreen(ILI9341_WHITE);
        screen_Mode = (screen_Mode + 1) % 3;
    } else if (p.x <= WIDTH / 3 && 0 <= p.x) { //In left third of screen
        tft.fillScreen(ILI9341_WHITE);
        while (ts.touched()) {
            //loop - only acts when it's released
        }
        tft.fillScreen(ILI9341_WHITE);
        screen_Mode = (screen_Mode - 1) % 3;
    }
  // REMOVE THE FOLLOWING LINE
  return "";
}
