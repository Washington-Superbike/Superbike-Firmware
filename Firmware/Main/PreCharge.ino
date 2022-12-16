#include "Precharge.h"


// The state HV_PRECHARGING, HV_ON are badly named.
// The enum should be renamed to HV_STATE
// and instead we should have HV_OFF, HV_ON, HV_ON states
HV_STATE hv_state = HV_OFF;

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
  HV_STATE old_state = hv_state;
  switch (hv_state) { // transitions
    case HV_OFF:
      if (check_HV_TOGGLE()) {
        hv_state = HV_PRECHARGING;
      }
      break;
    case HV_PRECHARGING:
      if (!check_HV_TOGGLE()) {
        // kill-switch activated or HV switch turned off
        hv_state = HV_OFF;
      }
      else if (!isHVSafe(preChargeData)) {
        // HV error detected
        hv_state = HV_ERROR;
      }
      else if (isPrecharged(preChargeData)) {
        // finished precharging
        hv_state = HV_ON;
      }
      else {
        // no updates, keep precharging
        hv_state = HV_PRECHARGING;
      }
      break;
    case HV_ON:
      if (!check_HV_TOGGLE()) {
        // kill-switch activated or HV switch turned off
        hv_state = HV_OFF;
      }
      else if (!isHVSafe(preChargeData)) {
        // HV error detected
        hv_state = HV_ERROR;
      }
      else {
        // no updates, keep HV on
        hv_state = HV_ON;
      }
      break;
    case HV_ERROR:
      if (isHVSafe(preChargeData)) {
        // if the error has been cleared
        hv_state = HV_OFF;
      } else {
        // otherwise stay here
        hv_state = HV_ERROR;
      }
      break;
    default:
      hv_state = HV_OFF;
      break;
  } // transitions

  if (hv_state != old_state) {
    Serial.printf("HV transitioned from %s to %s state\n", state_name(old_state), state_name(hv_state));
  }
}


void preChargeCircuitFSMStateActions (PreChargeTaskData preChargeData) {
  switch (hv_state) { // state actions
    case HV_OFF:
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(PRECHARGE, LOW);
      break;
    case HV_PRECHARGING:
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(PRECHARGE, HIGH);
      break;
    case HV_ON:
      digitalWrite(CONTACTOR, HIGH);
      digitalWrite(PRECHARGE, LOW);
      break;
    case HV_ERROR:
      digitalWrite(CONTACTOR, LOW);
      digitalWrite(PRECHARGE, LOW);
    default:
      break;
  } // state actions
}

// Returns true if the motor controller is done precharging.
// Returns false otherwise.
bool isPrecharged(PreChargeTaskData preChargeData) {

  // Ret false if we haven't received all BMS cell voltages yet
  if (*preChargeData.cellVoltages.ready) {
    return false;
  }

  // Ret true if the difference between the main-accumulator-series-voltage and the
  // motorcontroller-voltage is less than 10% of the main-accumulator-series-voltage
  return ((*preChargeData.cellVoltages.seriesVoltage - *preChargeData.motorControllerBatteryVoltage) <= (*preChargeData.cellVoltages.seriesVoltage * 0.1) &&
          //and main-accumulator-voltage is greater than 80V (but this should be changed later as the bike voltage may be as low as ~60V).
          *preChargeData.cellVoltages.seriesVoltage > 80);
}

// this function returns true if there are no HV errors detected on the bike

bool isHVSafe(PreChargeTaskData preChargeData) {
  //BMSStatus bmsStatus = preChargeData.bmsStatus;
  MotorTemps motorTemps = preChargeData.motorTemps;

  /* !!!! these are commented out now but all of this should be checked when using the real bike !!!!
    though you will have to determine which of these are emergency HV states. i.e. Which ones should turn off the contactor instantly
    and which ones should you simply alert the rider?
    As of now, they all immediately turn off the contactor which may be dangerous for the rider
  */

  //if (*bmsStatus.ltc_fault == 1) return 0;
  //if (*bmsStatus.ltc_count != NUMBER_OF_LTCS) return 0;
  //    the below if can be reduced to if (*bmsStatus.bms_c_fault) which returns true for any non-zero bms_c_fault value
  //if (*bmsStatus.bms_c_fault == 1 || *bmsStatus.bms_c_fault == 2 || *bmsStatus.bms_c_fault == 4 ||    //checks BMS fault error codes
  //    *bmsStatus.bms_c_fault == 8) return 0;
  //if (*bmsStatus.bms_status_flag == 1 || *bmsStatus.bms_status_flag == 2) return 0;  //check if cells are above or below the voltage cutoffs
  if (*motorTemps.motorControllerTemperature >= MOTORCONTROLLER_TEMP_MAX
      || *motorTemps.motorTemperature >= MOTOR_TEMP_MAX)       return 0;
  return 1;
}

bool check_HV_TOGGLE() {
  return !digitalRead(HIGH_VOLTAGE_TOGGLE);
}

char* state_name(HV_STATE state) {
  switch (state) {
    case HV_OFF: return "HV_OFF";
    case HV_PRECHARGING: return "HV_PRECHARGING";
    case HV_ON: return "HV_ON";
    case HV_ERROR: return "HV_ERROR";
  }
}
