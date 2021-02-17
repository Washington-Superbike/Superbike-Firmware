// QUESTIONS:
// 1. DOES cellVoltages need to be longer than 20 for 20 cells? -g
// 2. DID WE DECIDE ON A GPIO FOR THE "DRIVING" MODE? - kaden
// 3. WHAT DO WE NEED TO DO WITH THE CELL VOLTAGE DATA. WHERE SHOULD I CALL AN AVERAGING VOLTAGE FUNCTION? -- depends on FSM state -- faster for precharging (time on sunday -- need to test)
//    DO I JUST ADD CELL VOLTAGES TO COMPARE TO BMS_DECLARED VOLTAGE? WHAT WILL CHANGE WHEN WE HAVE ALL CELLS?
// 4. HOW DO WE WANT TO STORE THERMISTER DATA?
// 5. HOW OFTEN DO WE WANT TO REQUEST VOLTAGE DATA? WHERE DO WE WANT TO CALL A FUNCTION THAT REQUESTS VOLTAGE DATA? -- need to test response time for LTC voltage data
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <elapsedMillis.h>
#include <FlexCAN_T4.h>
// WHAT DO THE NUMBERS AT THE END OF EACH LINE MEAN IN TERMS OF SYNTAX?
#define WHEEL_CIRCUMFERENCE 1 //for calculating speed
#define GEAR_RATIO 1 //for calculating rpm
#define MOTOR_MSG_1 0x0CF11E05 //motor controller message - CAN
#define MOTOR_MSG_2 0x0CF11F05 // motor controller message - CAN
#define LED_PIN 13 // pin for the teensy led (just for testing)
#define STANDBY 20 // when true, turns off the connection to the can bus from the Teensy
#define TFT_CS 21 //The display chip select pin
#define TFT_DC 19 // the display
#define TFT_RST 20 //the display reset pin
#define UPDATE_FREQUENCY 10 //the update frequency for loops on the teensy in milliseconds
#define ROLLING_AVERAGE_LEN 6 // the length of the array used for rolling average
#define CONTACTOR 17 //digital pin for contactor control
#define PRECHARGE_RELAY 18 //digital pin for pre-charge relay control
// ROUEN'S DEFINE STATMENTS: ADDED WINTER 2021
#define DD_BMS_STATUS_IND 0x01dd0001 // BMS cell data message
#define DD_BMSC_TH_STATUS_IND 0x01df0e00 // themistor message for BMSC
#define BMSC1_LTC1_CELLS_04  0x01df0900 // convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC1_CELLS_58  0x01df0a00
#define BMSC1_LTC1_CELLS_912 0x01df0b00
#define BMSC1_LTC2_CELLS_04  0x01df0901 // convention: BMSC, LTC, CELL RANGE
#define BMSC1_LTC2_CELLS_58  0x01df0a01
#define BMSC1_LTC2_CELLS_912 0x01df0b01
// The following are my temporary fields that can be removed later: Added winter 2021
int bms_status_flag=0;
int bms_c_id=0;
int bms_c_fault=0;
int ltc_fault=0;
int ltc_count=0;
float cellVoltages[24]; // voltages starting with the first LTC
float thermistorTemp[36]; // assuming a message with 7 LTCs
// THE FOLLOWING TWO DATATYPES NEED TO BE CHANGED
int thermistorEnabled; // assuming only 2 LTCs
int thermistorPresent; 
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); //the display controller
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN_bus; //access to can bus reading
// ELABORATE PLEASE
CAN_message_t CAN_msg; // data is read into this from the can bus
CAN_message_t CAN_send; // data that is sent into the can bus
// WHAT DOES "REMOVE THESE FIELDS" MEAN BELOW
//All of these represent the data returned to us from the CAN bus, we can remove these as fields later
//and have them be directly passed to the display update/ error functions later
float velocity=0;
float current_used=0; // current coming from motor controller
float battery_voltage=0;
float throttle=0;
float controller_temperature=0;
float motor_temperature=0;
byte controller_status=0;
byte switch_signals_status=0;
//used for refreshing display
elapsedMillis display=0; // WHAT IS DATATYPE elapsedMillis
uint16_t error_code=0; // WHAT IS THIS ERROR CODE USED FOR
float preChargeAverage[ROLLING_AVERAGE_LEN];
void checkForUpdates(){
  if(display>=50){
   //    updateDisplay();
       display=0;
    }
}
// EXPLAIN THE FOLLOWING CODE
void decipherMotorControllerStatusOne(CAN_message_t msg){
  velocity=((msg.buf[1]<<8)|msg.buf[0]);
  current_used=((msg.buf[3]<<8)|msg.buf[2])/10.0;
  battery_voltage=((msg.buf[5]<<8)|msg.buf[4])/10.0;
  error_code=((msg.buf[7]<<8)|msg.buf[6]);
}
void decipherMotorControllerStatusTwo(CAN_message_t msg){
  throttle=msg.buf[0]/255.0;
  controller_temperature=msg.buf[1]-40;
  motor_temperature=msg.buf[2]-30;
  controller_status=msg.buf[4];
  switch_signals_status=msg.buf[5];
}
// IS my datasheet bugged?
// A function to move the BMS statusInd information to global data fields. 
void decipherBMSStatus(CAN_message_t msg){
  bms_status_flag=msg.buf[0];
  bms_c_id=msg.buf[1];
  bms_c_fault=msg.buf[2];
  ltc_fault=msg.buf[3];
  ltc_count=msg.buf[4];
}
// A method for reading cell voltages that assumes a CAN message with only 4 cells. 
// consider genaralizing more by including a general message for every LTC and 
// cell group??
void decipherCellsVoltage(CAN_message_t msg) {
  // THE FOLLOWING DATATYPE NEEDS TO BE CHANGED
  uint32_t msgID = msg.id;
  int totalOffset = 0; // totalOffset equals the index of array cellVoltages
  int cellOffset = (((msgID >> 8) & 0xF) - 0x9);
  int ltcOffset = (msgID & 0x1);
  totalOffset = (cellOffset * 4) + (ltcOffset * 12);
  int cellIndex;
  Serial.print("MSG BUF:"); 
  for(int i=0; i<8; i++){
    Serial.print(msg.buf[i]); Serial.print(" ");
  }
  Serial.println();
  // this loop always reads voltage from the first 4 cells
  for (cellIndex=0;cellIndex < 8; cellIndex+=2) {
    cellVoltages[cellIndex/2 + totalOffset] = ((((float)(msg.buf[cellIndex+1] <<8) + (float)(msg.buf[cellIndex])/10000))/10000) ;
  }
}
void decipherThermistors(CAN_message_t msg) {
  int ltcID = msg.buf[0];
  thermistorEnabled = msg.buf[1];
  thermistorPresent = msg.buf[2];
  int *currentThermistor = msg.buf[3];
  int thermistor;
  for (thermistor = 0; thermistor < 4; thermistor++) {
    thermistorTemp[thermistor + 5*ltcID] = currentThermistor[thermistor];
  }
}
// Explain this code in more detail
//checks the can bus for any new data 
void checkCAN(){
  int readValue = CAN_bus.read(CAN_msg);
    if (readValue!=0){ // if we read a message
      switch (CAN_msg.id) {
        case MOTOR_MSG_1:
          decipherMotorControllerStatusOne(CAN_msg);
          break;
        case MOTOR_MSG_2:
          decipherMotorControllerStatusTwo(CAN_msg);
          break;
        case DD_BMS_STATUS_IND:
          decipherBMSStatus(CAN_msg);
          printBMSStatus();
          break;
        case BMSC1_LTC1_CELLS_04:
          decipherCellsVoltage(CAN_msg);
          break;
        case BMSC1_LTC1_CELLS_58:
          decipherCellsVoltage(CAN_msg);
          break;
        case BMSC1_LTC1_CELLS_912:
          decipherCellsVoltage(CAN_msg);
          break;
        case BMSC1_LTC2_CELLS_04:
          decipherCellsVoltage(CAN_msg);
          break;
        case BMSC1_LTC2_CELLS_58:
          decipherCellsVoltage(CAN_msg);
          break;
        case BMSC1_LTC2_CELLS_912:
          decipherCellsVoltage(CAN_msg);
          break;
        case DD_BMSC_TH_STATUS_IND:
          decipherThermistors(CAN_msg);
          break;
      }
//      if(CAN_msg.id==MOTOR_MSG_1){ //motor controller message
//        decipherMotorControllerStatusOne(CAN_msg); 
//      }else if(CAN_msg.id==MOTOR_MSG_2){ //motot controller message
//        decipherMotorControllerStatusTwo(CAN_msg);
//      } else if(CAN_msg.id==DD_BMS_STATUS_IND) { // BMS_STATUS_IND message
//        decipherBMSStatus(CAN_msg);
//        printBMSStatus(); // CONSIDER USING SWITCH STATEMENT, consider using mod for message id for generalized function
//      } else if(CAN_msg.id==BMSC1_LTC1_CELLS_04) { 
//        decipherCellsVoltage(CAN_msg);
//      } else if(CAN_msg.id==BMSC1_LTC1_CELLS_58) { 
//        decipherCellsVoltage(CAN_msg);
//      }else if(CAN_msg.id==BMSC1_LTC1_CELLS_912) { 
//        decipherCellsVoltage(CAN_msg);
//      }else if(CAN_msg.id==BMSC1_LTC2_CELLS_04) { 
//        decipherCellsVoltage(CAN_msg);
//      } else if(CAN_msg.id==BMSC1_LTC2_CELLS_58) { 
//        decipherCellsVoltage(CAN_msg);
//      } else if(CAN_msg.id==BMSC1_LTC2_CELLS_912) { 
//        decipherCellsVoltage(CAN_msg);
//      } else if(CAN_msg.id==DD_BMSC_TH_STATUS_IND) { 
//        decipherThermistors(CAN_msg);
//      }  
      else{
        Serial.print("MSG: ");
        for(int i=0; i<CAN_msg.len;i++){
          Serial.print(CAN_msg.buf[i]);
        }
        Serial.printf("ID: %x", CAN_msg.id);
        Serial.println();
      }
    }
    delay(500);
}
//haven't implemented display yet
void updateDisplay(){
  Serial.printf("vel: %.3f\n",velocity);
  Serial.printf("current: %.3f\n",current_used);
  Serial.printf("voltage: %.3f\n",battery_voltage);
  Serial.printf("error: %x\n",error_code);
  Serial.printf("throttle: %.3f\n", throttle);
  Serial.printf("controller temp: %.3f 'C\n", controller_temperature);
  Serial.printf("motor temp: %.3f 'C\n", motor_temperature);
  Serial.printf("controller status: %x\n", controller_status);
  Serial.printf("signals status: %x\n", switch_signals_status);
}
// I am converting hex to an int -- would this cause a problem?
// The following function prints the global data fields where the BMS CAN message information is stored. 
// This information is printed in a way that hoomans can understand easily. Information for the "ltc_fault" 
// is only printed if the fault is detected.
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
//    if (bms_status_flag==1) {
//      Serial.printf("at least one cell V is > High Voltage Cutoff\n");
//    }
//    else if (bms_status_flag==2) {
//      Serial.printf("at least one cell V is < Low Voltage Cutoff\n");
//    }
//    else if (bms_status_flag==4) {
//      Serial.printf("at least one cell V is > Balance Voltage Cutoff\n");
//    }
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
//    if (bms_c_fault==1) {
//      Serial.printf("BMS Fault: configuration not locked\n");
//    }
//    else if (bms_c_fault==2) {
//      Serial.printf("BMS Fault: not all cells present\n");
//    }
//    else if (bms_c_fault==4) {
//      Serial.printf("BMS Fault: thermistor overtemp\n");
//    }
//    else if (bms_c_fault==8) {
//      Serial.printf("BMS Fault: not all thermistors present\n");
//    }
    if (ltc_fault==1) {
      Serial.printf("LTC fault detected\n");
    }
    Serial.printf("%d LTCs detected\n");
}
void printMessage(const CAN_message_t &msg){
  for(int i=0;i<msg.len;i++){
      Serial.print(msg.buf[i]);
      Serial.print(":");
  }
  Serial.println();
}

IntervalTimer checkCANTimer;
volatile signed char checkCANFlag;
void checkCANisr(){
  checkCANFlag = 1;
}
IntervalTimer requestBMSVoltageTimer;
volatile signed char requestBMSVoltageFlag;
void requestBMSVoltageISR() {
  requestBMSVoltageFlag;
}
IntervalTimer updateDisplayTimer;
volatile signed char updateDisplayFlag;
void updateDisplayISR(){
  updateDisplayFlag = 1;
}
 
//object for the IntervalTimer interrupt, and flag variables
// we can use 4 of these for timer interrupts
IntervalTimer preChargeFSMTimer;
volatile signed char preChargeFlag; // needs to be volatile to avoid compiler issues
// Interrupt service routine for the precharge circuit
void tickPreChargeFSM() {
  preChargeFlag = 1;
}
// NOTE FOR DEBUGGING -- USE PRINT STATEMENTS
// read datasheet for charger and BMS -- CAN bus section -- data we are recieving -- write some code for recieving can bus information
//State machine code for the pre-charge circuit
// NOTE: "input" needs to change to the GPIO value for the on-button for the bike
// NOTE: FL mentioned using local variables for the states, consider where to initialize so that the states
// can be passed to the preChargeCircuitFSM function
enum PC_States { PC_SMstart, PC_open , PC_close, PC_justClosed } PC_State;
void preChargeCircuitFSM ()
{
   switch(PC_State) { // transitions
      case PC_SMstart:
         PC_State = PC_open;
         break;
      case PC_open:
        // when the GPIO for the bike's start switch is known, use: digitalRead(pin)
         if (0 == 1) {            //change 0 to the digital read
            PC_State = PC_close;
            break;
         }
         break;
//         else {
//            PC_State = PC_open;
//            break;
//         }
      case PC_close:
         if ( 0 ) {    //change to condition: motor controller voltage = BMS declared voltage
            PC_State = PC_close;
            break;
         }
         else {
            PC_State = PC_justClosed;
            break;
         }
      case PC_justClosed:
         PC_State = PC_justClosed;
         break;
      default:
         PC_State = PC_SMstart;
         break;
   } // transitions
   
   switch(PC_State) { // state actions
      case PC_open: 
         digitalWrite(CONTACTOR, LOW);
         digitalWrite(PRECHARGE_RELAY, LOW); // SANITY CHECK: DOES THIS OPEN THE PRECHARGE RELAY?
         break; 
      case PC_close: 
         // requestBMSVoltageISR.update( a faster time); 
         digitalWrite(CONTACTOR, LOW);
         digitalWrite(PRECHARGE_RELAY, HIGH);
         break;
      case PC_justClosed:
         // requestBMSVoltageISR.update( a slower time); 
         digitalWrite(CONTACTOR, HIGH); // WHAT DOES THIS LINE DO??? -- THIS MAY NEED CHANGING
         digitalWrite(PRECHARGE_RELAY, LOW);
         break;
      default:
         break;
   } // state actions
}
void requestCellVoltages(int LTC){
  if(LTC == -1){
    CAN_msg.id = 0x01de0800;
    CAN_bus.write(CAN_msg);
  }else if (LTC == 1){
  CAN_msg.id = 0x01de0801;
  CAN_bus.write(CAN_msg);
  }
}
void setup() {
  // THE FOLLOWING CODE GOES IN MAIN -- setup()
  CAN_bus.begin();
  CAN_bus.setBaudRate(250000);
  preChargeFlag = 0;
  PC_State = PC_SMstart;
  // start the prechargeFSM Timer, call ISR every 1 ms
  preChargeFSMTimer.priority(0); // highest priority
  preChargeFSMTimer.begin(tickPreChargeFSM, 1000);
  preChargeFSMTimer.priority(1); // highest priority
  preChargeFSMTimer.begin(requestBMSVoltageISR, 1000);
  preChargeFSMTimer.priority(0); // highest priority
  preChargeFSMTimer.begin(updateDisplayISR, 1000);
  preChargeFSMTimer.priority(0); // highest priority
  preChargeFSMTimer.begin(checkCANisr, 1000);
  Serial.setTimeout(20);
  
  while (1) {
    if (preChargeFlag) {
      // for testing purposes, the following line could be changed to blink an LED
      preChargeCircuitFSM();
      noInterrupts();
      preChargeFlag = 0;
      interrupts();
    }
    if (requestBMSVoltageFlag) {
      // this part probably needs work
      requestCellVoltages(-1); // LTC 1
      requestCellVoltages(1); // LTC 2
      noInterrupts();
      requestCellVoltageFlag = 0;
      interrupts();
    }
    if (updateDisplayFlag){
      // code function that updates the display
      noInterrupts();
      updateDisplayFlag = 0;
      interrupts();
    }
    if (checkCANFlag){
      checkCAN();
      noInterrupts();
      checkCANFlag = 0;
      interrupts();
    }
    requestCellVoltages(Serial.parseInt());
    checkCAN();
    float s = 0;
    for(int i=0; i<24;i++){
      s+=cellVoltages[i];
    }
    Serial.print("Series voltage: ");Serial.println(s);
  }
}
void loop(){
}
