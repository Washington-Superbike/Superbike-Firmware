#pragma once      /// Pragma once is the equivalent of a ifndef statement, avoiding creating extra copies of .h files.
#include "CAN.h"
#include "Display.h"
#include "Precharge.h"
#include "DataLogging.h"
#include "FreeRTOS_TEENSY4.h"
#include <TimeLib.h>

static float angle_X = 0.0;
static float angle_Y = 0.0;

static int bms_status_flag = 0;
static int bms_c_id = 0;
static int bms_c_fault = 0;
static int ltc_fault = 0;
static int ltc_count = 0;
static float cellVoltagesArr[BMS_CELLS];  // voltages starting with the first LTC
static float seriesVoltage;
static bool cellsReady;
static float thTemps[10];       // assuming only 10 thermistors
static int thermistorEnabled;
static int thermistorPresent;

static float auxiliaryBatteryVoltage = 0;
static float Rate_Roll = 0.0;
static float Rate_Pitch = 0.0;
static float Rate_Yaw = 0.0;

static float Rate_CalibrationRoll = 0.0;
static float Rate_CalibrationPitch = 0.0;
static float Rate_CalibrationYaw = 0.0;

static int Rate_CalibrationNumber = 0;
static float Acc_X = 0.0;
static float Acc_Y = 0.0;
static float Acc_Z = 0.0;
static float Angle_Roll = 0.0;
static float Angle_Pitch = 0.0;

// Define predicted angles and uncertainties
static float Kalman_AngleRoll= 0.0;
static float Kalman_UncertaintyAngleRoll=2*2;
static float Kalman_AnglePitch=0;
static float Kalman_UncertaintyAnglePitch=2*2;
// Output of filter
static float Kalman_1DOutput[]={0,0};



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

static displayPointer displayTaskWrap = {};

static MeasurementScreenData measurementData = {};
static MotorStats motorStats = {};
static MotorTemps motorTemps = {};
static CellVoltages cellVoltages = {};
static PreChargeTaskData preChargeData = {};
static BMSStatus bmsStatus = {};
static ThermistorTemps thermistorTemps = {};
static ChargerStats chargerStats = {};
static ChargeControllerStats chargeControllerStats = {};

static CANTaskData canTaskData;
static DataLoggingTaskData dataLoggingTaskData;

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

SemaphoreHandle_t spi_mutex;




bool get_SPI_control(void);
