// HeaterControl.h
// #pragma once
#ifndef HEATER_CONTROL_H
#define HEATER_CONTROL_H    


#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// =============================================================
//                       Heater State Enum
// =============================================================
typedef enum HeaterState { 
    HEATER_STATE_ON,
    HEATER_STATE_OFF
}HeaterState;

void HeaterControl_init();
void HeaterControl_update();
void Heater_on();
void Heater_off();
HeaterState HeaterControl_getState(); // ✅ Add getter declaration

bool ScheduleService_isInitialized();

#endif // HEATER_CONTROL_H
