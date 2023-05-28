#pragma once

#include "CAN.h"
#include "DataLogging.h"
#include "Precharge.h"

typedef struct _Context {
    MotorStats motor_stats;
    MotorTemps motor_temps;
    ChargeControllerStats charge_controller_stats;
    ChargerStats charger_stats;
    BMSStatus bms_status;
    ThermistorTemps thermistor_temps;
    BatteryVoltages battery_voltages;
    CSVWriter logs[CONFIG_LOG_COUNT];
    GyroKalman gyro_kalman;
    bool sd_started;
} Context;
