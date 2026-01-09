// HeaterControl.cpp
#include "HeaterControl.h"
#include "ScheduleService.h"
#include "TemperatureService.h"
#include <Arduino.h>
#include "config.h"

HeaterState heaterState = HEATER_STATE_OFF;
SemaphoreHandle_t heaterMutex = NULL;


void HeaterControl_init()
{
    heaterMutex = xSemaphoreCreateMutex();
    if (!heaterMutex)
    {
        Serial.println("❌ Failed to create Heater mutex!");
        return;
    }

    pinMode(HEATER_RELAY_PIN, OUTPUT);
    digitalWrite(HEATER_RELAY_PIN, HIGH);  // Start with heater OFF
        heaterState = HEATER_STATE_OFF;// Set initial state
    Serial.printf("🔥 Heater Control initialized on pin %d\n", HEATER_RELAY_PIN);
}

void HeaterControl_update()
{
      // ✅ Add safety check - don't run if services aren't ready
    if (!ScheduleService_isInitialized()) {
        Serial.println("⚠️ HeaterControl: ScheduleService not ready, skipping");
        return;
    }
    Serial.println("");
    Serial.println("⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️");
    Serial.println("HeaterControl_update called");
    Serial.println("⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️⭕️");
     Serial.println("");
    TemperatureData temps;
    if (!TemperatureService::getTemperatures(temps))
        return;

    float target = ScheduleService_getCurrentTarget();
Serial.printf("\n");
Serial.printf("❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌\n");
Serial.printf("Current Heater Temp: %.2f°C, Target: %.2f°C\n", temps.heater, target);
Serial.printf("❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌\n");
Serial.printf("\n");
    if (temps.heater < target - 0.3)
        Heater_on();
    else if (temps.heater > target + 0.3)
        Heater_off();
}


void Heater_on()
{
    if (xSemaphoreTake(heaterMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (heaterState != HEATER_STATE_ON) {  // Only switch if needed
            digitalWrite(HEATER_RELAY_PIN, LOW);
            heaterState = HEATER_STATE_ON;
            Serial.printf("🔥 Heater ON 🔥 (Pin %d = LOW)\n", HEATER_RELAY_PIN);
            Serial.printf("🔍 Pin state: %d\n", digitalRead(HEATER_RELAY_PIN));
        }
        xSemaphoreGive(heaterMutex);
    }
}

void Heater_off()
{
    if (xSemaphoreTake(heaterMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (heaterState != HEATER_STATE_OFF) {  // Only switch if needed
            digitalWrite(HEATER_RELAY_PIN, HIGH);
            heaterState = HEATER_STATE_OFF;
            Serial.printf("🧊 Heater OFF 🧊 (Pin %d = HIGH)\n", HEATER_RELAY_PIN);
            Serial.printf("🔍 Pin state: %d\n", digitalRead(HEATER_RELAY_PIN));
        }
        xSemaphoreGive(heaterMutex);
    }
}

// Add this function after your existing functions
HeaterState HeaterControl_getState()
{
    HeaterState state = HEATER_STATE_OFF;  // Default
    
    if (heaterMutex != NULL && xSemaphoreTake(heaterMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        state = heaterState;
        xSemaphoreGive(heaterMutex);
    }
    
    return state;
}