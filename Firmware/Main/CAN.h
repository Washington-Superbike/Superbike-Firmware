
#define MOTOR_STATS_MSG 0x0CF11E05                    //motor controller message - CAN
#define MOTOR_TEMPS_MSG 0x0CF11F05                    // motor controller message - CAN
#define EVCC_STATS 0x18e54024                         // Charge controller status (current,volt...)
#define CHARGER_STATS 0x18eb2440                      // Thunderstruck Charger status (current,volt...)
#define DD_BMS_STATUS_IND 0x01dd0001                  // BMS cell data message (overvolt,undervolt...)
#define DD_BMSC_TH_STATUS_IND 0x01df0e00              // themistor values message
#define BMSC1_LTC1_CELLS_04  0x01df0900               // convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC1_CELLS_58  0x01df0a00
#define BMSC1_LTC1_CELLS_912 0x01df0b00
#define BMSC1_LTC2_CELLS_04  0x01df0901               // convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC2_CELLS_58  0x01df0a01
#define BMSC1_LTC2_CELLS_912 0x01df0b01

#define STANDBY 20                                    // standby GPIO pin for the can transceiver, high=standby, low=normal
#define BMS_CELLS 20                                  // the number of cells connected to the main accumulator BMS

#include <FlexCAN_T4.h>
#include "Display.h"

#ifndef _CAN_H_
#define _CAN_H_


typedef struct {
    float* RPM;
    float* motorCurrent;
    float* motorControllerBatteryVoltage;
    int* errorMessage;
} MotorStats;

typedef struct{
    float* throttle;
    float* motorControllerTemperature;
    float* motorTemperature;
    byte* controllerStatus;
} MotorTemps;

typedef struct {
  byte* en; 
  float* chargeVoltage;
  float* chargeCurrent;
} ChargeControllerStats;

typedef struct {
  byte* statusFlag;
  byte* chargeFlag;
  float* outputVoltage;
  float* outputCurrent;
  int8_t* chargerTemp;
} ChargerStats;

typedef struct {
    int* bms_status_flag;
    int* bms_c_id;
    int* bms_c_fault;
    int* ltc_fault;
    int* ltc_count;
} BMSStatus;

typedef struct {
    float *temps;
} ThermistorTemps;

typedef struct {
  float* cellVoltages;
} CellVoltages;

typedef struct {
    MotorStats motorStats;
    MotorTemps motorTemps;
    BMSStatus bmsStatus;
    ThermistorTemps thermistorTemps;
    CellVoltages cellVoltages;
    ChargerStats chargerStats;
    ChargeControllerStats chargeControllerStats;
    float *seriesVoltage;
} CANTaskData;

void canTask(CANTaskData canData);
void checkCAN(CANTaskData canData);
void decodeMotorStats(CAN_message_t msg, MotorStats motorStats);
void decodeMotorTemp(CAN_message_t msg, MeasurementScreenData *measurementData);

#endif // _CAN_H
