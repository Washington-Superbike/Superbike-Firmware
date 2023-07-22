/**
 * Contains all precharge + HV related state machine controls.
 * Estimates gyro angle using Kalman filter.
*/
#include "Precharge.h"
#include <Wire.h>
#include "GPIO.h"

// I2C is incredibly unstable? Or perhaps not using proper wiring causes this,
// but the reading in precharge data can often bug out and output
// "nan" because of randomness? I would personally recommend
// having some sort of true or false based
// indicator at the bottom right of the speedometer screen
// that outputs "true" or something to indicate
// that the gyro is not bugging out. Or perhaps
// output the gyro data on the bottom right
// to indicate danger? Something like that.

// OKAY, MAKE SURE YOU READ THIS IF YOU SEE ISSUES WITH THE GYRO.
// By my *limited* understanding, I think the problem is that
// the gyro needs to be consistently powered, hence why proper
// gyro setup code has a significant delay between
// turning the thing on and actually reading data from it.
// An easy way to work around it, is to power on the Teensy
// and then once everything is up and running,
// reprogram it by using the button on the board.
// In the case of the actual race, I would turn on low-voltage
// and then wait a second and then turn it off and then
// turn it back on.

/* Current HV state */
static HV_STATE hv_state = HV_OFF;

// Returns true if the motor controller is done precharging.
// Returns false otherwise.
bool isPrecharged(PreChargeTaskData preChargeData) {

  BatteryVoltages battery_voltages = preChargeData.context->battery_voltages;
  MotorStats motor_stats = preChargeData.context->motor_stats;

  // Ret false if we haven't received all BMS cell voltages yet
  if (!(battery_voltages.hv_cell_voltages_ready)) {
    return false;
  }

  // Ret true if the difference between the main-accumulator-series-voltage and the
  // motorcontroller-voltage is less than 10% of the main-accumulator-series-voltage
  return ((battery_voltages.hv_series_voltage - motor_stats.motor_controller_battery_voltage) <=
          (battery_voltages.hv_series_voltage * 0.1))
          //and main-accumulator-voltage is greater than 80V (but this should be changed later as the bike voltage may be as low as ~60V).
          && battery_voltages.hv_series_voltage > 80;
}

// this function returns true if there are no HV errors detected on the bike
bool isHVSafe(PreChargeTaskData preChargeData) {
  //BMSStatus bmsStatus = preChargeData.bmsStatus;
  MotorTemps motor_temps = preChargeData.context->motor_temps;

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
  if (motor_temps.motor_controller_temperature >= MOTORCONTROLLER_TEMP_MAX
      || motor_temps.motor_temperature >= MOTOR_TEMP_MAX)       return 0;
  return 1;
}

const char* state_name(HV_STATE state) {
  switch (state) {
    case HV_OFF: return "HV_OFF";
    case HV_PRECHARGING: return "HV_PRECHARGING";
    case HV_ON: return "HV_ON";
    case HV_ERROR: return "HV_ERROR";
    default: return "UNKNOWN_STATE";
  }
}

// NOTE: "input" needs to change to the GPIO value for the on-button for the bike
void preChargeCircuitFSMTransitions (PreChargeTaskData preChargeData) {
  HV_STATE old_state = hv_state;
  GyroKalman *gyro_kalman = &preChargeData.context->gyro_kalman;
  switch (hv_state) { // transitions
    case HV_OFF:
      if (check_HV_toggle()) {
        hv_state = HV_PRECHARGING;
      }
      break;
    case HV_PRECHARGING:
      if (!check_HV_toggle()) {
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
      if (!check_HV_toggle()) {
        // kill-switch activated or HV switch turned off
        hv_state = HV_OFF;
      }
      else if (!isHVSafe(preChargeData) || gyro_kalman->angle_Y > 45 || gyro_kalman->angle_Y < -45 || gyro_kalman->angle_X > 45 || gyro_kalman->angle_X < -45) {
        // HV error detected
        hv_state = HV_ERROR;
      }
      else {
        // no updates, keep HV on
        hv_state = HV_ON;
      }
      break;
    case HV_ERROR:
      if (!check_HV_toggle()) {
        // kill-switch activated or HV switch turned off
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

void preChargeCircuitFSMStateActions () {
  switch (hv_state) { // state actions
    case HV_OFF:
      open_contactor();
      open_precharge();
      break;
    case HV_PRECHARGING:
      open_contactor();
      close_precharge();
      break;
    case HV_ON:
      close_contactor();
      open_precharge();
      break;
    case HV_ERROR:
      open_contactor();
      open_precharge();
    default:
      break;
  } // state actions
}

void gyro_signals(GyroKalman *gyro_kalman) {
  // Start I2C communication with MPU6050
  Wire.beginTransmission(0x68); // 0x68 is default register value for MPU6050

  // Switch on low pass filter
  Wire.write(0x1A); // activate low pass filter
  Wire.write(0x05); // cut off frequency of 10 Hz
  Wire.endTransmission();

  // Configure accelerometer output
  Wire.beginTransmission(0x68);
  Wire.write(0x1C); // 1C is relevant register
  Wire.write(0x10); // full scale range of +/-8g
  Wire.endTransmission();

  // Access registers storing accelerometer measurements
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);

  // Read accelerometer measurements
  int16_t AccXLSB = Wire.read() << 8 | Wire.read(); // x-direction
  int16_t AccYLSB = Wire.read() << 8 | Wire.read(); // y-direction
  int16_t AccZLSB = Wire.read() << 8 | Wire.read(); // z-direction

  // Configure gyroscope output and pull rotation rate measurements from sensor
  // Set sensitivity scale factor
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); // 1B is hexadecimal associated with gyroscope configuration
  Wire.write(0x8); // 8 is hexadecimal for LSB sensitivity of 65.6 LSB/degree/second
  Wire.endTransmission();

  // Access registers storing gyro measurements
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);

  // Read gyro measurements
  int16_t GyroX = Wire.read() << 8 | Wire.read(); // around x-axis
  int16_t GyroY = Wire.read() << 8 | Wire.read(); // around y-axis
  int16_t GyroZ = Wire.read() << 8 | Wire.read(); // around z-axis

  // Convert measurement units to degree/second
  gyro_kalman->RateRoll = (float)GyroX / 65.5;
  gyro_kalman->RatePitch = (float)GyroY / 65.5;
  gyro_kalman->RateYaw = (float)GyroZ / 65.5;

  // Convert measurements to from LSB to g
  // Divide 4096 because full range +/-8g is associated with 4096 LSB/g
  gyro_kalman->AccX = (float)AccXLSB / 4096;
  gyro_kalman->AccY = (float)AccYLSB / 4096;
  gyro_kalman->AccZ = (float)AccZLSB / 4096;

  float AccX = gyro_kalman->AccX;
  float AccY = gyro_kalman->AccY;
  float AccZ = gyro_kalman->AccZ;

  // Calculate absolute angles
  // IMPORTANT LINE OF CODE:
  //    - WE CAN ADD OR SUBTRACT FROM THE ANGLE DEPENDING ON THE PLACEMENT OF GYROSCOPE
  //    - EX: If MPU6050 is laid on port side, add 90 to AnglePitch
  gyro_kalman->AngleRoll = (atan(AccY / sqrt(AccX * AccX + AccZ * AccZ)) * 1 / (3.142 / 180)) + 5;
  gyro_kalman->AnglePitch = (-atan(AccX / sqrt(AccY * AccY + AccZ * AccZ)) * 1 / (3.142 / 180));
}

void kalman_1d(float KalmanState, float KalmanUncertainty, float KalmanInput, float KalmanMeasurement, GyroKalman *gyro_kalman) {
  KalmanState = KalmanState + 0.004 * KalmanInput;
  KalmanUncertainty = KalmanUncertainty + 0.004 * 0.004 * 4 * 4;
  float KalmanGain = KalmanUncertainty * 1 / (1 * KalmanUncertainty + 3 * 3);
  KalmanState = KalmanState + KalmanGain * (KalmanMeasurement - KalmanState);
  KalmanUncertainty = (1 - KalmanGain) * KalmanUncertainty;
  // Output of filter
  gyro_kalman->Kalman1DOutput[0] = KalmanState;
  gyro_kalman->Kalman1DOutput[1] = KalmanUncertainty;
}

void updateGyroData(GyroKalman *gyro_kalman) {
  gyro_signals(gyro_kalman);
  
  gyro_kalman->RateRoll -= gyro_kalman->RateCalibrationRoll;
  gyro_kalman->RatePitch -= gyro_kalman->RateCalibrationPitch;
  gyro_kalman->RateYaw -= gyro_kalman->RateCalibrationYaw;

  // Calculate Roll angle (around x axis)
  kalman_1d(gyro_kalman->angle_X, gyro_kalman->KalmanUncertaintyAngleRoll, gyro_kalman->RateRoll, gyro_kalman->AngleRoll, gyro_kalman);

  // Update Kalman output to angle roll and uncertaintity
  gyro_kalman->angle_X = gyro_kalman->Kalman1DOutput[0];
  gyro_kalman->KalmanUncertaintyAngleRoll = gyro_kalman->Kalman1DOutput[1];

  // Calculate Pitch angle (around y-axis)
  kalman_1d(gyro_kalman->angle_Y, gyro_kalman->KalmanUncertaintyAnglePitch, gyro_kalman->RatePitch, gyro_kalman->AnglePitch, gyro_kalman);

  // Update Kalman output to angle pitch and uncertaintity
  gyro_kalman->angle_Y = gyro_kalman->Kalman1DOutput[0];
  gyro_kalman->KalmanUncertaintyAnglePitch = gyro_kalman->Kalman1DOutput[1];
}

void initI2C(GyroKalman *gyro_kalman) {
  Wire.setClock(400000); // MPU6050 supports up to 400k Hz in specifications
  Wire.begin();
  delay(50); // give delay for device to start

  // Start gyro in power mode
  Wire.beginTransmission(0x68);
  Wire.write(0x6B); // 6B is relevant register
  Wire.write(0x00); // all bits must be 0 to start and continue device
  Wire.endTransmission();
}

void preChargeTask(void *taskData) {
  PreChargeTaskData preChargeData = *(PreChargeTaskData *)taskData;
  GyroKalman *gyro_kalman = &preChargeData.context->gyro_kalman;

  // Perform gyroscope calibration measurements
  // 2000 milliseconds = 2 seconds to add all measured variables to calibration variables
  // This is important because this solves the issue of a non-zero rotation rate when stationary
  for (gyro_kalman->RateCalibrationNumber = 0; gyro_kalman->RateCalibrationNumber < 2000; gyro_kalman->RateCalibrationNumber++) {
    gyro_signals(gyro_kalman);
    gyro_kalman->RateCalibrationRoll += gyro_kalman->RateRoll;
    gyro_kalman->RateCalibrationPitch += gyro_kalman->RatePitch;
    gyro_kalman->RateCalibrationYaw += gyro_kalman->RateYaw;

    // delay for 1ms for each round of all-axis measurements
    vTaskDelay(configTICK_RATE_HZ / 1000);
  }

  // Take average of calibrated rotation rate values from each direction
  gyro_kalman->RateCalibrationRoll /= 2000;
  gyro_kalman->RateCalibrationPitch /= 2000;
  gyro_kalman->RateCalibrationYaw /= 2000;

  while (1) {
    preChargeCircuitFSMStateActions();
    preChargeCircuitFSMTransitions(preChargeData);
    //Serial.println(gyro_kalman->angle_X);
    //Serial.println(gyro_kalman->angle_Y);
    updateGyroData(gyro_kalman);

    // 100 ms should be unnoticeable compared to other task updates
    // but should be fast to pick up errors / switch updates
    vTaskDelay((10 * configTICK_RATE_HZ) / 1000);
  }
}
