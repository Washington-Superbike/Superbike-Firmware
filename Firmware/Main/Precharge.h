

#ifndef PRECHARGE_H_
#define PRECHARGE_H_

#define CONTACTOR 17 //digital pin for contactor control
#define PRECHARGE 18 //digital pin for relay in series with precharge resistor
#define BMS_CELLS 24 // the number of cells connected to the main accumulator BMS
#define BMS_DECLARED_VOLTAGE 80

extern volatile signed char preChargeFlag; 
enum PC_STATE { PC_START, PC_OPEN , PC_CLOSE, PC_JUST_CLOSED };

typedef struct PrechargeValues {
  float* seriesVoltage; // from the main accumulator
};

#endif
