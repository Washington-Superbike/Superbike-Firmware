

#ifndef PRECHARGE_H_
#define PRECHARGE_H_

#define CONTACTOR 4 //digital pin for contactor control (closing or opening)
#define PRECHARGE 3 //digital pin for relay in series with precharge resistor
#define HIGH_VOLTAGE_TOGGLE 5 // digital pin for starting precharge, exit precharging, exit done-precharging
#define CLOSE_CONTACTOR_BUTTON 6 // digital pin for closing the contactor (usable after precharging)
#define CONTACTOR_PRECHARGED_LED 400 // digital pin for LED that illuminates after precharge complete
#define CONTACTOR_CLOSED_LED 401 // digital pin for LED that illuminates when the contactor is closed 
#define NUMBER_OF_LTCS 2 // THIS NEEDS TO BE CHANGED TO OUR ACTUAL NUMBER OF LTCs
#define MOTORCONTROLLER_TEMP_MAX 65 // THIS ALSO MAY NEED TO BE CHANGED
#define MOTOR_TEMP_MAX 80 

extern volatile signed char preChargeFlag; 
enum PC_STATE { PC_START, PC_OPEN , PC_CLOSE, PC_JUST_CLOSED };

typedef struct PreChargeTaskData {
  float* seriesVoltage; // from the main accumulator
  PC_STATE* PC_State;
  float* motorControllerBatteryVoltage;
};


void tickPreChargeFSM();
void preChargeTask(PreChargeTaskData preChargeData, MotorStats motorStats);
void preChargeCircuitFSMTransitionActions (PreChargeTaskData preChargeData,  BMSStatus bmsStatus, MotorStats motorStats);
void preChargeCircuitFSMStateActions (PreChargeTaskData preChargeData);
void preChargeCheck(PreChargeTaskData preChargeData, MotorStats motorStats);
int checkIfPrecharged(PreChargeTaskData preChargeData);
int closeContactor(PreChargeTaskData preChargeData, BMSStatus bmsStatus, MotorTemps motorTemps);

#endif
