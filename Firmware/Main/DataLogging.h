
#ifndef DATALOG_H_
#define DATALOG_H_

#include "SdFat.h"

#define SAVE_RATE 20                                                     //how many writes/reads until we close/reopen the files (battery could unexpectedly turn off while file is open)


#define MOTOR_TEMP_LOG "motor_temperature_log.csv"
#define MOTOR_CONTROLLER_TEMP_LOG "motor_controller_temperature_log.csv"
#define BMS_VOLTAGE_LOG "bms_voltage_log.csv"
#define MOTOR_CONTROLLER_VOLTAGE_LOG "motor_controller_voltage_log.csv"
#define CURRENT_LOG "current_log.csv"
#define THERMISTOR_FILE_BASE "thermistor_temp_"
#define THERMISTOR_FILE_END "_log.csv"
#define RPM_LOG "rpm_log.csv"



typedef struct CSVWriterStruct{
    bool open;
    String filename;
    SdFile file;
    byte saveCount;
}CSVWriter;

void dataLoggingTask();
void updateSaveStatus(CSVWriter writer);
void addRecordToCSV(CSVWriter writer, int sTime, float record);
void addRecordToCSV(CSVWriter writer, int sTime, int record);

#endif
