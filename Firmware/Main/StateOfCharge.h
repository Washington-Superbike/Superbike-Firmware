
#ifndef SOC_H_
#define SOC_H_

enum SOC_STATE { SOC_START, SOC_CHARGING, SOC_DISCHARGING, SOC_IDLE };

typedef struct SOCTaskData {
  SOC_STATE* SOC_State;
  int* newCurrentFlag;
  int* dischargingCurrentFlag;
  int* chargingCurrentFlag;
  float* stateOfCharge;
  float* newCurrent;
  float* oldCurrent;
  float* batteryCapacity;
  float* newCurrentTime;
  float* oldCurrentTime;
};

#define DISCHARGING_CURRENT_THRESHOLD 9000
#define CHARGING_CURRENT_THRESHOLD 9000
//
void SOC_FSMTransitionActions (SOCTaskData socData);
void SOC_FSMStateActions (SOCTaskData socData);
float adjustSOC (SOCTaskData socData, float current);
double readDoubleFromEEPROM(int startAddress);
void writeDoubleToEEPROM(int startAddress, double data);

#endif
