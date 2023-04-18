/**
   @file DataLogging.ino
     @author    Washington Superbike
     @date      1-March-2023
     @brief
          The DataLogging.ino file is used to excecute on the DataLogging task
          for the bike's firmware. This task uses the CSVWriter data passed as logs
          to process and then save the data into files. There's not much else to it.
          Iterate over logs and save it.

    \note
      All members should be able to use it without any trouble.
      Chase made a method to process the logs data without any trouble, but they  be able to use it without any trouble.

    \todo
      Use the void pointer stuff in the addRecord method. Fix it up. Not required, but
      preferred.
      \n \n
      Goal 2.
      \n \n
      Goal 3.
      \n \n
      Final Goal.
*/
#include "DataLogging.h"
#include "Display.h"
#include "Main.h"
#include "FreeRTOS_TEENSY4.h"

void dataLoggingTask(void *dlData) {
  /// The dataLoggingTask() processes the data, for each of the logs.
  /// Just uses sdFat methods within the helper methods addRecord(), saveFiles() as
  /// required to process the neatly organized CSVWriter data and then save it.
  DataLoggingTaskData dl = *(DataLoggingTaskData *)dlData;
  int sTime;
  unsigned int lastSave = millis();
  while (1) {
    int mTime = millis();
    sTime = (millis() - epochTime) / 1000;
    for (int i = 0; i < dl.writersLen; i++) {
      addRecord(dl.writers[i], sTime);
    }
    if ((millis() - lastSave) > 10000) {
      Serial.print("Saving files...");
      saveFiles(dl.writers, dl.writersLen);
      Serial.println("saved");
      lastSave = millis();
    }
    // delay 50ms (change to modify how fast records are saved
    // in the future, this 'writeRate' could be specific to each data log
    // ex: motor current should write every 5ms but battery voltage only every 1 second
    // as battery voltage doesn't change very fast but current can
    vTaskDelay((50 * configTICK_RATE_HZ) / 1000);
  }
}

bool startSD() {
  /// startSD() attempts to begin communication with the SD card on SPI (single bit bus)
  ///Returns true if no errors exist, returns false if an error exists
  return  sd.begin(SdioConfig(FIFO_SDIO));
}

bool openFile(CSVWriter *writer) {
  /// openFile() attemps to open the file designated at the filename inside CSVWriter
  /// Returns true if no errors, returns false if any error exists
  writer->open = writer->file.open(writer->filename, O_RDWR | O_CREAT);
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

void saveFiles(CSVWriter **writers, int writersLen) {
  /// saveFiles() calls on saveFile() multiple times
  /// until all the files are saved.
  for (int i = 0; i < writersLen; i++) {
    saveFile(writers[i]);
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

  /// \note NOTE:
  /// The function kinda works but should be redone later to use (void *) datatype instead of (float *). Then cast the (void *) to whatever datatype is given by D_TYPE
  /// Example: to print an int you would do
  /**
   * THIS IS BETTER VIEWED IN CODE THAN IN DOCUMENTATION...
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
 /// \n \n
  /// I want to add it in now but it should be tested before being merged obviously.. Chase

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

void startRecording() {
  /// startRecording() sets the "epoch" or beginning time so that each data log is synced
  /// up.
  epochTime = millis();
}








