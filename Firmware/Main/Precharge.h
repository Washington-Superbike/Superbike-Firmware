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

void preChargeTask(void *taskData);
void preChargeCircuitFSMTransitions (PreChargeTaskData preChargeData);
void preChargeCircuitFSMStateActions (PreChargeTaskData preChargeData);
bool isPrecharged(PreChargeTaskData preChargeData);
int closeContactor(PreChargeTaskData preChargeData);

// I2C Accelerometer/Gyroscope access methods
void setupI2C(PreChargeTaskData preChargeData);
void gyro_signals(PreChargeTaskData preChargeData);
void updateGyroData(PreChargeTaskData preChargeData);
void kalman_1d(float KalmanState, float KalmanUncertainty, float KalmanInput, float KalmanMeasurement, PreChargeTaskData preChargeData);

#endif // _PRECHARGE_H
