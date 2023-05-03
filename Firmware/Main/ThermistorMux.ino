// BMS sends the thermistor temperature messages every 1 second with
// alternating LTC IDs, and there are 4 sets of thermistors for the mux to switch
// between.

#include "ThermistorMux.h"
#include "FreeRTOS_TEENSY4.h"

#define delayScalar 1 // try 1/5?

void muxTask(void *currentMuxSelects) {
  while (1) {
    switchSelects((byte *)currentMuxSelects);
    vTaskDelay(configTICK_RATE_HZ * delayScalar);
  }
}

// rotates through setting the AB signals to the next element in the sequence
// 00 > 01 > 10 > 11 every time called, but skips one extra element ahead
// after every 4th second so that a message sending time from the BMS lines up with 
// with every combination of thermistor set and LTC ID
void switchSelects(byte *currentMuxSelects) {
  static float counter = 0;
  byte nextSelects = ((byte)counter) % 4;
  switch(nextSelects) {               // AB
    case 0:                           // 00
      digitalWrite(SELECT_A, LOW);  
      digitalWrite(SELECT_B, LOW);
      *currentMuxSelects = 0;
      break;
    case 1:                           // 01
      digitalWrite(SELECT_A, LOW);
      digitalWrite(SELECT_B, HIGH);
      *currentMuxSelects = 1;
      break;
    case 2:                           // 10
      digitalWrite(SELECT_A, HIGH);
      digitalWrite(SELECT_B, LOW);
      *currentMuxSelects = 2;
      break;
    case 3:                           // 11
      digitalWrite(SELECT_A, HIGH);
      digitalWrite(SELECT_B, HIGH);
      *currentMuxSelects = 3;
      break;
  }
  counter = counter + 1 + (1 / (4 * switchesPerSec)); // sum of (1 / (4 * switchesPerSec))'s will add up to 1 after 4 seconds
  if (counter >= 4 * (1 + (4 * switchesPerSec))) {    // 0 and RHS should be equivalent states for the counter (SORRY)
    counter = 0;
  }
}
