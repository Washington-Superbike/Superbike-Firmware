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
    digitalWrite(CONTACTOR_CONTROL, LOW);
}

void close_contactor() {
    digitalWrite(CONTACTOR_CONTROL, HIGH);
}

void open_precharge() {
    digitalWrite(PRECHARGE_CONTROL, LOW);
}

void close_precharge() {
    digitalWrite(PRECHARGE_CONTROL, HIGH);
}


void initGPIO() {

  /* TODO: remove CLOSE_CONTACTOR_BUTTON, LED outputs, TS_CS */

  pinMode(HIGH_VOLTAGE_TOGGLE, INPUT_PULLUP);

  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);

  pinMode(TS_CS, OUTPUT);
  digitalWrite(TS_CS, HIGH);

  pinMode(PRECHARGE_CONTROL, OUTPUT);
  open_precharge();
  pinMode(CONTACTOR_CONTROL, OUTPUT);
  open_contactor();

  pinMode(CAN_TX, OUTPUT);
  pinMode(CAN_RX, INPUT_PULLDOWN);
}
