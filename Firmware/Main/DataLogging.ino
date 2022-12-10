
#include "DataLogging.h"
#include "Display.h"
#include "Main.h"
#include "FreeRTOS_TEENSY4.h"

//Represents the serial connection to the sd card and any internal buffers
SdFat sd;

unsigned long epochTime; //represents the time that recording started

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

void saveFile(CSVWriter *writer) {
  writer->file.sync();
}

void saveFiles(CSVWriter **writers, int writersLen) {
  for (int i = 0; i < writersLen; i++) {
    saveFile(writers[i]);
  }
}

//closeFile closes the file inside the CSVWriter
void closeFile(CSVWriter *writer) {
  writer->file.close();
}

void printFile(CSVWriter *writer) {
  if (!writer->open) {
    openFile(writer);
  }
  int data;
  while ((data = writer->file.read()) >= 0) Serial.write(data);
}

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
      if ((millis() -lastSave) > 10000) {
          Serial.print("Saving files...");
          saveFiles(dl.writers, dl.writersLen);
          Serial.println("saved");
          lastSave = millis();
      }
    vTaskDelay((50 * configTICK_RATE_HZ) / 1000);
  }
}

//addRecord adds a record to the data log with the included time in seconds since the recording has started
//the data comes from the dataIn member (shared variable to other tasks)
void addRecord(CSVWriter *writer, int sTime) {
  if (!writer->open) {
    openFile(writer);
  }
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
