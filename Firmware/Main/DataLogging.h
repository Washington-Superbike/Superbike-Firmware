/**
   @file DataLogging.h
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The DataLogging.h config file for CAN bus for the bike's firmware. This initializes
          all variables that are passed along to all other files as
          pointers. Then it runs the setup methods for all those
          files and then it sets up RTOS to run all the different files
          as individual tasks. These tasks are: datalogging,
          display, precharge, CAN, idle. These tasks will be further
          described in the documentation for their individual files.


    \note
      up all members to be able to use it without any trouble.

    \todo
      Goal 1.
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

#define DATALOGGING_TASK_STACK_SIZE configMINIMAL_STACK_SIZE + 32368

//Names for each of the log files
#define MOTOR_TEMPERATURE_LOG "motor_temperature_log.csv"
#define MOTOR_CONTROLLER_TEMPERATURE_LOG "motor_controller_temperature_log.csv"
#define BMS_VOLTAGE_LOG "bms_voltage_log.csv"
#define MOTOR_CONTROLLER_VOLTAGE_LOG "motor_controller_voltage_log.csv"
#define MOTOR_CURRENT_LOG "current_log.csv"
#define THERMISTOR_LOG "thermistor_log.csv"
#define RPM_LOG "rpm_log.csv"

// Use built-in SD for SPI modes on Teensy 3.5/3.6.
// Teensy 4.0 use first SPI port.
// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

const size_t BUF_DIM = 4096;

//Represents the serial connection to the sd card and any internal buffers
SdFat sd;

unsigned long epochTime; //represents the time that recording started

//Clarification on how SD card stores data: each block is generally about 512 or 1024 bytes. SdFat stores an internal buffer of 512 and when the limit is reached
//only then does it save the data onto the sd card. This is an important trade off. if we are less than that value and the bike suddenly turns off, those <512 bytes aren't stored
//unless we somehow flush the data or use file.close(); this means we should potentially implement a signal for recording values, or even just only record when the HV system is on (not for testing things like precharge obv)
//or we just periodically flush data but that uses more resources on the teensy (this depends on how long it takes + how fast we are writing data)


enum data_type {
  INT,
  FLOAT
};


//Represents a writer to a CSV log file on the sd card
typedef struct {
  const char *filename;
  int dataValuesLen;
  float *dataValues;             //array of pointers to shared variables (the data values in the csv log)
  data_type D_TYPE;
  bool open;
  SdFile file;

} CSVWriter;

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
