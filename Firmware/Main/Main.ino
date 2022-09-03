#include "CAN.h"
#include "Display.h"
#include "Main.h"
#include "Precharge.h"
#include "DataLogging.h"
#include "Scheduler.h"

#define DATA_LOG_ENABLE 0

static int bms_status_flag = 0;
static int bms_c_id = 0;
static int bms_c_fault = 0;
static int ltc_fault = 0;
static int ltc_count = 0;
static float cellVoltagesArr[BMS_CELLS];  // voltages starting with the first LTC
static float seriesVoltage;
static float thTemps[10];       // assuming a message with 7 LTCs
static int thermistorEnabled;          // assuming only 2 LTCs
static int thermistorPresent;

static float auxiliaryBatteryVoltage = 0;

static float RPM = 0;
static float motorCurrent = 0;
static float motorControllerBatteryVoltage = 0;
static float throttle = 0;
static float motorControllerTemp = 0;
static float motorTemp = 0;
static int errorMessage = 0;
static byte controllerStatus = 0;

static byte evccEnable = 0;
static float evccVoltage = 0;
static float evccCurrent = 0;

static byte chargeFlag = 0;
static byte chargerStatusFlag = 0;
static float chargerVoltage = 0;
static float chargerCurrent = 0;
static int8_t chargerTemp = 0;

static PC_STATE PC_State; // NEED TO DOUBLE CHECK

static Screen screen = {};

static MeasurementScreenData measurementData = {};
static MotorStats motorStats = {};
static MotorTemps motorTemps = {};
static CellVoltages cellVoltages = {};
static PreChargeTaskData preChargeData = {};
static BMSStatus bmsStatus = {};
static ThermistorTemps thermistorTemps = {};
static ChargerStats chargerStats = {};
static ChargeControllerStats chargeControllerStats = {};


static CSVWriter motorTemperatureLog = {};
static CSVWriter motorControllerTemperatureLog = {};
static CSVWriter motorControllerVoltageLog = {};
static CSVWriter motorCurrentLog = {};
static CSVWriter rpmLog = {};
static CSVWriter thermistorLog = {};
static CSVWriter bmsVoltageLog = {};
static CSVWriter *logs[] = {&motorTemperatureLog, &motorControllerTemperatureLog, &motorControllerVoltageLog, &motorCurrentLog, &rpmLog, &thermistorLog, &bmsVoltageLog};

unsigned long timer = millis();
int cycleCount = 0;

int lowerUpperCells = -1;
unsigned long ms = millis();
byte sdStarted = 0;

void initializeLogs() {
  motorTemperatureLog = {MOTOR_TEMPERATURE_LOG, (uint8_t *)&motorTemp, 1, FLOAT};
  motorControllerTemperatureLog = {MOTOR_CONTROLLER_TEMPERATURE_LOG, (uint8_t *)&motorControllerTemp, 1, FLOAT};
  motorControllerVoltageLog = {MOTOR_CONTROLLER_VOLTAGE_LOG, (uint8_t *)&motorControllerBatteryVoltage, 1, FLOAT};
  motorCurrentLog = {MOTOR_CURRENT_LOG, (uint8_t *)&motorCurrent, 1, FLOAT};
  rpmLog = {RPM_LOG, (uint8_t *)&RPM, 1, FLOAT};
  thermistorLog = {THERMISTOR_LOG, (uint8_t *)&thTemps[0], 10, FLOAT};
  bmsVoltageLog = {BMS_VOLTAGE_LOG, (uint8_t *)&seriesVoltage, 1, FLOAT};
}

void initializeCANStructs() {
  motorStats = {&RPM, &motorCurrent, &motorControllerBatteryVoltage, &errorMessage};
  motorTemps = {&throttle, &motorControllerTemp, &motorTemp, &controllerStatus};
  cellVoltages = {&cellVoltagesArr[0]};
  bmsStatus = { &bms_status_flag, &bms_c_id, &bms_c_fault, &ltc_fault, &ltc_count};
  thermistorTemps = {thTemps};
  chargerStats = {&chargeFlag, &chargerStatusFlag, &chargerVoltage, &chargerCurrent, &chargerTemp};
  chargeControllerStats = {&evccEnable, &evccVoltage, &evccCurrent};
}

void initializePreChargeStruct() {
  preChargeData = {&seriesVoltage, &PC_State, &motorControllerBatteryVoltage};
}

void setup() {
  pinMode(HIGH_VOLTAGE_TOGGLE, INPUT_PULLUP);
  pinMode(CLOSE_CONTACTOR_BUTTON, INPUT_PULLUP);
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(TS_CS, OUTPUT);
  digitalWrite(TS_CS, HIGH);
  pinMode(PRECHARGE, OUTPUT);
  digitalWrite(PRECHARGE, LOW);
  pinMode(CONTACTOR, OUTPUT);
  digitalWrite(CONTACTOR, LOW);
  pinMode(CONTACTOR_PRECHARGED_LED, OUTPUT);
  digitalWrite(CONTACTOR_PRECHARGED_LED, LOW);
  pinMode(CONTACTOR_CLOSED_LED, OUTPUT);
  digitalWrite(CONTACTOR_CLOSED_LED, LOW);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, LOW);
  //motor temp points to motor controller temp for now
  measurementData = {&seriesVoltage, &motorControllerBatteryVoltage, &auxiliaryBatteryVoltage, &RPM, &motorControllerTemp, &motorCurrent, &errorMessage, 
    &chargerVoltage, &chargerCurrent,&bms_status_flag, &evccVoltage,thTemps};
  initializeCANStructs();
  // initial
  initializeLogs();

  Serial.print("Starting SD: ");
  if (startSD()) {
    Serial.println("SD successfully started");
    sdStarted = 1;
  } else {
    sdStarted = 0;
    Serial.println("Error starting SD card");
  }
  setupDisplay(screen);
  setupCAN();
  initializePreChargeStruct();
  setupFastTimerISR();
  setupSlowTimerISR(preChargeData);
}

void loop() {
  if (fastTimerFlag == 1) { // 20 ms interval
    fastTimerFlag = 0;
    canTask({motorStats, motorTemps, bmsStatus, thermistorTemps, cellVoltages, chargerStats, chargeControllerStats, &seriesVoltage});
    if (fastTimerIncrement % 2 == 0) { // 40 ms interval
      preChargeCircuitFSMTransitionActions(preChargeData, bmsStatus, motorTemps);
      preChargeCircuitFSMStateActions(preChargeData);
    }
  }
  if (fastTimerIncrement % 5 == 0 && sdStarted) {// 1 second interval
    dataLoggingTask({logs, 7});
  }
  if (slowTimerFlag == 1) { // 500 ms interval
    //    Serial.println("slow timer flag");
    slowTimerFlag = 0;
    displayTask(measurementData, screen);
    if (slowTimerIncrement % 4 == 0) { // 2 second interval
      requestCellVoltages(lowerUpperCells);
      lowerUpperCells *= -1;
      Serial.println("Requesting cell voltages");
    }
    if (slowTimerIncrement % 20 == 0 && sdStarted) {
      saveFiles(logs, 7);
      Serial.println("saved logging files");
    }
  }
}
