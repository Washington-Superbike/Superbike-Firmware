#include "Precharge.h"


// PC_STATE PC_State;

// Interrupt service routine for the precharge circuit
void tickPreChargeFSM() {
  preChargeFlag = 1;
}


// NOTE: "input" needs to change to the GPIO value for the on-button for the bike
// NOTE: FL mentioned using local variables for the states, consider where to initialize so that the states
// can be passed to the preChargeCircuitFSM function
void preChargeCircuitFSMTransitionActions (PreChargeTaskData preChargeData, BMSStatus bmsStatus, MotorTemps motorTemps){
  switch (*(preChargeData.PC_State)) { // transitions
    case PC_START:
      *preChargeData.PC_State = PC_OPEN;
      break;
    case PC_OPEN:
      // when the GPIO for the bike's start switch is known, use: digitalRead(pin)
      if ( digitalRead(HIGH_VOLTAGE_TOGGLE) == 1 ) {            //change 0 to the digital read
        *preChargeData.PC_State = PC_CLOSE;
        break;
      }
      break;
    case PC_CLOSE:
      if (digitalRead(HIGH_VOLTAGE_TOGGLE) == 0) { // kill-switch activated
        *preChargeData.PC_State = PC_OPEN;
      }
      else if (checkIfPrecharged(preChargeData) == 0) { // precharge not finished 
        *preChargeData.PC_State = PC_CLOSE;
        break;
      }
      else if (closeContactor(preChargeData, bmsStatus, motorTemps) == 1 && digitalRead(CLOSE_CONTACTOR_BUTTON) == 1) { 
        *preChargeData.PC_State = PC_JUST_CLOSED;
        break;
      } // precharge finished, CLOSE_CONTACTOR_BUTTON pushed, no errors
      else { // precharge finished, but CLOSE_CONTACTOR_BUTTON not pushed
        *preChargeData.PC_State = PC_CLOSE;
        break;
      }
    case PC_JUST_CLOSED:
      if (digitalRead(HIGH_VOLTAGE_TOGGLE) == 0 || closeContactor(preChargeData, bmsStatus, motorTemps) == 0) { // kill-switch activated or error detected
        *preChargeData.PC_State = PC_OPEN;
      }
      else {
        *preChargeData.PC_State = PC_JUST_CLOSED;
        break;
      }
    default:
      *preChargeData.PC_State = PC_START;
      break;
  } // transitions
}


void preChargeCircuitFSMStateActions (PreChargeTaskData preChargeData){
  switch (*(preChargeData.PC_State)) { // state actions
    case PC_OPEN:
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(PRECHARGE, LOW); 
      digitalWrite(CONTACTOR_CLOSED_LED, LOW);
      break;
    case PC_CLOSE:
      // requestBMSVoltageISR.update( a faster time);
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(PRECHARGE, HIGH);
      digitalWrite(CONTACTOR_CLOSED_LED, LOW);
      break;
    case PC_JUST_CLOSED:
      // requestBMSVoltageISR.update( a slower time);
      digitalWrite(CONTACTOR, HIGH); 
      digitalWrite(PRECHARGE, LOW);
      digitalWrite(CONTACTOR_CLOSED_LED, HIGH);
      break;
    default:
      break;
  } // state actions
  if (checkIfPrecharged(preChargeData) == 1) {
    digitalWrite(CONTACTOR_PRECHARGED_LED, HIGH); // precharged confirmed
  }
  else {
    digitalWrite(CONTACTOR_PRECHARGED_LED, LOW); // not-precharged confirmed
  }
}


// This function returns 1 if the difference between the main-accumulator-series-voltage and the 
// motorcontroller-voltage is less than 10% of the main-accumulator-series-voltage. This function 
// returns 0 otherwise.
// THIS MAY WORK -- CONDSIDER ADDING A WAY TO CHECK IF WE HAVE VOLTAGE DATA FOR ALL CELLLS
int checkIfPrecharged(PreChargeTaskData preChargeData) {
  return ((*preChargeData.seriesVoltage - *preChargeData.motorControllerBatteryVoltage) <= (*preChargeData.seriesVoltage * 0.1)); 
}

// this function returns 1 if the contactor may be closed. This function returns 0 if the 
// contactor must be opened.
int closeContactor(PreChargeTaskData preChargeData, BMSStatus bmsStatus, MotorTemps motorTemps) {
  if (!checkIfPrecharged(preChargeData)) return 0;
  //if (*bmsStatus.ltc_fault == 1) return 0;
  //if (*bmsStatus.ltc_count != NUMBER_OF_LTCS) return 0;
  //if (*bmsStatus.bms_c_fault == 1 || *bmsStatus.bms_c_fault == 2 || *bmsStatus.bms_c_fault == 4 ||    //checks BMS fault error codes 
    //  *bmsStatus.bms_c_fault == 8) return 0;
  //if (*bmsStatus.bms_status_flag == 1 || *bmsStatus.bms_status_flag == 2) return 0;  //check if cells are above or below the voltage cutoffs
  if (*motorTemps.motorControllerTemperature >= MOTORCONTROLLER_TEMP_MAX
      || *motorTemps.motorTemperature >= MOTOR_TEMP_MAX)       return 0;
  return 1;
}
