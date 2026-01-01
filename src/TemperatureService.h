#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "config.h"

// Logical names for your sensors
enum TempSensorId {
    TEMP_OUTSIDE = 0,
    TEMP_COOLSIDE,
    TEMP_HEATER,
    TEMP_COUNT
};

// Structure holding all temperatures
struct TemperatureData {
    float outside;
    float coolside;
    float heater;
};

namespace TemperatureService {
    void init();
    bool getTemperatures(TemperatureData &out);
}