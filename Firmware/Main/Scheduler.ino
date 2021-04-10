#include "Scheduler.h"

IntervalTimer fastTimer;
IntervalTimer slowTimer;
volatile signed char fastTimerFlag;
volatile signed char slowTimerFlag;
volatile unsigned char fastTimerIncrement;
volatile unsigned char slowTimerIncrement;

void setupFastTimerISR() {
  fastTimerIncrement = 0;
  fastTimer.priority(0);
  fastTimer.begin(raiseFastTimerFlag, 20000); // currently 20 ms
}

void setupSlowTimerISR(PreChargeTaskData preChargeTaskData) {
  slowTimerIncrement = 0;
  *preChargeTaskData.PC_State = PC_START;
  slowTimer.priority(0);
  slowTimer.begin(raiseSlowTimerFlag, 500000); // currently 500 ms
}

void raiseFastTimerFlag() {
  fastTimerFlag = 1;
  fastTimerIncrement += 1;
}

void raiseSlowTimerFlag() {
  slowTimerFlag = 1;
  slowTimerIncrement += 1;
}
