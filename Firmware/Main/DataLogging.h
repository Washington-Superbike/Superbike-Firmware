/**
   @file DataLogging.h
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The DataLogging.h config file for the DataLogging task for the bike's firmware. This defines the variables that are passed along to the DataLogging.ino file and
          others if they use it. Then it creates the initial reference (there's a proper
          C programming term for it) for all the methods used in DataLogging.ino. This file also creates two typedef structs that basically allow us to package the data in DataLogging for the task in a nice way. The other struct CSVWriter is used to organize the data properly to be used in the SdFat framework to write to the SD card. Like all header files, this exists as the skeleton/framework for the .ino or main c file.


    \note
      Honestly, I dont see any need to fix this further. The only thing I foresee, is
      that the State of Charge team would be more interested in tweaking the macros (#define statements) and other aspects of this file and Main.h to ensure that they can add more files into the DataLogging task.
      \n \n

      Clarification on how SD card stores data: each block is generally about 512 or
      bytes. SdFat stores an internal buffer of 512 and when the limit is reached
      only then does it save the data onto the sd card. This is an important trade off
      if we are less than that value and the bike suddenly turns off, those <512 bytes
      aren't stored unless we somehow flush the data or use file.close(); this means we
      should potentially implement a signal for recording values, or even just only
      record when the HV system is on (not for testing things like precharge obv)
      or we just periodically flush data but that uses more resources on the teensy (this
      depends on how long it takes + how fast we are writing data)

    \todo
      Just add the LV current or the added up current to the current logging situation.
      Could be useful in future if we want to be tryhards and use a neural network to process currents or just to log our current and see what went wrong in the race.
      (Latter is way more likely)
      \n \n
      Goal 2.
      \n \n
      Goal 3.
      \n \n
      Final Goal.
*/

#ifndef DATALOG_H_
#define DATALOG_H_

#include "SdFat.h"

/// Uses the configMINIMAL_STACK_SIZE variable in Main.h to add up to the stack size used for the dataLoggingTask()
#define DATALOGGING_TASK_STACK_SIZE configMINIMAL_STACK_SIZE + 32368

//Names for each of the log files
/// The name for the motor temperature log.
#define MOTOR_TEMPERATURE_LOG "motor_temperature_log.csv"
/// The name for the motor controller temperature log
#define MOTOR_CONTROLLER_TEMPERATURE_LOG "motor_controller_temperature_log.csv"
/// The name for the BMS voltage log
#define BMS_VOLTAGE_LOG "bms_voltage_log.csv"
/// The name for the motor controller voltage log
#define MOTOR_CONTROLLER_VOLTAGE_LOG "motor_controller_voltage_log.csv"
/// The name for the motor controller current log
#define MOTOR_CURRENT_LOG "current_log.csv"
/// The name for the thermistor temperature log
#define THERMISTOR_LOG "thermistor_log.csv"
/// The name for the motor RPM log
#define RPM_LOG "rpm_log.csv"

/// Use built-in SD for SPI modes on Teensy 3.5/3.6.
// Teensy 4.0 use first SPI port.
// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

const size_t BUF_DIM = 4096;

/// Represents the serial connection to the sd card and any internal buffers
SdFat sd;

/// Represents the time that recording started
unsigned long epochTime;

/**
 * Basic enum for data types for logging. Nothing else really.
 */
enum data_type {
  INT,
  FLOAT
};


/**
 * Represents a writer to a CSV log file on the sd card. Complex struct.
 * Keep it as it is, doesn't require changing really. Just use it.
 */
typedef struct {
  const char *filename;
  int dataValuesLen;
  //array of pointers to shared variables (the data values in the csv log)
  // Dereferences the values
  float *dataValues;
  data_type D_TYPE;
  bool open;
  SdFile file;

} CSVWriter;

/**
 * A struct for the overall dataLoggingTaskData. Stores a pointer
 * to the pointer (array) of CSVWriters. Thus to process the logs,
 * you have to dereference the pointer and then call on the array by
 * index. The length just represents the number of logs and log files to generate.
 * This would be of interest to teams that might want to add more info in the
 * log SD card.
 */
typedef struct {
  CSVWriter **writers;
  int writersLen;
} DataLoggingTaskData;

void dataLoggingTask(void *dlData);
bool startSD();
bool openFile(CSVWriter *writer);
void closeFile(CSVWriter *writer);
void saveFile(CSVWriter *writer);
void saveFiles(CSVWriter **writers, int writersLen);
void printFile(CSVWriter *writer);
void addRecord(CSVWriter *writer, int sTime);
void startRecording();
#endif
