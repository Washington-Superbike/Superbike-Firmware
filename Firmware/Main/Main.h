/**
   @file Main.h
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The main config file for bike firmware. This initializes
          all variables that are passed along to all other files as
          pointers. Then it creates the initial reference (there's a proper
          C programming term for it) for all the methods used in Main.ino.
          This file exists as an overall configuration for the
          bike firmware as all these variables can be changed to meet
          requirements. Like all other header files, it exists
          as the skeleton on which the .ino file is built.

    The main config file for bike firmware. This initializes
    all variables that are passed along to all other files as
    pointers. Then it creates the initial reference (there's a proper
    C programming term for it) for all the methods used in Main.ino.
    This file exists as an overall configuration for the
    bike firmware as all these variables can be changed to meet
    requirements. Like all other header files, it exists
    as the skeleton on which the .ino file is built.

    \note
      Main thing to note. The statement #pragma once is a compiler specific command for newer compilers in C.
      It is the equivalent of ifndef define statements. If you dont know what those are, please take CSE 351.
      To give the gist of it, you need a #pragma once or ifndef statment (in our older code) to ensure
      that when running the code, when you do a #include Main.h and if every other file does that,
      the #pragma once command ensures that the first time the Main.h is #included, it is properly
      defined, so that when a file attempts to include it again, it skips that including.
      If that doesn't make sense, dont worry, CSE 351, CSE 374. Please take them. They're
      like top 6 EE classes.
      Secondly, in this file all variables are static because the static keyword in C++
      (which is what .ino files are technically speaking) is basically used
      to define global variables that are not to be removed in memory. All the variables
      below are the skeleton of the firmware, every file references to these variables
      by being passed their address and changing their value using that address.
      Instead of creating 10 copies when passing variables around, there's just
      one global copy.
      ALSO, if you are looking to change configurations for the overall
      firmware this is the place to do it. Changing the number of CAN devices connected,
      the Screen type used, etc. All can be done from here.

    \todo
      Based on refinements made for preCharge/controls.ino, remove spare and redundant variables. And just generally ALL the spare variables.
      \n \n
      Based on the changes implemented for SoC, I would add another variable for the low-voltage
      current sneors or any other current sensors you add
      \n \n
      CHANGE THE NUM_THERMI based on the number of thermistors that Powertain settles on.
      \n \n
      Final Goal.
*/

#pragma once
#include "CAN.h"
#include "Display.h"
#include "Precharge.h"
#include "DataLogging.h"
#include "FreeRTOS_TEENSY4.h"
#include <TimeLib.h>


/// This primarily exists to debug the changes made in the FlexCAN library.
/// If there are no devices connected on the CAN bus, the firmware crashes
/// This line can be set to 0 to ensure that the CAN bus does not bother
/// to check the CAN bus if there are 0 nodes connected.
#define CAN_NODES 0

/// This exists to be changed based on the final number of thermistors
/// we settle on having in the code later.
#define NUM_THERMI 10

/// A screen enum that defines the type of screen to generate
/// in Display.ino.
/// \note NOTE: CHANGE THIS LINE TO CHANGE DISPLAY TYPE!
static Screen screen = {SPEEDOMETER};

// BMS and Battery values. Determined via CAN and other protocols.
// TODO: PROBABLY ADD THE LV System current here.
/// Self explanatory. Read datasheet if more info needed.
static int bms_status_flag = 0;
/// Self explanatory. Read datasheet if more info needed.
static int bms_c_id = 0;
/// Self explanatory. Read datasheet if more info needed.
static int bms_c_fault = 0;
/// Self explanatory. Read datasheet if more info needed.
static int ltc_fault = 0;
/// Self explanatory. Read datasheet if more info needed.
static int ltc_count = 0;
/// Self explanatory. Read datasheet if more info needed.
static float cellVoltagesArr[BMS_CELLS];  // voltages starting with the first LTC
/// Self explanatory. Read datasheet if more info needed.
static float seriesVoltage;
/// Self explanatory. Read datasheet if more info needed.
static bool cellsReady;
/// Self explanatory. Read datasheet if more info needed.
static float thTemps[NUM_THERMI];
/// Self explanatory. Read datasheet if more info needed.
static int thermistorEnabled;
/// Self explanatory. Read datasheet if more info needed.
static int thermistorPresent;
/// Self explanatory. Read datasheet if more info needed.
static float auxiliaryBatteryVoltage = 0;

// All these variables are used to read in the data from the gyroscope.
/// angle_X is the variable used for measuring the "x-axis angle" from the gyroscope.
static float angle_X = 0.0;
/// angle_Y is the variable used for measuring the "y-axis angle" from the gyroscope.
static float angle_Y = 0.0;
/// USELESS REMOVE LATER: Rate_Roll stores the rate of the roll angle (x-axis)
static float Rate_Roll = 0.0;
/// USELESS REMOVE LATER: Rate_Pitch stores the rate of the pitch angle (y-axis)
static float Rate_Pitch = 0.0;
/// USELESS REMOVE LATER: Rate_Yaw stores the rate of the yaw angle (z-axis)
static float Rate_Yaw = 0.0;
/// USELESS REMOVE LATER: Rate_CalibrationRoll stores the rate calibration of the roll angle (x-axis)
static float Rate_CalibrationRoll = 0.0;
/// USELESS REMOVE LATER: Rate_CalibrationPitch stores the rate calibration of the pitch angle (y-axis)
static float Rate_CalibrationPitch = 0.0;
/// USELESS REMOVE LATER: Rate_CalibrationYaw stores the rate calibration of the pitch angle (y-axis)
static float Rate_CalibrationYaw = 0.0;
/// USELESS REMOVE LATER: Rate_CalibrationNumber stores the rate calibration number)
static int Rate_CalibrationNumber = 0;

/// USELESS REMOVE LATER: Acc_X stores the acceleration along the x-axis
static float Acc_X = 0.0;
/// USELESS REMOVE LATER: Acc_Y stores the acceleration along the y-axis
static float Acc_Y = 0.0;
/// USELESS REMOVE LATER: Acc_Z stores the acceleration along the z-axis
static float Acc_Z = 0.0;
/// USELESS REMOVE LATER: Angle_Roll stores essentially angle_X
static float Angle_Roll = 0.0;
/// USELESS REMOVE LATER: Angle_Pitch stores essentially angle_Y
static float Angle_Pitch = 0.0;

// All these variables are used to filter the gyroscope angles using the Kalman filter.
/// USELESS REMOVE LATER: Kalman_AngleRoll is the Kalman filter processed roll angle.
static float Kalman_AngleRoll = 0.0;
/// USELESS REMOVE LATER:
static float Kalman_UncertaintyAngleRoll = 2 * 2;
/// USELESS REMOVE LATER:
static float Kalman_AnglePitch = 0;
/// USELESS REMOVE LATER:
static float Kalman_UncertaintyAnglePitch = 2 * 2;
/// USELESS REMOVE LATER: Output of the filter [0] is angle_x, [1] is angle_y
static float Kalman_1DOutput[] = {0, 0};

// All these variables are can values derived from the motor controller.
/// Self explanatory. Read datasheet if more info needed.
static float RPM = 0;
/// Self explanatory. Read datasheet if more info needed.
static float motorCurrent = 0;
/// Self explanatory. Read datasheet if more info needed.
static float motorControllerBatteryVoltage = 0;
/// Self explanatory. Read datasheet if more info needed.
static float throttle = 0;
/// Self explanatory. Read datasheet if more info needed.
static float motorControllerTemp = 0;
/// Self explanatory. Read datasheet if more info needed.
static float motorTemp = 0;
/// Self explanatory. Read datasheet if more info needed.
static int errorMessage = 0;
/// Self explanatory. Read datasheet if more info needed.
static byte controllerStatus = 0;

// Charge controller variables
/// Self explanatory. Read datasheet if more info needed.
static byte evccEnable = 0;
/// Self explanatory. Read datasheet if more info needed.
static float evccVoltage = 0;
/// Self explanatory. Read datasheet if more info needed.
static float evccCurrent = 0;

// Charger variables
/// Self explanatory. Read datasheet if more info needed.
static byte chargeFlag = 0;
/// Self explanatory. Read datasheet if more info needed.
static byte chargerStatusFlag = 0;
/// Self explanatory. Read datasheet if more info needed.
static float chargerVoltage = 0;
/// Self explanatory. Read datasheet if more info needed.
static float chargerCurrent = 0;
/// Self explanatory. Read datasheet if more info needed.
static int8_t chargerTemp = 0;

/// An instance of a struct to store all display data
static MeasurementScreenData measurementData = {};
/// An instance of a struct to store all motor data
static MotorStats motorStats = {};
/// An instance of a struct to store all motor temperature data
static MotorTemps motorTemps = {};
/// An instance of a struct to store all cell voltage data
static CellVoltages cellVoltages = {};
/// An instance of a struct to store all bms status data
static BMSStatus bmsStatus = {};
/// An instance of a struct to store all thermistorTemperatures
static ThermistorTemps thermistorTemps = {};
/// An instance of a struct to store all charger stats data
static ChargerStats chargerStats = {};
/// An instance of a struct to store all charge controller stats data
static ChargeControllerStats chargeControllerStats = {};

// These variables store all the task data for all 4 tasks (except IDLE)
/// An instance of a struct to store all CAN data, then pass it as a void pointer.
static CANTaskData canTaskData;
/// An instance of a struct to store all display data, then pass it as a void pointer.
static displayPointer displayTaskWrap = {};
/// An instance of a struct to store all dataLogging data, then pass it as a void pointer.
static DataLoggingTaskData dataLoggingTaskData;
/// An instance of a struct to store all preCharge data, then pass it as a void pointer.
static PreChargeTaskData preChargeData = {};

// Log variables used for dataLogging task. Change things here if you want to
// add more variables to logging.
/// An instance of the logging struct for motor temperatures.
static CSVWriter motorTemperatureLog = {};
/// An instance of the logging struct for motor controller temperatures.
static CSVWriter motorControllerTemperatureLog = {};
/// An instance of the logging struct for motor controller voltage.
static CSVWriter motorControllerVoltageLog = {};
/// An instance of the logging struct for motor controller current.
static CSVWriter motorCurrentLog = {};
/// An instance of the logging struct for motor RPM.
static CSVWriter rpmLog = {};
/// An instance of the logging struct for thermistor temperatures.
static CSVWriter thermistorLog = {};
/// An instance of the logging struct for BMS voltages.
static CSVWriter bmsVoltageLog = {};
/// An instance of the logging struct for storing all the above logs to pass onto dataLogging.
static CSVWriter *logs[] = {&motorTemperatureLog, &motorControllerTemperatureLog, &motorControllerVoltageLog, &motorCurrentLog, &rpmLog, &thermistorLog, &bmsVoltageLog};

/// @brief
unsigned long timer = millis();
int cycleCount = 0;

int lowerUpperCells = -1;
unsigned long ms = millis();
byte sdStarted = 0;

SemaphoreHandle_t spi_mutex;

/**
   \note
   setup() calls on all the helper methods in Main.ino. These helper methods serve
   to initialize all pin modes, package all variables from Main.h into structs, initialize
   all communication protocols (as of now, just I2C), and then use those packaged structs to
   call on the RTOS xTaskCreate() to  then send the variables off to various tasks with no issues.
   xTaskCreate() takes in a function name (pointer to an existing function which contains the essence
   of the task (these exist in the different files)), a label (string), stack size (integer/number),
   the parameters required for that task (a void pointer to the packaged structs from before)
   and lastly a task priority. There's another variable but it's kinda useless for our case
   and is only discussed in the detailed discussion below. \n \n

   Taking from the FreeRTOS documentation, the parameters serve the following DETAILED purposes.
   \n \n
   pvTaskCode aka the pointer to the function used for the task. This function is usually
   an infinite loop with an RTOS VTaskDelay() call which will be further elaborated on within
   those functions in the documentation.
   \n \n
   pcName aka the string with the funciton name. This is ususally used for debugging purposes.
   usStackDepth is the number of words to allocate for the task's stack (for those of you who havent
   had a chance to take CSE 374 or CSE 351, stack is dynamic memory, thus if any of your tasks
   use a lot of parameters or perform recursion, the stack size might have to be increased).
   Display has often been a victim of the stack size being too small in the past.
   \n \n
   pvParameters aka the void pointer to the struct composed of the variables declared in Main.h.
   Just a simple void pointer which is passed to the task when it executes.
   While passing variables and being careful with memory leaks is not too much of a concern
   thanks to the Arduino framework, it is still important to use good practices
   like this void pointer task memory unit to avoid any issues (plus RTOS just requires something
   to be passed through anyway).
   \n \n
   uxPriority aka the priority at which the task will execute. Lower priority numbers indicate
   tasks with lower priority in execution. That is, while the RTOS is constantly swapping between
   your tasks, ensuring they execute properly, etc the higher priority tasks are given highest
   priority to run once the CPU is ready. Just makes sense really.
   \n \n
   pxCreatedTask is a variable that is used to pass a handle to the created task outside of the
   xTaskCreate() function. That means the ??? IDK it's not needed.
*/
void setup();
/**
   \note
   setupPins() calls on the Arduino library's pinMode and digitalWrite() methods
   to initialize pins to specific states. The variables and macros corresponding to the
   pin numbers are taken from the .h files of the task they serve.
*/
void setupPins();
/**
   \note
   initializeDisplayStructs() uses the variables:
   seriesVoltage, motorControllerBatteryVoltage, auxiliaryBatteryVoltage, RPM, motorControllerTemp,
   motorCurrent, errorMessage, chargerCurrent, chargerVoltage, bms_status_flag, evccVoltage, thTempshe
   and then packages them into a variable called measurementData which is taken from Display.h
   Then the measurement data and screen variable are packaged into the displayTaskWrap variable
   which is passed to the displayTask as the void pointer for the task's data.
*/
void initializeDisplayStructs();
/**
   \note
   initializeLogStructs() uses too many variables...
   and it packages them into different logging variables which are then passed to the "logs"
   variable which is used by the dataLoggingTask(). Therefore, to add more data to datalogging or change the variables
   first you would need to change the corresponding variables that make up the "logs" varaible in Main.h and then
   change those corresponding variables in initializeLogStructs().
   \n \n
   This will be handy to the SOC team later as they implement more changes.
*/
void initializeLogStructs();
/**
   \note
   initializeCANStructs() uses too many variables...
   and it packages them into the different variables that are all being updated by the CAN protocol.
   These are variables that are read in from the motorController, Battery Management System, thermistors,
   charger, and charge controller. These are then packaged in to the canTaskData variable which is passed
   to canTask(). canTask() updates the variable which in turn updates it for everything, allowing display to be updated
   with no problem (since all variables used by tasks are just pointers to the original variable that is declared
   in Main.h and then updating one just updates everything across the board.
   \n \n
   Update this accordingly as we add more CAN items to the bike (if we ever do).
*/
void initializeCANStructs();
/**
   \note
   initializePreChargeStruct() uses too many variables...
   It is primarily composed of the cellVoltages, bmsStatus, etc. which are used
   to shutdown or startup to bike depending on the situation it is in. The rest of the variables
   correspond to the bloated method we use for gyroscope reading. It is good for racing and is
   efficient enough, but if we ever run out of stuff to do please make it less bloated as
   described in the TODO at the top of the file.
*/
void initializePreChargeStruct();
/**
   \note
   Just a basic helper method that takes control of the spiMutex which is used as an example line
   in setup() but not actually used in the current xTaskCreate() methods or this RTOS in general.
   \n \n
   The purpose of a mutex is to create a sort of indication to the RTOS that there is
   a specific resource being used for a specific task and to not begin any other task
   that interrupts this specific resource, like the pins used for the SPI protocol for
   this case, etc. etc.
   \n \n
   Please read up on Mutex, etc. in the RTOS documentation to make sure you use it correctly.
   This is only necessary if there are different tasks with two different devices using the
   same communication protocol.
*/
bool get_SPI_control(unsigned int ms);
/**
   \note
   Just a basic helper method releases control of the spiMutex which is used as an example line
   in setup() but not actually used in the current xTaskCreate() methods or this RTOS in general.
   \n \n
   The purpose of a mutex is to create a sort of indication to the RTOS that there is
   a specific resource being used for a specific task and to not begin any other task
   that interrupts this specific resource, like the pins used for the SPI protocol for
   this case, etc. etc.
   \n \n
   Please read up on Mutex, etc. in the RTOS documentation to make sure you use it correctly.
   This is only necessary if there are different tasks with two different devices using the
   same communication protocol.
*/
void release_SPI_control(void);
/**
   \note
   Just a basic helper method that returns the time variable that is stored in the teensy's RTC.
*/
time_t getTeensy3Time();
/**
   \note
   Just a basic IDLE task that seems to do nothing. If triggered it blocks out for 50 CPU cycles.
   Doing nothing.
*/
void idleTask(void *taskData);
/**
   \note
   Arduino framework classic. setup() should run once. loop() should run forever.
   Doesn't work in our case because we run RTOS via setup().
*/
void loop();
