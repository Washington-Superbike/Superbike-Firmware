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
      NUMBER_OF_LTCS needs to be changed for the REAL number of LTCs. MOTORCONTROLLER_TEMP_MAX might need to be changed depending. MOTOR_TEMP_MAX might need to be changed depending.
      \n \n
      Goal 3.
      \n \n
      Final Goal.
*/
#ifndef _PRECHARGE_H_
#define _PRECHARGE_H_

#include "FreeRTOS_TEENSY4.h"

#define PRECHARGE_TASK_STACK_SIZE configMINIMAL_STACK_SIZE + 4096

/// Teensy pin for contactor control (closing or opening)
#define CONTACTOR 17
/// Teensy pin for relay in series with precharge resistor
#define PRECHARGE 16
/// Teensy pin for starting precharge, exit precharging, exit done-precharging
#define HIGH_VOLTAGE_TOGGLE 24
/// THIS NEEDS TO BE CHANGED TO OUR ACTUAL NUMBER OF LTCs
#define NUMBER_OF_LTCS 20
/// Motor controller temperature max. This might need to be changed
/// depending on how high the MCU hits during operation.
#define MOTORCONTROLLER_TEMP_MAX 65
/// Motor temperature max. This might need to be changed
/// depending on how high the motor hits during operation.
#define MOTOR_TEMP_MAX 80

/// An enum for all the states. OFF, Precharge, ON, Error
enum HV_STATE {HV_OFF , HV_PRECHARGING, HV_ON, HV_ERROR};

/**
 * Just too many things in here. The packaged struct for processing preCharge data
 * This contains the BMS data, the motorData nad the cellVoltages, all good for
 * processing. Then there's about 15 variables for processing gyroscope data.
 * This can be reduced down to two: angle_X and angle_Y.
 */
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

/// The state storage variable. Used to keep track of how things are.
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
