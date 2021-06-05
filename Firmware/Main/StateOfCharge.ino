#include "StateOfCharge.h"

float adjustSOC(SOCTaskData socData, float oldCurrent, float newCurrent, int oldTime, int newTime) {
  return (*socData.stateOfCharge) * simpson38rule(oldTime, newTime, oldCurrent, newCurrent) / (*socData.batteryCapacity) * 1.0;
}

float simpson38rule(int a, int b, double fa, double fb) {
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
  if (*socData.newChargingCurrent > CHARGING_CURRENT_THRESHOLD && *socData.chargingCurrentFlag) {
    *socData.chargingCurrentFlag = 0;
    *socData.SOC_State = SOC_CHARGING; 
  } else if (*socData.newDischargingCurrent > DISCHARGING_CURRENT_THRESHOLD && *socData.disChargingCurrentFlag) {
    *socData.disChargingCurrentFlag = 0;
    *socData.SOC_State = SOC_DISCHARGING;
  } else {
    *socData.SOC_State = SOC_IDLE;
  }
}

void SOC_FSMStateActions (SOCTaskData socData){
  switch (*(socData.SOC_State)) { // state actions\
    case SOC_START:
      break;
    case SOC_CHARGING:
      *socData.stateOfCharge = adjustSOC(socData, *socData.oldCurrent, *socData.newCurrent, *socData.oldCurrentTime, *socData.newCurrentTime);
      *socData.batteryCapacity += simpson38rule(*socData.oldCurrentTime, *socData.newCurrentTime, *socData.oldCurrent, *socData.newCurrent); // 
      break;
    case SOC_DISCHARGING:
      *socData.stateOfCharge = adjustSOC(socData, *socData.oldCurrent, *socData.newCurrent, *socData.oldCurrentTime, *socData.newCurrentTime);
      *socData.batteryCapacity -= simpson38rule(*socData.oldCurrentTime, *socData.newCurrentTime, *socData.oldCurrent, *socData.newCurrent);
      break;
    case SOC_IDLE:
      break;
    default:
      break;
  } // state actions
}

// This is a function that will read all 8 bytes of a double from the EEPROM and return it. 
// function needs checking
double readDoubleFromEEPROM(int startAddress) {
  double readDouble = 0;
  for (int currentByte = startAddress; currentByte < startAddress + 7; currentByte++) {
    readDouble &= EEPROM.read() << currentByte;
  }
  return readDouble;
}

// needs checking
void writeDoubleToEEPROM(int startAddress, double data) {
  byte currentByte;
  for (int currentAddress = startAddress; currentAddress < startAddress + 7; currentAddress++) {
    currentByte = data >> (startAddress + 7 - currentAddress);
    EEPROM.write(currentAddress, currentBtye);
  }
}
