#include "CAN.h"
#include "Display.h"
#include "Main.h"

MeasurementScreenData measurementData = {};
MotorStats motorStats = {};

int bms_status_flag = 0;
int bms_c_id = 0;
int bms_c_fault = 0;
int ltc_fault = 0;
int ltc_count = 0;
float cellVoltages[24];         // voltages starting with the first LTC
float thermistorTemp[36];       // assuming a message with 7 LTCs
int thermistorEnabled;          // assuming only 2 LTCs
int thermistorPresent;

float auxiliaryBatteryVoltage = 0;

float RPM = 0;
float motorCurrent = 0; // current coming from motor controller
float motorControllerBatteryVoltage = 0;
float throttle = 0;
float motorControllerTemp = 0;
float motorTemp = 0;
int errorMessage = 0;
byte controllerStatus = 0;
byte switchSignalsStatus = 0;


void setup() {
    measurementData = {&motorControllerBatteryVoltage, &auxiliaryBatteryVoltage, &RPM, &motorTemp, &errorMessage};
    motorStats = {&RPM, &motorCurrent, &motorControllerBatteryVoltage, &errorMessage};
}


void loop() {
    if(displayFlag) {
        displayTask(measurementData);
    }
    if(preChargeFlag) {
        preChargeTask();
    }
    if(canFlag) {
        canTask(motorStats, motorTemps);
    }

}
