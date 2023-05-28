/**
    The CAN.h config file for the CAN task for the bike's firmware. Contains definitions for CAN message IDs and relevant function declarations.
*/

#ifndef _CAN_H_
#define _CAN_H_

#include "config.h"
#include "stdint.h"

typedef uint8_t byte;

/// Uses the configMINIMAL_STACK_SIZE variable in Main.h to add up to the stack size used for the canTask()
#define CAN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

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
/* BMS Request Cell Voltages LTC 1 */
#define BMSC1_LTC1_REQUEST_CELLS 0x01de0800
/* BMS Request Cell Voltages LTC 2 */
#define BMSC1_LTC2_REQUEST_CELLS 0x01de0801

/**
 * RPM: RPM reported by motor controller
 * motor_current: current used by motor (should check if this is current used by motor or motor controller, there can be a large difference)
 * motor_controller_battery_voltage: voltage being applied to HV input terminals on motor controller
 * error_message: errors status bits reported by controller 
 */
typedef struct {
  float RPM;
  float motor_current;
  float motor_controller_battery_voltage;
  int error_message;
} MotorStats;

/**
 * throttle: throttle applied (should post range of this value here after testing)
 * motor_controller_temperature: temperature of motor controller in C
 * motor_temperature: temperature of motor in C
 * controller_status: status bits reported by controller (not errors)
 */
typedef struct {
  float throttle;
  float motor_controller_temperature;
  float motor_temperature;
  byte controller_status;
} MotorTemps;

/**
 * en: if charge controller has charging enabled
 * charge_voltage: what voltage the charge controller is telling charger to use
 * charge_current: what current the charge controller is telling charger to use 
 */
typedef struct {
  byte en;
  float charge_voltage;
  float charge_current;
} ChargeControllerStats;

/**
 * status_flag + charge_flag: may have to ask Thunderstruck, not in datasheet
 * but we originally got this CAN struct definition from their engineers
 * output_voltage: voltage applied by charger
 * output_current: current applied by charger
 * charger_temp: charger temperature
 */
typedef struct {
  byte status_flag;
  byte charge_flag;
  float output_voltage;
  float output_current;
  int8_t charger_temp;
} ChargerStats;

/**
 * bms_status_flag: each bit represents an error, check datasheet
 * bms_c_id: cell_id that is reporting a fault
 * bms_c_fault: fault related to cell mentioned above
 * ltc_fault: check datasheet, if ltc is reporting an error
 * ltc_count: how many ltcs are detected, should be 2 for our 20s pack
 */
typedef struct {
  /* need to change bms_status_flag to int after display feature is completed */
  float bms_status_flag;
  int bms_c_id;
  int bms_c_fault;
  int ltc_fault;
  int ltc_count;
} BMSStatus;

/**
 * temps_valid: if temp has been received over CAN by BMS
 * temps: temperature of thermistor reported by BMS
 */
typedef struct {
  bool temps_valid[CONFIG_THERMISTOR_COUNT];
  float temps[CONFIG_THERMISTOR_COUNT];
} ThermistorTemps;

/**
 *  hv_cell_voltages_ready: true if cell voltage has been reported by BMS since boot
 *  hv_cell_voltages: HV cell voltages sent by BMS over CAN
 *  hv_series_voltage: sum of hv_cell_voltages, updated whenever new voltages are received over CAN
 *  aux_battery_voltage: auxiliary (LV) battery voltage
 */
typedef struct {
  bool hv_cell_voltages_ready;
  float hv_cell_voltages[CONFIG_HV_CELL_COUNT];
  float hv_series_voltage;
  float aux_battery_voltage;
} BatteryVoltages;

/**
 * Struct passed to CAN task
 * bike_context: shared pointer to struct representing most information present on bike
 */

#include "Precharge.h"
#include "DataLogging.h"
#include "context.h"

typedef struct {
    Context *bike_context;
} CANTaskData;

void canTask(void *canData);
void initCAN();

#endif // _CAN_H
