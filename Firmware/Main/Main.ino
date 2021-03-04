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
static byte switchSignalsStatus = 0;


static byte displayFlag = 0;
static byte canFlag = 0;
static byte dataLoggingFlag = 0;
static byte saveFlag = 0;

static MeasurementScreenData measurementData = {};
static MotorStats motorStats = {};
static MotorTemps motorTemps = {};
static CellVoltages cellVoltages = {};
static PreChargeTaskData preChargeData = {};
static BMSStatus bmsStatus = {};
static ThermistorTemps thermistorTemps = {};

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
byte ranFlag = 0;

void initializeLogs() {
  motorTemperatureLog = {MOTOR_TEMPERATURE_LOG, 1, &motorTemp};
  motorControllerTemperatureLog = {MOTOR_CONTROLLER_TEMPERATURE_LOG, 1, &motorControllerTemp};
  motorControllerVoltageLog = {MOTOR_CONTROLLER_VOLTAGE_LOG, 1, &motorControllerBatteryVoltage};
  motorCurrentLog = {MOTOR_CURRENT_LOG, 1, &motorCurrent};
  rpmLog = {RPM_LOG, 1, &RPM};
  thermistorLog = {THERMISTOR_LOG, 10, &thTemps[0]};
  bmsVoltageLog = {BMS_VOLTAGE_LOG, 1, &seriesVoltage};
}

void setup() {
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(TS_CS, OUTPUT);
  digitalWrite(TS_CS, HIGH);
  pinMode(16, OUTPUT);
  pinMode(16, LOW);
  measurementData = {&motorControllerBatteryVoltage, &auxiliaryBatteryVoltage, &RPM, &motorTemp, &motorCurrent, &errorMessage};
  motorStats = {&RPM, &motorCurrent, &motorControllerBatteryVoltage, &errorMessage};
  cellVoltages = {&cellVoltagesArr[0]};
  bmsStatus = { &bms_status_flag, &bms_c_id, &bms_c_fault, &ltc_fault, &ltc_count};
  thermistorTemps = {thTemps};
  // initial
  initializeLogs();
  Serial.print("Starting SD: ");
  Serial.println(!startSD());
  Serial.print("Opening motor temp log: ");
  Serial.println(!openFile(&motorTemperatureLog));
  Serial.println("starting program");
  setupCAN();
  setupCANISR();
}

void loop() {
  if (millis() - timer >= 20) {
    cycleCount++;
    ranFlag = 0;
    timer = millis();
  }
  if (!ranFlag) {
    if (cycleCount % 1000 == 0 ) {
      Serial.println("SAVING LOGS");
      saveFiles(logs, 7);
    }
    if (cycleCount % 500 ) {
      //displayTask(measurementData);
    }
    if (checkCANFlag) {
      canTask({motorStats, motorTemps, bmsStatus, thermistorTemps, cellVoltages,  &seriesVoltage});
      checkCANFlag = 0;
    }
    if (requestBMSVoltageFlag) {
      requestCellVoltages(lowerUpperCells);
      lowerUpperCells *= -1;
      requestBMSVoltageFlag = 0;
    }
    if (cycleCount % 50 == 0 ) {
      dataLoggingTask({logs, 7});
      ranFlag = 1;
    }
  }
}
