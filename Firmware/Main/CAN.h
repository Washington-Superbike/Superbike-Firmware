
#define MOTOR_STATS_MSG 0x0CF11E05                        //motor controller message - CAN
#define MOTOR_TEMP_MSG 0x0CF11F05                        // motor controller message - CAN
#define STANDBY 20                                    // standby for can transceiver, high=standby, low=normal
#define DD_BMS_STATUS_IND 0x01dd0001                  // BMS cell data message
#define DD_BMSC_TH_STATUS_IND 0x01df0e00              // themistor message for BMSC
#define BMSC1_LTC1_CELLS_04  0x01df0900               // convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC1_CELLS_58  0x01df0a00
#define BMSC1_LTC1_CELLS_912 0x01df0b00
#define BMSC1_LTC2_CELLS_04  0x01df0901               // convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC2_CELLS_58  0x01df0a01
#define BMSC1_LTC2_CELLS_912 0x01df0b01

#include <FlexCAN_T4.h>


void canTask(MeasurementScreenData *measurementData);
void decodeMotorStats(CAN_message_t msg, MeasurementScreenData *measurementData );
void decodeMotorTemp(CAN_message_t msg, MeasurementScreenData *measurementData);
