/**
   @file Main.ino
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The main file for bike firmware. This includes
          Main.h which in turn interconnects all files
          and allows for access to the global variables.
          These variables are then passed along to all other files as
          pointers. Then it runs the setup methods for all those
          files and then it sets up RTOS to run all the different files
          as individual tasks. These tasks are: datalogging,
          display, precharge, CAN, idle. These tasks will be further
          described in the documentation for their individual files.

    Main. The source for the entire firmware. Utilizes Arduino's framework
    with void setup() and void loop() methods, but only void setup() is used.
    Before the void setup(), Main.h is included into the file which in turn
    interconnects all files and allows for access to the global variables.
    These variables are what is eventually used in setup() and all the
    helper methods, being passed along to all other files/tasks
    as pointers, allowing for them to updated and accessed at the same time.
    If Can.ino updates the motorTemperature, Display.ino has
    an updated value for the motorTemperature. Then the void setup will execute
    everything and also act as the loop. That is because the setup calls on the
    xTaskCreate() method from the RTOS library and then the vTaskStartScheduler()
    method which in turn abstracts everything from us and rotates between
    all the tasks updating them and transferring resources as required. As of now,
    tasks operate on different resources than each other, thus there is no
    need for semaphores, however, if that is changed in the future, there
    is an example line of creating a semaphore above the calls to the
    xTaskCreate() method. The breakdown of how these methods and variables
    work in detail are explained further in the documentation closer for
    those methods and variables specifically. Please make sure to read
    and understand everything before you make any changes to this firmware.

    \note
      As of 3/1/2023, I think this firmware is fully race-ready and meeting
      all requirements to race. Ideally, I would use HIL and their CAN interface
      to further test the requirements and ensure proper performance under race-communication
      circumstances, but I'm just tired, you guys can do that while testing
      everything else next quarter. As of now, my goal is to create full documentation
      for this codebase and train up all members to be able to use it without any trouble.

    \todo
      Refine further.
      \n \n
      Look for redundant variables, pins, etc and remove.
      \n \n
      Change the name of preCharge. It's not just preCharge now, I personally like the name
      "controls.ino" and "controls.h". Make sure you change the #includes as well if you do that.
      Please Please change "preCharge" or whatever it's called to not use this many global/struct
      variables. It doesn't need everything globalized, it can pass them as parameters and eventually
      output to global variables *preCharge.angle_X and *preCharge.angle_Y.
      \n \n
      Github tutorial:
      \n \n
      Syed Yasir
      \n \n 
*/

// Includes the Main.h file which in includes all other files and interconnects all other files together.
#include "Main.h"

void setup() {

  /// First this method calls on setupPins() to set certain pins to output and input. This initializes
  /// how the pins will be used and interact with peripherals for the rest of the runtime of the firmware.
  setupPins();

  /// Then this method calls on initializeDisplayStructs() to map the static variables from Main.h from
  /// to a struct that is used for the displayTask()
  initializeDisplayStructs();
  /// Then this method calls on initializeCANStructs() to map the static variables from Main.h from
  /// to a struct that is used for the canTask()
  initializeCANStructs();
  /// Then this method calls on initializeLogStructs() to map the static variables from Main.h from
  /// to a struct that is used for the dataLoggingTask()
  initializeLogStructs();
  /// Then this method calls on initializePreChargeStruct() to map the static variables from Main.h from
  /// to a struct that is used for the prechargeTask()
  initializePreChargeStruct();


  /// Then this method initializes Serial communication using Serial.begin() initializing Serial at a
  /// baud rate of 115200.
  Serial.begin(115200);

  /// \note
  /// DISCLAIMER: When first running this code on a fresh board, the time will not properly update to
  /// Pacific Standard, or any real-looking time. Instead it will update to whatever
  /// default value is on the teensy board RTC (real-time clock). Thus, to set the Teensy RTC
  /// to the real world clock, you need to upload the sketch:
  /// \n \n
  /// File->Examples->Time->TimeTeensy3 and open the serial port. That will set the internal time to the
  /// time on the laptop I believe, or connects to the internet somehow to determine the "current" time.
  /// This next line calls with the statement setTime(Teensy3Clock.get()) sets the current stored time
  /// to the time stored in the internal RTC in the Teensy, allowing for Teensy time from the internal RTC
  /// (powered by the coin cell).
  setTime(Teensy3Clock.get());

  /// Then this method starts the SD Card and prints the status if that works.
  Serial.print("Starting SD: ");
  if (startSD()) {
    Serial.println("SD successfully started");
    sdStarted = 1;
  } else {
    sdStarted = 0;
    Serial.println("Error starting SD card");
  }

  /// Then this method calls on the setupDisplay() method which just initializes variables that are used
  /// by the displayTask()
  setupDisplay(measurementData);

  /// Then this method calls on the setupCAN() method which just initializes the CAN bus at a baud rate
  /// of 250000. And then begins the CAN bus.
  setupCAN();

  /// Then this method calls on the setupI2C() method which just initializes the I2C communication protocol,
  /// setting the clock to 40KHz, reading in the initial values of the gyroscope, taking ~2 seconds
  /// worth of data to calibrate.
  setupI2C(preChargeData);

  // unused but left as a reminder for how you can use it
  spi_mutex = xSemaphoreCreateMutex();

  /// The main and most important function of setup is initializing the tasks using xTaskCreate().
  /// The details of parameters are explained in the Note at the start of setup().
  /// The stack sizes are edited in Main.h and the priorities are logically assigned for
  /// preChargeTask() with highest priority 5, canTask() with second highest priority 4 since
  /// it contains data that can be used to trigger Off states for preChargeTask(). dataLogging()
  /// is 3, it's important to update the stored data from the race, but it's more important
  /// to shutdown the bike if batteries are dying. displayTask() is a 2 because see aforementioned reasoning
  /// for dataLogging(). idleTask() is a 1 ideally we never want to idle unless everything is taken care of
  /// and the teensy is that powerful.

  portBASE_TYPE s1, s2, s3, s4, s5;
  s1 = xTaskCreate(prechargeTask, "PRECHARGE TASK", PRECHARGE_TASK_STACK_SIZE, (void *)&preChargeData, 5, NULL);
  // make sure to set CAN_NODES in config.h
  s2 = xTaskCreate(canTask, "CAN TASK", CAN_TASK_STACK_SIZE, (void *)&canTaskData, 4, NULL);
  s3 = xTaskCreate(idleTask, "IDLE_TASK", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  s4 = xTaskCreate(displayTask, "DISPLAY TASK", DISPLAY_TASK_STACK_SIZE, (void*)&measurementData, 2, NULL);
  s5 = xTaskCreate(dataLoggingTask, "DATA LOGGING TASK", DATALOGGING_TASK_STACK_SIZE, (void*)&dataLoggingTaskData, 3, NULL);

  /// Then, the firmware checks if the tasks passed, if they failed, it stays in a loop printing error
  /// creating tasks.
  if (s1 != pdPASS || s2 != pdPASS || s3 != pdPASS || s4 != pdPASS || s5 != pdPASS) {
    Serial.println("Error creating tasks");
    while (1);
  }

  /// Lastly the method prints out that it will attempt to start the scheduler.
  /// If this succeeds, only lines from the task methods should know execute.
  Serial.println("Starting the scheduler");
  vTaskStartScheduler();

  /// If this fails, the method prints out insufficient RAM and breaks.
  Serial.println("Insufficient RAM");
}

void setupPins() {
  /// \note NOTE:
  /// SOME OF THESE PINS MAY NEED TO BE REMOVED NOW:
  /// CONTACTOR_CLOSED_LED, CONTACTOR_PRECHARGED_LED, CLOSE_CONTACTOR_BUTTON

  /// Initializes the pin corresponding to High Voltage Toggle as an input PULLUP resistor.
  pinMode(HIGH_VOLTAGE_TOGGLE, INPUT_PULLUP);
  /// Initializes the pin corresponding to Closing the Contactor as an input PULLUP resistor.
  pinMode(CLOSE_CONTACTOR_BUTTON, INPUT_PULLUP);
  /// Initializes the pin corresponding to the Screen reset as output and then outputs HIGH to that pin.
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);
  /// Initializes the pin corresponding to the Chip Select as output and then outputs HIGH to that pin.
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);

  /// UNSURE WHAT THE TS_CS pin is and at this point I'm too scared to ask. Sets it to output and HIGH.
  pinMode(TS_CS, OUTPUT);
  digitalWrite(TS_CS, HIGH);

  /// Initializes the pin corresponding to the Precharge as output and then outputs LOW to that pin.
  pinMode(PRECHARGE, OUTPUT);
  digitalWrite(PRECHARGE, LOW);
  /// Initializes the pin corresponding to the Contactor as output and then outputs LOW to that pin.
  pinMode(CONTACTOR, OUTPUT);
  digitalWrite(CONTACTOR, LOW);

  /// Initializes the pin corresponding to the Contactor Precharge-Read LED
  /// as output and then outputs LOW to that pin.
  pinMode(CONTACTOR_PRECHARGED_LED, OUTPUT);
  digitalWrite(CONTACTOR_PRECHARGED_LED, LOW);

  /// Initializes the pin corresponding to the Contactor-Closed LED as output and then outputs LOW to that pin.
  pinMode(CONTACTOR_CLOSED_LED, OUTPUT);
  digitalWrite(CONTACTOR_CLOSED_LED, LOW);
}

void initializeDisplayStructs() {
  measurementData = {
    &seriesVoltage, &motorControllerBatteryVoltage, &auxiliaryBatteryVoltage,
    &RPM, &motorControllerTemp, &motorCurrent, &errorMessage,
    &chargerCurrent, &chargerVoltage, &bms_status_flag, &evccVoltage, thTemps
  };
}

void initializeLogStructs() {
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
                   &Kalman_UncertaintyAngleRoll, &Kalman_AnglePitch, &Kalman_UncertaintyAnglePitch, Kalman_1DOutput
                  };
}

bool get_SPI_control(unsigned int ms) {
  return xSemaphoreTake(spi_mutex, ms);
}

void release_SPI_control(void) {
  xSemaphoreGive(spi_mutex);
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

void loop() {
}
