/**
   @file CAN.h
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The CAN.h config file for the CAN task for the bike's firmware. This defines the variables that are passed along to the CAN.ino file and
          others if they use it. Then it creates the initial reference (there's a proper
          C programming term for it) for all the methods used in CAN.ino. This file also creates multiple typedef structs that basically allow us to package the data that is processed in CAN, nicely. There are a lot of structs and they will be spoken about in detail below. Like all header files, this exists as the skeleton/framework for the .ino or main c file.


    \note
      Frankly, unless you add more CAN devices or if the BMS or any other CAN device acts up, you should have no reason to update or mess with this at all.

    \todo
      Nothing?
      \n \n
      More Nothing?
      \n \n
      Nothing supreme?
      \n \n
      Nothing maxima.
*/
#ifndef _CAN_H_
#define _CAN_H_
#include <FlexCAN_T4.h>
#include "Display.h"
#include "FreeRTOS_TEENSY4.h"

/// Uses the configMINIMAL_STACK_SIZE variable in Main.h to add up to the stack size used for the canTask()
#define CAN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE + 4096

/// Motor controller message - CAN
#define MOTOR_STATS_MSG 0x0CF11E05
/// Motor controller message - CAN
#define MOTOR_TEMPS_MSG 0x0CF11F05
/// Charge controller status (current,volt...)
#define EVCC_STATS 0x18e54024
/// Thunderstruck Charger status (current,volt...)
#define CHARGER_STATS 0x18eb2440
/// BMS cell data message (overvolt,undervolt...)
#define DD_BMS_STATUS_IND 0x01dd0001
/// Themistor values message
#define DD_BMSC_TH_STATUS_IND 0x01df0e00
/// Convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC1_CELLS_04  0x01df0900
/// Convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC1_CELLS_58  0x01df0a00
/// Convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC1_CELLS_912 0x01df0b00
/// Convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC2_CELLS_04  0x01df0901
/// Convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC2_CELLS_58  0x01df0a01
/// Convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC2_CELLS_912 0x01df0b01

/// This is set to 24 instead of 20 because the BMS sends cells in packs of 12
/// so it makes the decipher function simpler. This represents the number of cells
/// connected to the main accumulator BMS
#define BMS_CELLS 24

/// CAN bus handle
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN_bus;

/// Used to format reading/writing on the CAN bus
CAN_message_t CAN_msg;

/// If cellVoltagesReady[INDEX] is true, we have received that cell's voltage from the BMS
/// it is false otherwise (so we know when we have collected all distinct cell voltages
static bool cellVoltagesReady[BMS_CELLS] = {false};

/**
 * A simple struct to store all the stats from the motor. Current, battery voltage,
 * RPM and an errorMessage.
 */
typedef struct {
  float* RPM;
  float* motorCurrent;
  float* motorControllerBatteryVoltage;
  int* errorMessage;
} MotorStats;

/**
 * A simple struct to store all the stats from the motor temperatures? Throttle,
 * motorControllerTemperature, motorTemperature, and controllerStatus.
 */
typedef struct {
  float* throttle;
  float* motorControllerTemperature;
  float* motorTemperature;
  byte* controllerStatus;
} MotorTemps;

/**
 * A simple struct to store all the stats from the chargeController. The charge voltage
 * and charge current.
 */
typedef struct {
  byte* en;
  float* chargeVoltage;
  float* chargeCurrent;
} ChargeControllerStats;

/**
 * A simple struct to store all the stats from the charger. The status flag,
 * the chargeFlag, the outputVoltage, outputCurrent and chargerTemperature.
 */
typedef struct {
  byte* statusFlag;
  byte* chargeFlag;
  float* outputVoltage;
  float* outputCurrent;
  int8_t* chargerTemp;
} ChargerStats;

/**
 * A simple struct to store all the stats from the BMS. The BMS flags,
 * the BMS_id?, the BMS_fault?, the ltc_fault?, the ltc_count? I would
 * read through the datasheets for the bms.
 */
typedef struct {
  int* bms_status_flag;
  int* bms_c_id;
  int* bms_c_fault;
  int* ltc_fault;
  int* ltc_count;
} BMSStatus;

/**
 * A simple struct to store all the stats from the thermistors (just the temperatures
 * really).
 */
typedef struct {
  float *temps;
} ThermistorTemps;

/**
 * A simple struct to store the cellVoltages, the seriesVoltages of the cells and
 * if they're ready?? Unsure.
 */
typedef struct {
  float* cellVoltages;
  float* seriesVoltage;
  bool* ready;
} CellVoltages;

/**
 * A simple struct to store the CanTaskData, a combination
 * of all the previous structs all into this one and then passing it onto the canTask()
 * to be processed by the Can.ino
 */
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
