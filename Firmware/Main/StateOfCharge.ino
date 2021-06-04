#include "StateOfCharge.h"

float adjustSOC(SOCTaskData socData, float oldCurrent, float newCurrent, int oldTime, int newTime) {
  return (*socData.stateOfCharge) * simpson38rule(oldTime, newTime, oldCurrent, newCurrent) / (*socData.batteryCapacity) * 1.0;
}

float simpson38rule(a, b, fa, fb) {
  return ((b - a) / 8) * (fa + 3 * ((2 * a + b) / 3) + 3 * ((a + 2b) / 3) + fb);
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
  if (*socData.newChargingCurrent > CHARGING_CURRENT_THRESHOLD) {
    *socData.SOC_State = SOC_CHARGING; 
  } else if (*socData.newDischargingCurrent > DISCHARGING_CURRENT_THRESHOLD) {
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
      *socData.stateOfCharge = adjustSOC(socData, -1.0, -1.0, -1, -1);
      *socData.batteryCapacity = *socData.batteryCapacity + (*socData.chargingCurrent * SOC_TIME_INTERVAL);
      break;
    case SOC_DISCHARGING:
      *socData.stateOfCharge = adjustSOC(socData,  -1.0, -1.0, -1, -1);
      *socData.batteryCapacity = *socData.batteryCapacity - (*socData.newDischargingCurrent * SOC_TIME_INTERVAL);
      break;
    case SOC_IDLE:
      break;
    default:
      break;
  } // state actions
}
