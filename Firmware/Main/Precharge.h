

#ifndef PRECHARGE_H_
#define PRECHARGE_H_

#define CONTACTOR 17 //digital pin for contactor control
#define PRECHARGE 18 //digital pin for relay in series with precharge resistor
#define BMS_DECLARED_VOLTAGE 80 // use 10% difference instead

extern volatile signed char preChargeFlag; 
enum PC_STATE { PC_START, PC_OPEN , PC_CLOSE, PC_JUST_CLOSED };

typedef struct PreChargeTaskData {
  float* seriesVoltage; // from the main accumulator
  PC_STATE* PC_State;
  float* motorControllerBatteryVoltage;
};

void tickPreChargeFSM();
void preChargeTask(PreChargeTaskData preChargeData, MotorStats motorStats);
void preChargeCircuitFSMTransitionActions (PreChargeTaskData preChargeData, MotorStats motorStats);
void preChargeCircuitFSMStateActions (PreChargeTaskData preChargeData);
void preChargeCheck(PreChargeTaskData preChargeData, MotorStats motorStats);

#endif
