/**
   @file DataLogging.ino
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
#include "DataLogging.h"
#include "Display.h"
#include "Main.h"
#include "FreeRTOS_TEENSY4.h"

//dataLoggingTask processes all of the data logs and formats each CSV file output
void dataLoggingTask(void *dlData) {
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

//startSD attempts to begin communication with the SD card on SPI (single bit bus)
//Returns true if no errors exist, returns false if an error exists
bool startSD() {
  return  sd.begin(SdioConfig(FIFO_SDIO));
}

//openFile attemps to open the file designated at the filename inside CSVWriter
//Returns true if no errors, returns false if any error exists
bool openFile(CSVWriter *writer) {
  writer->open = writer->file.open(writer->filename, O_RDWR | O_CREAT);
  return writer->open;
}

//closeFile closes the file inside the CSVWriter
void closeFile(CSVWriter *writer) {
  writer->file.close();
}

void saveFile(CSVWriter *writer) {
  writer->file.sync();
}

void saveFiles(CSVWriter **writers, int writersLen) {
  for (int i = 0; i < writersLen; i++) {
    saveFile(writers[i]);
  }
}

void printFile(CSVWriter *writer) {
  if (!writer->open) {
    openFile(writer);
  }
  int data;
  while ((data = writer->file.read()) >= 0) Serial.write(data);
}

//addRecord adds a record to the data log with the included time in seconds since the recording has started
//the data comes from the dataIn member (shared variable to other tasks)
void addRecord(CSVWriter *writer, int sTime) {
  if (!writer->open) {
    openFile(writer);
  }

  // the function kinda works but should be redone later to use (void *) datatype instead of (float *).
  // then cast the (void *) to whatever datatype is given by D_TYPE
  // example: to print an int you would do

  /*
    void *point = writer->dataValues;
    for(int i = 0; i < writer->dataValuesLen) {
      switch(writer->D_TYPE) {
        case FLOAT:
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
  // I want to add it in now but it should be tested before being merged obviously... - Chase

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

//startRecording sets the "epoch" or beginning time so that each data log is synced up
void startRecording() {
  epochTime = millis();
}
