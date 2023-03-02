/**
 * @file Main.ino
 *   @author    Washington Superbike
 *   @date      1-March-2023
 *   @brief
 *        The main file for bike firmware. This initializes
 *        all variables that are passed along to all other files as
 *        pointers. Then it runs the setup methods for all those
 *        files and then it sets up RTOS to run all the different files
 *        as individual tasks. These tasks are: datalogging,
 *        display, precharge, CAN, idle. These tasks will be further
 *        described in the documentation for their individual files.
 *
 *  Main. The source for the entire firmware. Utilizes Arduino's framework 
 *  with void setup() and void loop() methods, but only void setup() is used.
 *  Before the void setup, we initialize all the variables used throughout
 *  all the files as a set of static variables. The keyword static is used
 *  because in C++ it means that this variable is going to exist "for the
 *  lifetime of the code". These variables are initialized to 0 and then
 *  the void setup will execute everything and also act as the loop.
 *  That is because the setup calls on the xTaskCreate() method from the
 *  RTOS library and then the vTaskStartScheduler() method which in turn
 *  abstracts everything from us and rotates between all the tasks
 *  updating them and transferring resources as required. As of now,
 *  tasks operate on different resources than each other, thus there is no
 *  need for semaphores, however, if that is changed in the future, there
 *  is an example line of creating a semaphore above the calls to the
 *  xTaskCreate() method. The breakdown of how these methods and variables
 *  work in detail are explained further in the documentation closer for
 *  those methods and variables specifically. Please make sure to read
 *  and understand everything before you make any changes to this firmware.
 */


#include "Main.h" /// Includes the Main.h file which in turn interconnects all separate files together.


/// TODO: As of 3/1/2023, I think this firmware is fully race-ready and meeting
/// all requirements to race. Ideally, I would use HIL and their CAN interface
/// to further test the requirements and ensure proper performance under race-communication
/// circumstances, but alas that project is not done yet.
/// As of now, my goal is to create full documentation for this codebase and train
/// up all members to be able to use it without any trouble.



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
  preChargeData = {bmsStatus, motorTemps, cellVoltages, &motorControllerBatteryVoltage, &angle_X, &angle_Y,
                   &Rate_Roll, &Rate_Pitch, &Rate_Yaw, &Rate_CalibrationRoll, &Rate_CalibrationPitch, &Rate_CalibrationYaw,
                   &Rate_CalibrationNumber, &Acc_X, &Acc_Y, &Acc_Z, &Angle_Roll, &Angle_Pitch, &Kalman_AngleRoll, 
                   &Kalman_UncertaintyAngleRoll, &Kalman_AnglePitch, &Kalman_UncertaintyAnglePitch, Kalman_1DOutput};
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
//  setTime(Teensy3Clock.get());

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
  setupI2C(preChargeData);
  
  // unused but left as a reminder for how you can use it
  spi_mutex = xSemaphoreCreateMutex();

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
