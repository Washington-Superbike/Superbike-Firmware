
#define MOTOR_STATS_MSG 0x0CF11E05                        //motor controller message - CAN
#define MOTOR_TEMPS_MSG 0x0CF11F05                        // motor controller message - CAN
#define STANDBY 20                                    // standby for can transceiver, high=standby, low=normal
#define DD_BMS_STATUS_IND 0x01dd0001                  // BMS cell data message
#define DD_BMSC_TH_STATUS_IND 0x01df0e00              // themistor message for BMSC
#define BMSC1_LTC1_CELLS_04  0x01df0900               // convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC1_CELLS_58  0x01df0a00
#define BMSC1_LTC1_CELLS_912 0x01df0b00
#define BMSC1_LTC2_CELLS_04  0x01df0901               // convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC2_CELLS_58  0x01df0a01
#define BMSC1_LTC2_CELLS_912 0x01df0b01
#define BMS_CELLS 24 // the number of cells connected to the main accumulator BMS
#define EVCC_STATS 0x18e54024
#define CHARGER_STATS 0x18eb2440 

#include <circular_buffer.h>
#include <FlexCAN_T4.h>
#include <imxrt_flexcan.h>
//#include <isotp.h>
//#include <isotp_server.h>
#include <kinetis_flexcan.h>

#include "Display.h"

#ifndef CAN_H_
#define CAN_H_


typedef struct MotorStats{
    float* RPM;
    float* motorCurrent;
    float* motorControllerBatteryVoltage;
    int* errorMessage;
};

typedef struct MotorTemps{
    float* throttle;
    float* motorControllerTemperature;
    float* motorTemperature;
    byte* controllerStatus;
};

typedef struct ChargeControllerStats{
  byte* en; 
  float* chargeVoltage;
  float* chargeCurrent;
};

typedef struct ChargerStats{
  byte* statusFlag;
  byte* chargeFlag;
  float* outputVoltage;
  float* outputCurrent;
  int8_t* chargerTemp;
};

typedef struct BMSStatus {
    int* bms_status_flag;
    int* bms_c_id;
    int* bms_c_fault;
    int* ltc_fault;
    int* ltc_count;
};

typedef struct ThermistorTemps {
    float *temps;
};

typedef struct CellVoltages{
  float* cellVoltages;
};

typedef struct CANTaskData{
    MotorStats motorStats;
    MotorTemps motorTemps;
    BMSStatus bmsStatus;
    ThermistorTemps thermistorTemps;
    CellVoltages cellVoltages;
    ChargerStats chargerStats;
    ChargeControllerStats chargeControllerStats;
    float *seriesVoltage;
};

void canTask(CANTaskData canData);
void checkCAN(CANTaskData canData);
void decodeMotorStats(CAN_message_t msg, MotorStats motorStats);
void decodeMotorTemp(CAN_message_t msg, MeasurementScreenData *measurementData);

#endif
