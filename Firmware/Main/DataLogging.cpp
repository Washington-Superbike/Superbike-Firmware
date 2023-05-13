/**
 * Controls writing of variable values to CSV files stored on the SD card.
*/

#include <SdFat.h>
#include "DataLogging.h"
#include "config.h"
#include "FreeRTOS_TEENSY4.h"
#include "context.h"

const size_t BUF_DIM = 4096;

/// Represents the time that recording started
unsigned long epochTime;

//SdFat file system
SdFs sd;

void dataLoggingTask(void *dlData) {
  /// The dataLoggingTask() processes the data, for each of the logs.
  /// Just uses sdFat methods within the helper methods addRecord(), saveFiles() as
  /// required to process the neatly organized CSVWriter data and then save it.
  DataLoggingTaskData *dl = (DataLoggingTaskData *)dlData;
  Context *context = dl->context;
  CSVWriter *writers = context->logs;
  const TickType_t epoch = xTaskGetTickCount();
  TickType_t last_save = xTaskGetTickCount();
  TickType_t now = xTaskGetTickCount();
  while (1) {
    if (context->sd_started) {
      now = xTaskGetTickCount();
      for (int i = 0; i < CONFIG_LOG_COUNT; i++) {
        addRecord(&writers[i], now - epoch);
      }
      if ((now - last_save) > 10000) {
        Serial.print("Saving files...");
        saveFiles(writers);
        Serial.println("saved");
        last_save = now;
      }
        // delay 50ms (change to modify how fast records are saved
      // in the future, this 'writeRate' could be specific to each data log
      // ex: motor current should write every 5ms but battery voltage only every 1 second
      // as battery voltage doesn't change very fast but current can
      vTaskDelay((50 * configTICK_RATE_HZ) / 1000);
    } else {
      Serial.println("SD card missing");
      // only print every 10 seconds, or until sd_card is fixed (not done currently)
      vTaskDelay((10000 * configTICK_RATE_HZ) / 1000);
    }
  }
}

bool startSD() {
  /// startSD() attempts to begin communication with the SD card on SPI (SDIO)
  ///Returns true if no errors exist, returns false if an error exists
  return sd.begin(SdioConfig(FIFO_SDIO));
}

bool openFile(CSVWriter *writer) {
  /// openFile() attemps to open the file designated at the filename inside CSVWriter
  /// Returns true if no errors, returns false if any error exists
  writer->open = writer->file.open(writer->filename, O_RDWR | O_CREAT | O_TRUNC);
  if (!writer->open)
    Serial.printf("ERROR: Failed to open file %s\n", writer->filename);
  return writer->open;
}

void closeFile(CSVWriter *writer) {
  /// closeFile() closes the file inside the CSVWriter
  writer->file.close();
}

void saveFile(CSVWriter *writer) {
  /// saveFile() saves the file inside the CSVWriter
  writer->file.sync();
}

void saveFiles(CSVWriter *writers) {
  /// saveFiles() calls on saveFile() multiple times
  /// until all the files are saved.
  for (int i = 0; i < CONFIG_LOG_COUNT; i++) {
    saveFile(&writers[i]);
  }
}

void printFile(CSVWriter *writer) {
  /// Opens the file using openFile() and then prints out what is
  /// read in from the openFile() method to the Serial monitor
  if (!writer->open) {
    openFile(writer);
  }
  int data;
  while ((data = writer->file.read()) >= 0) Serial.write(data);
}

void addRecord(CSVWriter *writer, int sTime) {

/// addRecord() adds a record to the data log with the included time in seconds since the
/// recording has started. The data comes from the dataIn member (shared variable to
/// other tasks) As of now it converts everything into a String and then prints it to the
/// file.

  if (!writer->open) {
    openFile(writer);
  }

  /// The function kinda works but should be redone later to use (void *) datatype instead of (float *). Then cast the (void *) to whatever datatype is given by D_TYPE
  /// Example: to print an int you would do
  /*
   *  void *point = writer->dataValues;
   * for(int i = 0; i < writer->dataValuesLen) {
   *    switch(writer->D_TYPE) {
   *    case FLOAT:
            sRecord.concat(",").concat(writer->dataValues[i]);
            point += sizeof(float); // how much space the float took in memory
            break;
          case INT:
            sRecord.concat(",").concat(writer->dataValues[i]);
            point += sizeof(int); // how much space the int took in memory
          case ... (can be any other D_TYPE we define)
            ... (similar to previous two cases)
            break;
          default: Serial.printf("Unknown D_TYPE: %u\n", D_TYPE); break;
        }
      }
  */

  String sRecord = String(sTime);
  for (int i = 0; i < writer->dataValuesLen; i++) {
    if (writer->D_TYPE == FLOAT) {
      sRecord.concat(",").concat(writer->dataValues[i]);
    } else if (writer->D_TYPE == INT) {
      sRecord.concat(",").concat(int(writer->dataValues[i]));
    }
  }
  writer->file.println(sRecord);
}
