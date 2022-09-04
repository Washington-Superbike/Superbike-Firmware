#include "Precharge.h"

PC_STATE pc_state = PC_START;

void prechargeTask(void *taskData) {
  PreChargeTaskData prechargeData = *(PreChargeTaskData *)taskData;
  while (1) {
    preChargeCircuitFSMStateActions(prechargeData);
    preChargeCircuitFSMTransitions(prechargeData);
    // 1ms should be unnoticeable compared to other task updates
    // but should be fast to pick up errors / switch updates
    vTaskDelay((1 * configTICK_RATE_HZ) / 1000);
  }
}

// NOTE: "input" needs to change to the GPIO value for the on-button for the bike
void preChargeCircuitFSMTransitions (PreChargeTaskData preChargeData) {
  
  switch (pc_state) { // transitions
    case PC_START:
      pc_state = PC_OPEN;
      break;
    case PC_OPEN:
      // when the GPIO for the bike's start switch is known, use: digitalRead(pin)
      
      if ( check_HV_TOGGLE() == 1 ) {            //change 0 to the digital read
        pc_state = PC_CLOSE;
        break;
      }
      break;
    case PC_CLOSE:
      if (check_HV_TOGGLE() == 0) { // kill-switch activated
        pc_state = PC_OPEN;
      }
      else if (isPrecharged(preChargeData) == 0) { // precharge not finished
        pc_state = PC_CLOSE;
        break;
      }
      else if (closeContactor(preChargeData) == 1 && check_CONTACTOR_CLOSE()==1) {
        pc_state = PC_JUST_CLOSED;
        break;
      } // precharge finished, CLOSE_CONTACTOR_BUTTON pushed, no errors
      else { // precharge finished, but CLOSE_CONTACTOR_BUTTON not pushed
        pc_state = PC_CLOSE;
        break;
      }
    case PC_JUST_CLOSED:
      if (check_HV_TOGGLE() == 0 || closeContactor(preChargeData) == 0) { // kill-switch activated or error detected
        pc_state = PC_OPEN;
      }
      else {
        pc_state = PC_JUST_CLOSED;
        break;
      }
    default:
      pc_state = PC_START;
      break;
  } // transitions
}


void preChargeCircuitFSMStateActions (PreChargeTaskData preChargeData) {
  switch (pc_state) { // state actions
    case PC_OPEN:
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(PRECHARGE, LOW);
//      Serial.println("Contactor closed led: low");
      digitalWrite(CONTACTOR_CLOSED_LED, LOW);
      break;
    case PC_CLOSE:
      // requestBMSVoltageISR.update( a faster time);
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(PRECHARGE, HIGH);
//      Serial.println("Contactor closed led: low");
      digitalWrite(CONTACTOR_CLOSED_LED, LOW);
      break;
    case PC_JUST_CLOSED:
      // requestBMSVoltageISR.update( a slower time);
      digitalWrite(CONTACTOR, HIGH);
      digitalWrite(PRECHARGE, LOW);
//      Serial.println("Contactor closed led: high");
      digitalWrite(CONTACTOR_CLOSED_LED, HIGH);
      break;
    default:
      break;
  } // state actions
  if (isPrecharged(preChargeData) == 1) {
//    Serial.println("Contactor precharged led: high");
    digitalWrite(CONTACTOR_PRECHARGED_LED, HIGH); // precharged confirmed
  }
  else {
//    Serial.println("Contactor precharged led: low");
    digitalWrite(CONTACTOR_PRECHARGED_LED, LOW); // not-precharged confirmed
  }
//  Serial.println(pc_state);
}

// This function returns 1 if the difference between the main-accumulator-series-voltage and the
// motorcontroller-voltage is less than 10% of the main-accumulator-series-voltage 
// (and greater than 80V right now but this should be removed later).
// This function returns 0 otherwise.
bool isPrecharged(PreChargeTaskData preChargeData) {
  if (*preChargeData.cellVoltages.ready) {
      return false;
  }
  return ((*preChargeData.cellVoltages.seriesVoltage - *preChargeData.motorControllerBatteryVoltage) <= (*preChargeData.cellVoltages.seriesVoltage * 0.1) && 
      *preChargeData.cellVoltages.seriesVoltage > 80);
}

// this function returns 1 if the contactor is in a good state to be closed
int closeContactor(PreChargeTaskData preChargeData) {
  //BMSStatus bmsStatus = preChargeData.bmsStatus;
  MotorTemps motorTemps = preChargeData.motorTemps;
  
  if (!isPrecharged(preChargeData)) return 0;
  //if (*bmsStatus.ltc_fault == 1) return 0;
  //if (*bmsStatus.ltc_count != NUMBER_OF_LTCS) return 0;
  //if (*bmsStatus.bms_c_fault == 1 || *bmsStatus.bms_c_fault == 2 || *bmsStatus.bms_c_fault == 4 ||    //checks BMS fault error codes
  //  *bmsStatus.bms_c_fault == 8) return 0;
  //if (*bmsStatus.bms_status_flag == 1 || *bmsStatus.bms_status_flag == 2) return 0;  //check if cells are above or below the voltage cutoffs
  if (*motorTemps.motorControllerTemperature >= MOTORCONTROLLER_TEMP_MAX
      || *motorTemps.motorTemperature >= MOTOR_TEMP_MAX)       return 0;
  return 1;
}

bool check_HV_TOGGLE() {
  return !digitalRead(HIGH_VOLTAGE_TOGGLE);
}

bool check_CONTACTOR_CLOSE() {
  return !digitalRead(CLOSE_CONTACTOR_BUTTON);
}
