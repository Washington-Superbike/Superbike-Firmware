#include "CAN.h"
#include "Display.h"
#include "Main.h"
#include "Precharge.h"
#include "DataLogging.h"

int bms_status_flag = 0;
int bms_c_id = 0;
int bms_c_fault = 0;
int ltc_fault = 0;
int ltc_count = 0;
float cellVoltages[BMS_CELLS];         // voltages starting with the first LTC
float thermistorTemp[36];       // assuming a message with 7 LTCs
int thermistorEnabled;          // assuming only 2 LTCs
int thermistorPresent;

float auxiliaryBatteryVoltage = 0;

int RPM = 0;
float motorCurrent = 0; // current coming from motor controller
float motorControllerBatteryVoltage = 0;
float throttle = 0;
float motorControllerTemp = 0;
float motorTemp = 0;
int errorMessage = 0;
byte controllerStatus = 0;
byte switchSignalsStatus = 0;


byte displayFlag = 0;
byte canFlag = 0;

MeasurementScreenData measurementData = {};
MotorStats motorStats = {};
MotorTemps motorTemps = {};
CellsVoltage cellsVoltage = {};
PrechargeValues prechargeValues = {};
BMSStatus bmsStatus = {};

CSVWriter motor_current_log = {};
SdFile myFile = {};
void setup() {
    measurementData = {&motorControllerBatteryVoltage, &auxiliaryBatteryVoltage, &RPM, &motorTemp, &motorCurrent, &errorMessage};
    motorStats = {&RPM, &motorCurrent, &motorControllerBatteryVoltage, &errorMessage};
    cellsVoltage = {&cellVoltages[0]};
    bmsStatus = { &bms_status_flag, &bms_c_id, &bms_c_fault, &ltc_fault, &ltc_count};

    motor_current_log = {MOTOR_CURRENT_LOG, false, 1, {&motorCurrent}};
    if(openFile(motor_current_log)){
        Serial.println("Couldn't open motor_current_log");
    }
}

void loop() {
    if(displayFlag) {
        displayTask(measurementData);
    }
    if(preChargeFlag) {
        //preChargeTask();
    }
    if(canFlag) {
        canTask(motorStats, motorTemps);
    }
}
