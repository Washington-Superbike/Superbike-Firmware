
#ifndef SCHED_H_
#define SCHED_H_

extern volatile signed char fastTimerFlag;
extern volatile signed char slowTimerFlag;
extern volatile unsigned char fastTimerIncrement;
extern volatile unsigned char slowTimerIncrement;

void setupFastTimerISR();
void setupSlowTimerISR(PreChargeTaskData preChargeTaskData);
void raiseFastTimerFlag();
void raiseSlowTimerFlag();

#endif
