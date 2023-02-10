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
    //  TODO: add another && next to the !check_HV_TOGGLE, that essentially checks the measured angle > 45 degrees on left or right side (+- 45 degrees?)
      if (!check_HV_TOGGLE() || (initialAngle_Y > 45) || (initialAngle_Y < -45)) {
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

// TODO: add a method that reads in gyro angle data again, which is then compared to initial angle, to determine leaning.

// 2 main variables are updated to determine angles of bike: initialAngle_X and initialAngle_Y
void getGyroAngles() {
  //Extract data
  gyro_signals();
  //Calculate rotation rates
  RateRoll-=RateCalibrationRoll;
  RatePitch-=RateCalibrationPitch;
  RateYaw-=RateCalibrationYaw;
  // Calculate Roll angle (around x axis)
  kalman_1d(initialAngle_X, KalmanUncertaintyAngleRoll, RateRoll, AngleRoll);
  // Update Kalman output to angle roll and uncertaintity
  initialAngle_X=Kalman1DOutput[0]; 
  KalmanUncertaintyAngleRoll=Kalman1DOutput[1];
  // Calculate Pitch angle (around y-axis)
  kalman_1d(initialAngle_Y, KalmanUncertaintyAnglePitch, RatePitch, AnglePitch);
  // Update Kalman output to angle pitch and uncertaintity
  initialAngle_Y=Kalman1DOutput[0]; 
  KalmanUncertaintyAnglePitch=Kalman1DOutput[1];
}

// Method 1 of 2 for gyroscope

// Method to gather/extract data from MPU6050 (gyroscope)
void gyro_signals(void) {
  // Start I2C communication with MPU6050
  Wire.beginTransmission(0x68);
  
  // Switch on low pass filter
  Wire.write(0x1A);
  Wire.write(0x05);
  Wire.endTransmission();

  // Configure accelerometer output
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission();

  // Take accelerometer measurements from sensor
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(); 
  Wire.requestFrom(0x68,6);
  int16_t AccXLSB = Wire.read() << 8 | Wire.read();
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();

  // Configure gyroscope output and pull rotation rate measurements from sensor
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); 
  Wire.write(0x8);
  Wire.endTransmission();     
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  int16_t GyroX=Wire.read()<<8 | Wire.read();
  int16_t GyroY=Wire.read()<<8 | Wire.read();
  int16_t GyroZ=Wire.read()<<8 | Wire.read();
  RateRoll=(float)GyroX/65.5;
  RatePitch=(float)GyroY/65.5;
  RateYaw=(float)GyroZ/65.5;

  // Convert measurements ro physical values
  AccX=(float)AccXLSB/4096;
  AccY=(float)AccYLSB/4096;
  AccZ=(float)AccZLSB/4096;

  // Calculate absolute angles
  // IMPORTANT LINE OF CODE:
  //    - WE CAN ADD OR SUBTRACT FROM THE ANGLE DEPENDING ON THE PLACEMENT OF GYROSCOPE
  //    - EX: If MPU6050 is laid on port side, add 90 to AnglePitch
  AngleRoll= atan(AccY/sqrt(AccX*AccX+AccZ*AccZ))*1/(3.142/180);
  AnglePitch= -atan(AccX/sqrt(AccY*AccY+AccZ*AccZ))*1/(3.142/180);
}

// Method 2 of 2 for gyroscope

// Function calculating angle and uncertainty implementing Kalman filter
// KalmanInput = rotation rate (Use RateRoll or RatePitch as parameter)
// KalmanMeasurement = accelerometer angle (Use AngleRoll or AnglePitch as parameter)
// KalmanState = angle with Kalman filter (Use initialAngle_X or initialAngle_Y as parameter)
void kalman_1d(float KalmanState, float KalmanUncertainty, float KalmanInput, float KalmanMeasurement) {
  KalmanState=KalmanState+0.004*KalmanInput;
  KalmanUncertainty=KalmanUncertainty + 0.004 * 0.004 * 4 * 4;
  float KalmanGain=KalmanUncertainty * 1/(1*KalmanUncertainty + 3 * 3);
  KalmanState=KalmanState+KalmanGain * (KalmanMeasurement-KalmanState);
  KalmanUncertainty=(1-KalmanGain) * KalmanUncertainty;
  // Output of filter
  Kalman1DOutput[0]=KalmanState; 
  Kalman1DOutput[1]=KalmanUncertainty;
}

// Communication with gyroscope and calibration
void setupGyroscope() {
  Serial.begin(57600);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Wire.setClock(400000);
  Wire.begin();
  delay(250);
  Wire.beginTransmission(0x68); 
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  // gyroscope calibration
  for (RateCalibrationNumber=0; RateCalibrationNumber<2000; RateCalibrationNumber ++) {
    gyro_signals();
    RateCalibrationRoll+=RateRoll;
    RateCalibrationPitch+=RatePitch;
    RateCalibrationYaw+=RateYaw;
    delay(1);
  }
  RateCalibrationRoll/=2000;
  RateCalibrationPitch/=2000;
  RateCalibrationYaw/=2000;
  LoopTimer=micros();
}
