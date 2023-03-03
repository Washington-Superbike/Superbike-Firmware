/**
   @file CAN.h
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The CAN.h config file for CAN bus for the bike's firmware. This initializes
          all variables that are passed along to all other files as
          pointers. Then it runs the setup methods for all those
          files and then it sets up RTOS to run all the different files
          as individual tasks. These tasks are: datalogging,
          display, precharge, CAN, idle. These tasks will be further
          described in the documentation for their individual files.


    \note
      up all members to be able to use it without any trouble.

    \todo
      Goal 1.
      \n \n
      Goal 2.
      \n \n
      Goal 3.
      \n \n
      Final Goal.
*/
#ifndef _CAN_H_
#define _CAN_H_
#include <FlexCAN_T4.h>
#include "Display.h"
#include "FreeRTOS_TEENSY4.h"


#define CAN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE + 4096

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

// this is set to 24 instead of 20 because the BMS sends cells in packs of 12
// so it makes the decipher function simpler
#define BMS_CELLS 24                                  // the number of cells connected to the main accumulator BMS

// CAN bus handle
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN_bus;

// used to format reading/writing on the CAN bus
CAN_message_t CAN_msg;

// if cellVoltagesReady[INDEX] is true, we have received that cell's voltage from the BMS
// it is false otherwise (so we know when we have collected all distinct cell voltages
static bool cellVoltagesReady[BMS_CELLS] = {false};


typedef struct {
  float* RPM;
  float* motorCurrent;
  float* motorControllerBatteryVoltage;
  int* errorMessage;
} MotorStats;

typedef struct {
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
  float* seriesVoltage;
  bool* ready;
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

void canTask(void *canData);
void setupCAN();
void decipherEVCCStats(CAN_message_t msg, ChargeControllerStats evccStats);
void decipherChargerStats(CAN_message_t msg, ChargerStats chargerStats);
void decodeMotorStats(CAN_message_t msg, MotorStats motorStats );
void decodeMotorTemps(CAN_message_t msg, MotorTemps motorTemps);
void decipherBMSStatus(CAN_message_t msg, BMSStatus bmsStatus);
void decipherCellsVoltage(CAN_message_t msg, CellVoltages cellVoltages);
void decipherThermistors(CAN_message_t msg, ThermistorTemps thermistorTemps);
void calculateSeriesVoltage(CellVoltages cellVs);
void checkCAN(CANTaskData canData);
void printBMSStatus();
void printMessage(CAN_message_t msg);
void requestCellVoltages(int LTC);

#endif // _CAN_H
