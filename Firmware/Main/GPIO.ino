/*
* Provides interfaces for reading/writing from commonly used GPIO pins
* TODO: add aux battery voltage analog read
*/

#include "GPIO.h"
#include "Display.h"

bool check_HV_toggle() {
  return !digitalRead(HIGH_VOLTAGE_TOGGLE);
}

void open_contactor() {
    digitalWrite(CONTACTOR_CONTROL, arduino::LOW);
}

void close_contactor() {
    digitalWrite(CONTACTOR_CONTROL, arduino::HIGH);
}

void open_precharge() {
    digitalWrite(PRECHARGE_CONTROL, arduino::LOW);
}

void close_precharge() {
    digitalWrite(PRECHARGE_CONTROL, arduino::HIGH);
}


void initGPIO() {

  /* TODO: remove CLOSE_CONTACTOR_BUTTON, LED outputs, TS_CS */

  pinMode(HIGH_VOLTAGE_TOGGLE, arduino::INPUT_PULLUP);

  pinMode(TFT_RST, arduino::OUTPUT);
  digitalWrite(TFT_RST, arduino::HIGH);
  pinMode(TFT_CS, arduino::OUTPUT);
  digitalWrite(TFT_CS, arduino::HIGH);

  pinMode(TS_CS, arduino::OUTPUT);
  digitalWrite(TS_CS, arduino::HIGH);

  pinMode(PRECHARGE_CONTROL, arduino::OUTPUT);
  open_precharge();
  pinMode(CONTACTOR_CONTROL, arduino::OUTPUT);
  open_contactor();

  pinMode(CAN_TX, arduino::OUTPUT);
  pinMode(CAN_RX, arduino::INPUT_PULLDOWN);
}
