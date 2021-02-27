
#include "DataLogging.h"

//chip select pin for the SD card
const int SD_CS = 15;

//Represents the serial connection to the sd card and any internal buffers
SdFat sd;

unsigned long epochTime; //represents the time that recording started

//startSD attempts to begin communication with the SD card on SPI (single bit bus)
//Returns true if no errors exist, returns false if an error exists
bool startSD(){
    return  sd.begin(SD_CS, SPI_HALF_SPEED);
}

//openFile attemps to open the file designated at the filename inside CSVWriter
//Returns true if no errors, returns false if any error exists
bool openFile(CSVWriter writer){
    return writer.file.open(writer.filename, O_WRITE|O_CREAT);
}


//closeFile closes the file inside the CSVWriter
void closeFile(CSVWriter writer){
    writer.file.close();
}

//dataLoggingTask processes all of the data logs and formats each CSV file output
void dataLoggingTask(){
    int sTime = (millis()-epochTime)/1000;
}

//addRecordToCSV adds a record to the data log with the included time in seconds since the recording has started
//the data comes from the dataIn member (shared variable to other tasks)
void addRecordToCSV(CSVWriter writer, int sTime){
    if(!writer.open){
        openFile(writer);
    }
    String sRecord = String(sTime);
    for(int i=0;i<writer.dataValuesLen;i++){
        sRecord.concat(",").concat(*(writer.dataValues[i]));
    }
    writer.file.println(sRecord);
}

//startRecording sets the "epoch" or beginning time so that each data log is synced up
void startRecording(){
    epochTime = millis();
}
