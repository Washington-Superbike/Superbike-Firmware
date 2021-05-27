#include "StateOfCharge.h"

float adjustSOC(SOCTaskData socData, float current) {
  return (*socData.stateOfCharge) * current * SOC_TIME_INTERVAL / (*socData.batteryCapacity) * 1.0;
}

// this function must be called AFTER SOC_FSMStateActions
void SOC_FSMTransitionActions (SOCTaskData socData){
  switch (*(socData.SOC_State)) { // transitions
    case SOC_START:
      break;
    case SOC_CHARGING:
      break;
    case SOC_DISCHARGING:
      break;
    default:
      *socData.SOC_State = SOC_START;
      break;
  } // transitions
  // transition conditions are independant of the current State
  if (*socData.chargingCurrent > CHARGING_CURRENT_THRESHOLD) {
    *socData.SOC_State = SOC_CHARGING; 
  } else if (*socData.dischargingCurrent > DISCHARGING_CURRENT_THRESHOLD) {
    *socData.SOC_State = SOC_DISCHARGING;
  } else {
    *socData.SOC_State = SOC_IDLE;
  }
}

public void SOC_FSMStateActions (SOCTaskData socData){
  switch (*(socData.SOC_State)) { // state actions\
    case SOC_START:
      break;
    case SOC_CHARGING:
      *socData.stateOfCharge = adjustSOC(socData, *socData.chargingCurrent);
      *socData.batteryCapacity = *socData.batteryCapacity - (*socData.chargingCurrent * SOC_TIME_INTERVAL);
      break;
    case SOC_DISCHARGING:
      *socData.stateOfCharge = adjustSOC(socData, *socData.dischargingCurrent);
      *socData.batteryCapacity = *socData.batteryCapacity + (*socData.dischargingCurrent * SOC_TIME_INTERVAL);
      break;
    case SOC_IDLE:
      break;
    default:
      break;
  } // state actions
}
