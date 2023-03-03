/**
   @file PreCharge.h
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The PreCharge.h config file for the PreCharge/Controls task for the bike's firmware. This defines the variables that are passed along to the PreCharge.ino file and others if they use it. Then it creates the initial reference (there's a proper C programming term for it) for all the methods used in PreCharge.ino. This file also creates two typedef structs that basically allow us to package the data in PreCharge for the task in a nice way. Like all header files, this exists as the skeleton/framework for the .ino or main c file.


    \note
      This file could be way better optimized. Remove all the stupid extra vars in
      the preChargeStruct data and then change the gyro methods to process items
      as parameters and not a bunch of global vars. This MIGHT require increasing
      PRECHARGE_TASK_STACK_SIZE to make sure there are no errors. But wait for an
      error and then fix the value, dont just increase the stack size without thinking
      about it.

    \todo
      Change the preChargeStruct data to use less floats, etc.
      \n \n
      Goal 2.
      \n \n
      Goal 3.
      \n \n
      Final Goal.
*/
#ifndef _PRECHARGE_H_
#define _PRECHARGE_H_

#include "FreeRTOS_TEENSY4.h"

#define PRECHARGE_TASK_STACK_SIZE configMINIMAL_STACK_SIZE + 4096

#define CONTACTOR 16 //digital pin for contactor control (closing or opening)
#define PRECHARGE 17 //digital pin for relay in series with precharge resistor
#define HIGH_VOLTAGE_TOGGLE 15   // digital pin for starting precharge, exit precharging, exit done-precharging
#define CLOSE_CONTACTOR_BUTTON 14 // digital pin for closing the contactor (usable after precharging)
#define CONTACTOR_PRECHARGED_LED 18 // digital pin for LED that illuminates after precharge complete
#define CONTACTOR_CLOSED_LED 19 // digital pin for LED that illuminates when the contactor is closed
#define NUMBER_OF_LTCS 20 // THIS NEEDS TO BE CHANGED TO OUR ACTUAL NUMBER OF LTCs
#define MOTORCONTROLLER_TEMP_MAX 65 // THIS ALSO MAY NEED TO BE CHANGED
#define MOTOR_TEMP_MAX 80

enum HV_STATE {HV_OFF , HV_PRECHARGING, HV_ON, HV_ERROR};

typedef struct {
  BMSStatus bmsStatus;
  MotorTemps motorTemps;
  CellVoltages cellVoltages; // from the main accumulator
  float* motorControllerBatteryVoltage;
  float* angle_X;
  float* angle_Y;
  float* RateRoll;
  float* RatePitch;
  float* RateYaw;

  float* RateCalibrationRoll;
  float* RateCalibrationPitch;
  float* RateCalibrationYaw;

  int* RateCalibrationNumber;
  float* AccX;
  float* AccY;
  float* AccZ;
  float* AngleRoll;
  float* AnglePitch;

  float* KalmanAngleRoll;
  float* KalmanUncertaintyAngleRoll;
  float* KalmanAnglePitch;
  float* KalmanUncertaintyAnglePitch;
  float* Kalman1DOutput;
} PreChargeTaskData;

HV_STATE hv_state = HV_OFF;

void preChargeTask(void *taskData);
void preChargeCircuitFSMTransitions (PreChargeTaskData preChargeData);
void preChargeCircuitFSMStateActions (PreChargeTaskData preChargeData);
bool isPrecharged(PreChargeTaskData preChargeData);
bool isHVSafe(PreChargeTaskData preChargeData);
bool check_HV_TOGGLE();
char* state_name(HV_STATE state);
// I2C Accelerometer/Gyroscope access methods
void setupI2C(PreChargeTaskData preChargeData);
void gyro_signals(PreChargeTaskData preChargeData);
void updateGyroData(PreChargeTaskData preChargeData);
void kalman_1d(float KalmanState, float KalmanUncertainty, float KalmanInput, float KalmanMeasurement, PreChargeTaskData preChargeData);

#endif // _PRECHARGE_H
