
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

bool startSD();
bool openFile(CSVWriter *writer);
void closeFile(CSVWriter *writer);
void dataLoggingTask(void *dlData);
void addRecordToCSV(CSVWriter *writer, int sTime, float record);

#endif
