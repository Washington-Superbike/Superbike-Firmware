
#include "DataLogging.h"


const int SD_CS = 15;

SdFat sd;

//Attempts to start communication with the SD card
//Returns true if no errors exist, returns false if an error exists
bool startSD(){
    return  sd.begin(SD_CS, SPI_HALF_SPEED);
}

//Attemps to open file on SD card
//Returns true if no errors, returns false if any error exists
bool openFile(CSVWriter writer){
    return writer.file.open(writer.filename, O_WRITE|O_CREAT);
}

void closeFile(CSVWriter writer){
    writer.file.close();
}

void dataLoggingTask(){
    int sTime = (millis()-epochTime)/1000;
}


void addRecordToCSV(CSVWriter writer, int sTime){
    if(!writer.open){
        openFile(writer);
    }
    String sRecord = String(sTime).concat(",").concat(*(writer.dataIn));
    writer.file.println(sRecord);
}


void startRecording(){
    epochTime = millis();
}
