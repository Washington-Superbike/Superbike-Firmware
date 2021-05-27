#ifndef SOC_H_
#define SOC_H_

enum SOC_STATE { SOC_START, SOC_CHARGING, SOC_DISCHARGING, SOC_IDLE };

typedef struct SOCTaskData {
  SOC_STATE* SOC_State;
  float* stateOfCharge;
  int* stateOfChargeMemoryAddress;
  float* dischargingCurrent;
  float* chargingCurrent;
  float* batteryCapacity;
};

#define DISCHARGING_CURRENT_THRESHOLD 9000
#define CHARGING_CURRENT_THRESHOLD 9000
#define SOC_TIME_INTERVAL 9000 // seconds between calls to the SOC state machine
//
void SOC_FSMTransitionActions (SOCTaskData socData);
//void SOC_FSMStateActions (SOCTaskData socData);
//float adjustSOC (SOCTaskData socData, float current);

#endif
