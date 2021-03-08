#include "Scheduler.h"

IntervalTimer preChargeFSMTimer; //object for the  precharge IntervalTimer interrupt, and flag variables
volatile signed char preChargeFlag ;     // needs to be volatile to avoid interrupt-related memory issues

void setupPreChargeISR(PreChargeTaskData pcData) {
  PC_State = PC_START;
  // start the prechargeFSM Timer, call ISR every 1 ms
  preChargeFSMTimer.priority(0); // highest priority
  preChargeFSMTimer.begin(tickPreChargeFSM, 1000);
}

IntervalTimer updateDisplayTimer;
volatile signed char updateDisplayFlag;
void updateDisplayISR() {
    updateDisplayFlag = 1;
}

void setupDisplayISR() {
  updateDisplayTimer.priority(0); // highest priority
  updateDisplayTimer.begin(updateDisplayISR, 1000000); // 1000 microseconds subject to change
}

IntervalTimer checkCANTimer;
volatile signed char checkCANFlag;
void checkCANisr() {
    checkCANFlag = 1;
}

IntervalTimer requestBMSVoltageTimer;
volatile signed char requestBMSVoltageFlag;
void requestBMSVoltageISR() {
    requestBMSVoltageFlag  = 1;
}

void setupCANISR() {
  requestBMSVoltageTimer.priority(1); // highest priority
  requestBMSVoltageTimer.begin(requestBMSVoltageISR, 2000000);
  checkCANTimer.priority(0); // highest priority
  checkCANTimer.begin(checkCANisr, 20000);
}
