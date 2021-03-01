
#include "DataLogging.h"

//chip select pin for the SD card
const int SD_CS = 18;

//Represents the serial connection to the sd card and any internal buffers
SdFat sd;

unsigned long epochTime; //represents the time that recording started

//startSD attempts to begin communication with the SD card on SPI (single bit bus)
//Returns true if no errors exist, returns false if an error exists
bool startSD() {
    return  sd.begin(SD_CS, SPI_HALF_SPEED);
}

//openFile attemps to open the file designated at the filename inside CSVWriter
//Returns true if no errors, returns false if any error exists
bool openFile(CSVWriter *writer) {
    writer->open=writer->file.open(writer->filename, O_RDWR|O_CREAT);
    return writer->open;
}

void saveFile(CSVWriter *writer) {
    writer->file.sync();
}

void saveFiles(CSVWriter **writers, int writersLen){
    for(int i=0; i<writersLen;i++){
        saveFile(writers[i]);
    }
}

//closeFile closes the file inside the CSVWriter
void closeFile(CSVWriter *writer) {
    writer->file.close();
}

void printFile(CSVWriter *writer) {
    if(!writer->open) {
        openFile(writer);
    }
    int data;
    while ((data = writer->file.read()) >= 0) Serial.write(data);
}

//dataLoggingTask processes all of the data logs and formats each CSV file output
void dataLoggingTask(DataLoggingTaskData dlData) {
    int sTime = (millis()-epochTime)/1000;
    for(int i=0; i<dlData.writersLen; i++) {
        addRecord(dlData.writers[i], sTime);
    }
}

//addRecordToCSV adds a record to the data log with the included time in seconds since the recording has started
//the data comes from the dataIn member (shared variable to other tasks)
void addRecord(CSVWriter *writer, int sTime) {
    if(!writer->open) {
        openFile(writer);
    }
    String sRecord = String(sTime);
    for(int i=0; i<writer->dataValuesLen; i++) {
        sRecord.concat(",").concat(writer->dataValues[i]);
    }
    writer->file.println(sRecord);
}

//startRecording sets the "epoch" or beginning time so that each data log is synced up
void startRecording() {
    epochTime = millis();
}
