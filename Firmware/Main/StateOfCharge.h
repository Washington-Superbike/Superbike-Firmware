
#ifndef SOC_H_
#define SOC_H_

enum SOC_STATE { SOC_START, SOC_CHARGING, SOC_DISCHARGING };

typedef struct SOCTaskData {
  SOC_STATE* SOC_State;
  float* stateOfCharge;
  int* stateOfChargeMemoryAddress;
};

#endif
