#include "Precharge.h"


PC_STATE PC_State;

// Interrupt service routine for the precharge circuit
void tickPreChargeFSM() {
  preChargeFlag = 1;
}

void preChargeTask(PreChargeTaskData pcData){
    preChargeCheck(pcData);
}

// NOTE: "input" needs to change to the GPIO value for the on-button for the bike
// NOTE: FL mentioned using local variables for the states, consider where to initialize so that the states
// can be passed to the preChargeCircuitFSM function
void preChargeCircuitFSM (PreChargeTaskData pcData)
{
  switch (PC_State) { // transitions
    case PC_START:
      PC_State = PC_OPEN;
      break;
    case PC_OPEN:
      // when the GPIO for the bike's start switch is known, use: digitalRead(pin)
      if ( 0 == 1 ) {            //change 0 to the digital read
        PC_State = PC_CLOSE;
        break;
      }
      break;
    //         else {
    //            PC_State = PC_OPEN;
    //            break;
    //         }
    case PC_CLOSE:
      if ( *(pcData.seriesVoltage) == BMS_DECLARED_VOLTAGE) {    //change to condition: motor controller voltage = BMS declared voltage
        PC_State = PC_CLOSE;
        break;
      }
      else {
        PC_State = PC_JUST_CLOSED;
        break;
      }
    case PC_JUST_CLOSED:
      PC_State = PC_JUST_CLOSED;
      break;
    default:
      PC_State = PC_START;
      break;
  } // transitions

  switch (PC_State) { // state actions
    case PC_OPEN:
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(PRECHARGE, LOW); // SANITY CHECK: DOES THIS OPEN THE PRECHARGE RELAY?
      break;
    case PC_CLOSE:
      // requestBMSVoltageISR.update( a faster time);
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(PRECHARGE, HIGH);
      break;
    case PC_JUST_CLOSED:
      // requestBMSVoltageISR.update( a slower time);
      digitalWrite(CONTACTOR, HIGH); // WHAT DOES THIS LINE DO??? -- THIS MAY NEED CHANGING
      digitalWrite(PRECHARGE, LOW);
      break;
    default:
      break;
  } // state actions
}

void preChargeCheck(PreChargeTaskData pcData) {
  if (preChargeFlag) {
    preChargeCircuitFSM(pcData);
    noInterrupts();
    preChargeFlag = 0;
    interrupts();
  }
}
