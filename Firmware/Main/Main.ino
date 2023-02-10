#include "Main.h"
#include "CAN.h"
#include "Display.h"
#include "Precharge.h"
#include "DataLogging.h"
#include "FreeRTOS_TEENSY4.h"
#include <TimeLib.h>


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

static float initialAngle = 0.0;

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

//CHANGE THIS LINE TO SET THE DISPLAY TYPE. TYPE is determine from the display.h SCREENTYPE enum.
static Screen screen = {SPEEDOMETER};
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

void initializeLogs() {
  motorTemperatureLog = {MOTOR_TEMPERATURE_LOG, 1, &motorTemp, FLOAT};
  motorControllerTemperatureLog = {MOTOR_CONTROLLER_TEMPERATURE_LOG, 1, &motorControllerTemp, FLOAT};
  motorControllerVoltageLog = {MOTOR_CONTROLLER_VOLTAGE_LOG, 1, &motorControllerBatteryVoltage, FLOAT};
  motorCurrentLog = {MOTOR_CURRENT_LOG, 1, &motorCurrent, FLOAT};
  rpmLog = {RPM_LOG, 1, &RPM, FLOAT};
  thermistorLog = {THERMISTOR_LOG, 10, &thTemps[0], FLOAT};
  bmsVoltageLog = {BMS_VOLTAGE_LOG, 1, &seriesVoltage, FLOAT};
  dataLoggingTaskData = {logs, 7};
}

void initializeCANStructs() {
  motorStats = {&RPM, &motorCurrent, &motorControllerBatteryVoltage, &errorMessage};
  motorTemps = {&throttle, &motorControllerTemp, &motorTemp, &controllerStatus};
  cellVoltages = {&cellVoltagesArr[0], &seriesVoltage, &cellsReady};
  bmsStatus = { &bms_status_flag, &bms_c_id, &bms_c_fault, &ltc_fault, &ltc_count};
  thermistorTemps = {thTemps};
  chargerStats = {&chargeFlag, &chargerStatusFlag, &chargerVoltage, &chargerCurrent, &chargerTemp};
  chargeControllerStats = {&evccEnable, &evccVoltage, &evccCurrent};
  canTaskData = {motorStats, motorTemps, bmsStatus, thermistorTemps, cellVoltages, chargerStats, chargeControllerStats, &seriesVoltage};
}

void initializePreChargeStruct() {
  preChargeData = {bmsStatus, motorTemps, cellVoltages, &motorControllerBatteryVoltage};
}

void setup() {
  // some of these pins may need to be removed now (contactor/precharge leds and close_contactor button)
  pinMode(HIGH_VOLTAGE_TOGGLE, INPUT_PULLUP);
  pinMode(CLOSE_CONTACTOR_BUTTON, INPUT_PULLUP);
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);
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
  //motor temp points to motor controller temp for now but they are separate on the bike
  measurementData = {&seriesVoltage, &motorControllerBatteryVoltage, &auxiliaryBatteryVoltage, &RPM, &motorControllerTemp, &motorCurrent, &errorMessage,
                     &chargerCurrent, &chargerVoltage, &bms_status_flag, &evccVoltage, thTemps
                    };

  displayTaskWrap = {&measurementData, &screen};
  initializeCANStructs();
  // initial
  initializeLogs();

  Serial.begin(115200);

  // NOTE: to set the Teensy RTC to the real world clock, you need to upload the sketch:
  // File->Examples->Time->TimeTeensy3 and open the serial port. That will set the internal RTC.
  // This function sets the Teensy time from the internal RTC (powered by the coin cell) but
  // does not sync it to the real world clock
  setTime(Teensy3Clock.get());

  // check to see if there is a crash report to print (doesn't work for all crashes)
  //  if (CrashReport) {
  //    Serial.print(CrashReport);
  //    delay(5000);
  //  }

  Serial.print("Starting SD: ");
  if (startSD()) {
    Serial.println("SD successfully started");
    sdStarted = 1;
  } else {
    sdStarted = 0;
    Serial.println("Error starting SD card");
  }

  setupDisplay(measurementData, screen);
  setupCAN();
  initializePreChargeStruct();

//  TODO: add a call to the gyroscope angle measuring method in Precharge.ino, to get the initial angle (straight bike)
//  that will be measured against. This should use the "initialAngle" variable that was initialized at the top of this file.

  // unused but left as a reminder for how you can use it
  spi_mutex = xSemaphoreCreateMutex();

// TODO: change the precharge task's struct to take in the initialAngle variable as well. Ask Yasir what that means if it's confusing.
  portBASE_TYPE s1, s2, s3, s4, s5;
  s1 = xTaskCreate(prechargeTask, "PRECHARGE TASK", PRECHARGE_TASK_STACK_SIZE, (void *)&preChargeData, 5, NULL);
  // make sure to set CAN_NODES in config.h
  s2 = xTaskCreate(canTask, "CAN TASK", CAN_TASK_STACK_SIZE, (void *)&canTaskData, 4, NULL);
  s3 = xTaskCreate(idleTask, "IDLE_TASK", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  s4 = xTaskCreate(displayTask, "DISPLAY TASK", DISPLAY_TASK_STACK_SIZE, (void*)&displayTaskWrap, 2, NULL);
  s5 = xTaskCreate(dataLoggingTask, "DATA LOGGING TASK", DATALOGGING_TASK_STACK_SIZE, (void*)&dataLoggingTaskData, 3, NULL);

  if (s1 != pdPASS || s2 != pdPASS || s3 != pdPASS || s4 != pdPASS || s5 != pdPASS) {
    Serial.println("Error creating tasks");
    while (1);
  }

  Serial.println("Starting the scheduler");
  // start scheduler
  vTaskStartScheduler();
  // should never hit this point unless the scheduler fails
  Serial.println("Insufficient RAM");
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void idleTask(void *taskData) {
  while (1) {
    vTaskDelay((50 * configTICK_RATE_HZ) / 1000);
  }
}


// the SPI mutex is still here even though it is unnecessary. it is leftover as a marker of
// how you could use a mutex in the future if two devices are using the same communication bus
bool get_SPI_control(unsigned int ms) {
  return xSemaphoreTake(spi_mutex, ms);
}

void release_SPI_control(void) {
  xSemaphoreGive(spi_mutex);
}

void loop() {
}
