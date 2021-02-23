#include "CAN.h"
#include "Display.h"
#include "Main.h"


int bms_status_flag = 0;
int bms_c_id = 0;
int bms_c_fault = 0;
int ltc_fault = 0;
int ltc_count = 0;
float cellVoltages[24];         // voltages starting with the first LTC
float thermistorTemp[36];       // assuming a message with 7 LTCs
int thermistorEnabled;          // assuming only 2 LTCs
int thermistorPresent;


float velocity = 0;
float current_used = 0; // current coming from motor controller
float battery_voltage = 0;
float throttle = 0;
float controller_temperature = 0;
float motor_temperature = 0;
uint16_t error_code = 0;
byte controller_status = 0;
byte switch_signals_status = 0;




void setup() {
  return;
}
void loop() {
  return;
}
