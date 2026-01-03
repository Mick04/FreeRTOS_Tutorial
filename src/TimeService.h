#pragma once

#include <Arduino.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// -------------------- Types --------------------
enum TimeState_t {
    TIME_DISCONNECTED,
    TIME_SYNCING,
    TIME_READY
};

// -------------------- API --------------------
void TimeService_init();
TimeState_t TimeService_getState();
struct tm TimeService_getLocalTime();  // returns current local time with DST
String TimeService_getFormattedDateTime();