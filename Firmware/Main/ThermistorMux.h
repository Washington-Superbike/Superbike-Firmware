#pragma once

#include "FreeRTOS_TEENSY4.h"

#define MUX_TASK_STACK_SIZE configMINIMAL_STACK_SIZE + 32368

#define SELECT_A 14
#define SELECT_B 15

void switchSelects(byte *currentMuxSelects);
