#include "Main.h"
#include "SOC.h"

/*
Notes from working: 4/17
    Created this file and a header file for the SOC code in order to store the progress for the project
    Figuring out the correct links for files within the code
    Likely need to merge this code with another module/task to be on the task scheduler
    Currently a barebones version of SOC, allowing for the SOC variable to be updated
        Directly updated, as there is intermediate calculations done, and not directly read from Teensy/BMS

    Need to get a reasonably estimate for the rated capacity of the battery
    Could create a small part in the precharge task in order to see the current charge vs max estimated charge.

    For when the code should always execute, I think that it should operate within the canBus task
        This allows us to constantly update the SOC for the user, and adds redundancy to checking the voltage of the batteries.

*/

/*
Breaking this down into tasks:
Task 1: initialize a new log that tracks the SOC over time
Task 2: Fix the program variables so that it can get the values needed (current, voltage, etc.)
Task 3: Fix calculations for SOC
Task 4: initialize constant to represent Capacity
*/




// Later on we can introduce a method to actually track the intial SOC, for now going to assume it is 100%

//Reduce battery pack voltage so that it can be read by teensy analog to digital converter
//Parse this voltage

//For integration of current we can simply have a running total in the data logging sheet
//Every time new current values come in, we multiply it by the time interval between data records (assuming the time between data records
// is not that long) We then have a the total integral of the current out of the battery over the running timer interval
// Updates the SOC in main for the current percentage of battery left

// Should probably return as a percentage, or if an int, 0-100
void stateOfChargeCalculation() {
  int currTime = millis() - lastSave;
  integralOfCharge += motorCurrent * (currTime - lastSave);
  SOC += (1/ratedCapacity) * integralOfCharge;
  lastSave = currTime - epochTime;
}
//Extracts data from data logging into an array
//Time is the first entry in the array

// I think that we can remove this method, as we can just make a log ourselves and use
// the address of motorcurrent in order to get what we need
int[] extractValues(CSVwriter *writer, float *dataArray, int dataArrayLength) {
  //opens file if not already open
  if(!writer -> open) {
    openFile(writer);
  }
  //goes to first index in the file
  writer -> file.seek(0);
  int dataIndex = 0;
  String record;
  while (writer->file.available()) {
    record = writer->file.readStringUntil('\n');
    record.trim();
    // skip the first column since it contains the time data
    int dataStartIndex = record.indexOf(',') + 1;
    // extract the float value from the record and store it in the array
    float value = record.substring(dataStartIndex).toFloat();
    calculateIntegral(value);
    dataArray[dataIndex] = value;
    dataIndex++;
    if (dataIndex >= dataArrayLen) {
      // exit the loop if we have parsed enough data
      break;
    }
  }
  // close the file when finished
  writer->file.close();
}
