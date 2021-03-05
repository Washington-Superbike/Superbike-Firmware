#include "CAN.h"

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN_bus;    // access to can bus reading
CAN_message_t CAN_msg;                                // data is read into this from the can bus


void setupCAN() {
    CAN_bus.begin();
    CAN_bus.setBaudRate(250000);
    setupCANISR(); 
}

void canTask(CANTaskData canData) {
        checkCAN(canData);
}


void decodeMotorStats(CAN_message_t msg, MotorStats motorStats ) {
    *(measurementData.RPM) = (float) ((msg.buf[1] << 8) | msg.buf[0]);
    *(measurementData.motorCurrent) = ((msg.buf[3] << 8) | msg.buf[2]) / 10.0;
    *(measurementData.mainBatteryVoltage) = ((msg.buf[5] << 8) | msg.buf[4]) / 10.0;
    *(measurementData.errorMessage) = ((msg.buf[7] << 8) | msg.buf[6]);

    
}

void decodeMotorTemps(CAN_message_t msg, MotorTemps motorTemps) {
    *(motorTemps.throttle) = msg.buf[0] / 255.0;
    *(motorTemps.motorControllerTemperature) = msg.buf[1] - 40;
    *(motorTemps.motorTemperature) = msg.buf[2] - 30;
    *(motorTemps.controllerStatus) = msg.buf[4];
}


void decipherBMSStatus(CAN_message_t msg, BMSStatus bmsStatus) {
    *(bmsStatus.bms_status_flag) = msg.buf[0];
    *(bmsStatus.bms_c_id) = msg.buf[1];
    *(bmsStatus.bms_c_fault) = msg.buf[2];
    *(bmsStatus.ltc_fault) = msg.buf[3];
    *(bmsStatus.ltc_count) = msg.buf[4];
}
// A method for reading cell voltages that assumes a CAN message with only 4 cells.

void decipherCellsVoltage(CAN_message_t msg, CellVoltages cellVoltages, float *seriesVoltage) {
    // THE FOLLOWING DATATYPE NEEDS TO BE CHANGED
    uint32_t msgID = msg.id;
    int totalOffset = 0; // totalOffset equals the index of array cellVoltages
    int cellOffset = (((msgID >> 8) & 0xF) - 0x9);
    int ltcOffset = (msgID & 0x1);
    totalOffset = (cellOffset * 4) + (ltcOffset * 12);
    int cellIndex;
    for (cellIndex = 0; cellIndex < 8; cellIndex += 2) {
      // I'm questioning this new line
        *(cellVoltages.cellVoltages + (cellIndex / 2 + totalOffset)) = ((((float)(msg.buf[cellIndex + 1] << 8) + (float)(msg.buf[cellIndex]) / 10000)) / 10000) ;
    }
    calculateSeriesVoltage(cellVoltages, seriesVoltage);
}

void decipherThermistors(CAN_message_t msg, ThermistorTemps thermistorTemps) {
    int ltcID = msg.buf[0];
    thermistorEnabled = msg.buf[1];
    thermistorPresent = msg.buf[2];
    int *currentThermistor = msg.buf[3];
    int thermistor;
    for (thermistor = 0; thermistor < 4; thermistor++) {
        thermistorTemps.temps[thermistor + 5 * ltcID] = currentThermistor[thermistor];
    }
}

// a funtion to sum the voltage of each cell in main accumulator
void calculateSeriesVoltage(CellVoltages cellVs, float *seriesVoltage) {
  float partialSeriesVoltage = 0;
  int currentCell;
  for (currentCell = 0; currentCell < BMS_CELLS; currentCell++) {
    partialSeriesVoltage += *(cellVs.cellVoltages + currentCell);
  }
  *seriesVoltage = partialSeriesVoltage;
  Serial.print("Series voltage: ");Serial.println(*seriesVoltage);
}

// checks the can bus for any new data
void checkCAN(CANTaskData canData) {
    int readValue = CAN_bus.read(CAN_msg);
    if (readValue != 0) { // if we read a message
        switch (CAN_msg.id) {
        case MOTOR_STATS_MSG:
            decodeMotorStats(CAN_msg, canData.motorStats);
            break;
        case MOTOR_TEMPS_MSG:
            decodeMotorTemps(CAN_msg, canData.motorTemps);
            break;
        case DD_BMS_STATUS_IND:
            decipherBMSStatus(CAN_msg, canData.bmsStatus);
            //printBMSStatus();
            break;
        case BMSC1_LTC1_CELLS_04:
            decipherCellsVoltage(CAN_msg,  canData.cellVoltages, canData.seriesVoltage);
            break;
        case BMSC1_LTC1_CELLS_58:
            decipherCellsVoltage(CAN_msg,  canData.cellVoltages, canData.seriesVoltage);
            break;
        case BMSC1_LTC1_CELLS_912:
            decipherCellsVoltage(CAN_msg,  canData.cellVoltages, canData.seriesVoltage);
            break;
        case BMSC1_LTC2_CELLS_04:
            decipherCellsVoltage(CAN_msg,  canData.cellVoltages, canData.seriesVoltage);
            break;
        case BMSC1_LTC2_CELLS_58:
            decipherCellsVoltage(CAN_msg,  canData.cellVoltages, canData.seriesVoltage);
            break;
        case BMSC1_LTC2_CELLS_912:
            decipherCellsVoltage(CAN_msg,  canData.cellVoltages, canData.seriesVoltage);
            break;
        case DD_BMSC_TH_STATUS_IND:
            decipherThermistors(CAN_msg, canData.thermistorTemps);
            break;
        }
    }
}


void printBMSStatus() {
    switch (bms_status_flag) {
    case 1:
        Serial.printf("at least one cell V is > High Voltage Cutoff\n");
        break;
    case 2:
        Serial.printf("at least one cell V is < Low Voltage Cutoff\n");
        break;
    case 4:
        Serial.printf("at least one cell V is > Balance Voltage Cutoff\n");
        break;
    }
    Serial.printf("The BMSC ID is %d\n", bms_c_id);
    switch (bms_c_fault) {
    case 1:
        Serial.printf("BMS Fault: configuration not locked\n");
        break;
    case 2:
        Serial.printf("BMS Fault: not all cells present\n");
        break;
    case 4:
        Serial.printf("BMS Fault: thermistor overtemp\n");
        break;
    case 8:
        Serial.printf("BMS Fault: not all thermistors present\n");
        break;
    }
    if (ltc_fault == 1) {
        Serial.printf("LTC fault detected\n");
    }
    Serial.printf("%d LTCs detected\n");
}

void printMessage(CAN_message_t msg) {
    for (int i = 0; i < msg.len; i++) {
        Serial.print(msg.buf[i]);
        Serial.print(":");
    }
    Serial.println();
}

void requestCellVoltages(int LTC) {
    if (LTC == -1) {
        CAN_msg.id = 0x01de0800;
        CAN_bus.write(CAN_msg);
    } else if (LTC == 1) {
        CAN_msg.id = 0x01de0801;
        CAN_bus.write(CAN_msg);
    }
}
