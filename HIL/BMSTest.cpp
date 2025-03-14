#include <FlexCAN_T4.h>

// // CAN Message IDs
// #define BMSC1_LTC1_CELLS_04  0x01df0900
// /// Convention: BMSC, LTC, CELL RANGE
// #define BMSC1_LTC1_CELLS_58  0x01df0a00
// /// Convention: BMSC, LTC, CELL RANGE
// #define BMSC1_LTC1_CELLS_912 0x01df0b00
// /// Convention: BMSC, LTC, CELL RANGE
// #define BMSC1_LTC2_CELLS_04  0x01df0901
// /// Convention: BMSC, LTC, CELL RANGE
// #define BMSC1_LTC2_CELLS_58  0x01df0a01
// /// Convention: BMSC, LTC, CELL RANGE
// #define BMSC1_LTC2_CELLS_912 0x01df0b01
// /* BMS Request Cell Voltages LTC 1 */
// #define BMSC1_LTC1_REQUEST_CELLS 0x01de0800
// /* BMS Request Cell Voltages LTC 2 */
// #define BMSC1_LTC2_REQUEST_CELLS 0x01de0801

// BMS CAN Message IDs
#define DD_BMSC_STATUS_IND          0x01df0e01  // BMS status indication
#define DD_BMSC_TH_STATUS_IND       0x01df0e00  // BMS thermistor readings
#define DD_BMS_CVCUR_REQ            0x01de0800  // REQUEST  cell voltages and currents (LTC1)
#define DD_BMS_CVCUR_C1_TO_C4_RSP   0x01df0900  // RESPONSE cell voltages and currents (cells 1-4)
#define DD_BMS_CVCUR_C5_TO_C8_RSP   0x01df0a00  // RESPONSE cell voltages and currents (cells 5-8)
#define DD_BMS_CVCUR_C9_TO_C12_RSP  0x01df0b00  // RESPONSE cell voltages and currents (cells 9-12)
// For LTC2, just OR with 0x01 to change the ID
 
// BMS Fault Codes For DD_BMSC_STATUS_IND
// BMS status flag definitions (msg.buf[0])
#define BMS_FLAG_CELL_HVC        0x01 // at least one cell voltage is > HVC 
#define BMS_FLAG_CELL_LVC        0x02 // at least one cell voltage is < LVC 
#define BMS_FLAG_CELL_BVC        0x04 // at least one cell voltage is > BVC 
 
// BMS flag definitions (msg.buf[1])
#define BMS_FAULT_NOT_LOCKED     0x01 // configuration not locked
#define BMS_FAULT_CENSUS         0x02 // not all cells present
#define BMS_FAULT_OVERTEMP       0x04 // thermistor overtemp
#define BMS_FAULT_THERM_CENSUS   0x08 // not all thermistors present
 
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> CAN_2;
 
void setup(void) {
  Serial.begin(115200); delay(400);
  CAN_2.begin();
  CAN_2.setBaudRate(250000);
  CAN_2.setMaxMB(16);
  CAN_2.enableFIFO();
  CAN_2.enableFIFOInterrupt();
  CAN_2.onReceive(canSniff);
  CAN_2.mailboxStatus();

  // Send faults here... or in loop 
}


// Handles reception of messages over CAN bus
void canSniff(const CAN_message_t &rec_msg) {
  CAN_message_t send_msg;
  send_msg.flags.extended = 1;
  uint16_t *wbuf = (uint16_t *)send_msg.buf;
  for (int i = 0; i < 4; i++) {
      wbuf[i] = 40000;
  }
  Serial.println(rec_msg.id, HEX);

  // Cell voltages were requested, send them back over CAN
  if (rec_msg.id == DD_BMS_CVCUR_REQ) {
    send_msg.id = DD_BMS_CVCUR_C1_TO_C4_RSP;
    CAN_2.write(send_msg);
    send_msg.id = DD_BMS_CVCUR_C5_TO_C8_RSP;
    CAN_2.write(send_msg);
    send_msg.id = DD_BMS_CVCUR_C9_TO_C12_RSP;
    CAN_2.write(send_msg);
  } else if (rec_msg.id == (DD_BMS_CVCUR_REQ | 0x01)) {
    // OR with 0x01 to change ltc from 1 to 2
    send_msg.id = DD_BMS_CVCUR_C1_TO_C4_RSP | 0x01;
    CAN_2.write(send_msg);
    send_msg.id = DD_BMS_CVCUR_C5_TO_C8_RSP | 0x01;
    CAN_2.write(send_msg);
    send_msg.id = DD_BMS_CVCUR_C9_TO_C12_RSP | 0x01;
    CAN_2.write(send_msg);
  }
}

 
void loop() {
  CAN_2.events();
 
  static uint32_t timeout = millis();
  if ( millis() - timeout > 1 ) {
    // send a message every second?   
  }
}

/* FOR REFERENCE:
 *
 * typedef struct CAN_message_t {
 *   uint32_t id = 0;         // can identifier
 *   uint16_t timestamp = 0;  // FlexCAN time when message arrived
 *   uint8_t idhit = 0;       // filter that id came from
 *   struct {
 *     bool extended = 0;     // identifier is extended (29-bit)
 *     bool remote = 0;       // remote transmission request packet type
 *     bool overrun = 0;      // message overrun
 *     bool reserved = 0;
 *   } flags;
 *   uint8_t len = 8;         // length of data
 *   uint8_t buf[8] = { 0 };  // data
 *   int8_t mb = 0;           // used to identify mailbox reception
 *   uint8_t bus = 0;         // used to identify where the message came from when events() is used.
 *   bool seq = 0;            // sequential frames
 * } CAN_message_t;
 * 
 */

void sendNormal() {
  CAN_message_t msg;
  msg.flags.extended = 1;

  msg.id = DD_BMSC_STATUS_IND;
  msg.len = 4;
  msg.buf[0] = 0;  // no voltage faults
  msg.buf[1] = 0;  // no thermistor faults
  msg.buf[2] = 0;  // unused?
  msg.buf[3] = 0;  // unused?

  // Send message
  CAN_2.write(msg);

  delay(1000);

  // call method to send actual temps for all 8 LTCs
  // should send all 40 thermistors at temp 30 C
  for (int i = 0; i < 8; i++) {
    sendThermistorTemps(i, false);
    delay(1000);
  }
}

void sendOvertempFault() {
  CAN_message_t msg;
  msg.flags.extended = 1;

  msg.id = DD_BMSC_STATUS_IND;
  msg.len = 4;
  msg.buf[0] = 0;
  msg.buf[1] = BMS_FAULT_OVERTEMP;  // thermistor overtemp fault
  msg.buf[2] = 0;
  msg.buf[3] = 0;

  // Send message
  CAN_2.write(msg);

  delay(1000);

  // call method to send actual temps for all 8 LTCs
  for (int i = 0; i < 8; i++) {
    sendThermistorTemps(i, true);
    delay(1000);
  }
}

// Send thermistor raw temps
// ltcID: 0-7
// Sends a message which indicates 5 present thermistors
// with 1 thermistor at 60 degrees Celsius (if overtemp is true)
// and 4 thermistors at 30 degrees Celsius
// if overtemp is false, all thermistors are at 30 C
void sendThermistorTemps(int ltcID, bool overtemp) {
  CAN_message_t msg;
  msg.flags.extended = 1;

  msg.id = DD_BMSC_TH_STATUS_IND;
  msg.length = 8;

  // Fill the message buffer
  msg.buf[0] = ltcID;       // bLtcIdx: Response from first LTC (index 0)
  msg.buf[1] = 0b00011111;  // bThEnabled: All five thermistors are enabled
  msg.buf[2] = 0b00011111;  // bThPresent: All five thermistors are present
  msg.buf[3] = overtemp ? 60 : 30;  // bThTemp[0]: Temperature of first thermistor (60 C)
  msg.buf[4] = 30;          // bThTemp[1]: 30 C
  msg.buf[5] = 30;          // bThTemp[2]: 30 C
  msg.buf[6] = 30;          // bThTemp[3]: 30 C
  msg.buf[7] = 30;          // bThTemp[4]: 30 C

  // Send the message
  CAN_2.write(msg);
}

void sendHVCFault() {
  CAN_message_t msg;
  msg.flags.extended = 1;

  msg.id = DD_BMSC_STATUS_IND;
  msg.len = 4;
  msg.buf[0] = BMS_FLAG_CELL_HVC;  // at least one cell voltage is > HVC
  msg.buf[1] = 0;
  msg.buf[2] = 0;
  msg.buf[3] = 0;

  // Send message
  CAN_2.write(msg);
}

void sendLVCFault() {
  CAN_message_t msg;
  msg.flags.extended = 1;

  msg.id = DD_BMSC_STATUS_IND;
  msg.len = 4;
  msg.buf[0] = BMS_FLAG_CELL_LVC;  // at least one cell voltage is < LVC
  msg.buf[1] = 0;
  msg.buf[2] = 0;
  msg.buf[3] = 0;

  // Send message
  CAN_2.write(msg);
}

void sendBVCFault() {
  CAN_message_t msg;
  msg.flags.extended = 1;

  msg.id = DD_BMSC_STATUS_IND;
  msg.len = 4;
  msg.buf[0] = BMS_FLAG_CELL_BVC;  // at least one cell voltage is > BVC
  msg.buf[1] = 0;
  msg.buf[2] = 0;
  msg.buf[3] = 0;

  // Send message
  CAN_2.write(msg);
}

void sendUnlockedFault() {
  CAN_message_t msg;
  msg.flags.extended = 1;

  msg.id = DD_BMSC_STATUS_IND;
  msg.len = 4;
  msg.buf[0] = 0;
  msg.buf[1] = BMS_FAULT_NOT_LOCKED;  // configuration not locked
  msg.buf[2] = 0;
  msg.buf[3] = 0;

  // Send message
  CAN_2.write(msg);
}

void sendCensusFault() {
  CAN_message_t msg;
  msg.flags.extended = 1;

  msg.id = DD_BMSC_STATUS_IND;
  msg.len = 4;
  msg.buf[0] = 0;
  msg.buf[1] = BMS_FAULT_CENSUS;  // not all cells present
  msg.buf[2] = 0;
  msg.buf[3] = 0;

  // Send message
  CAN_2.write(msg);
}

void sendThermistorCensusFault() {
  CAN_message_t msg;
  msg.flags.extended = 1;

  msg.id = DD_BMSC_STATUS_IND;
  msg.len = 4;
  msg.buf[0] = 0;
  msg.buf[1] = BMS_FAULT_THERM_CENSUS;  // not all thermistors present
  msg.buf[2] = 0;
  msg.buf[3] = 0;

  // Send message
  CAN_2.write(msg);
}

