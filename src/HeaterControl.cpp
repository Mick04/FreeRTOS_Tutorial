// HeaterControl.cpp
#include "HeaterControl.h"
#include "ScheduleService.h"
#include "TemperatureService.h"
#include <Arduino.h>
#include "config.h"

void HeaterControl_init()
{
    pinMode(HEATER_RELAY_PIN, OUTPUT);
    digitalWrite(HEATER_RELAY_PIN, LOW);  // Start with heater OFF
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
    digitalWrite(HEATER_RELAY_PIN, LOW);// LOW = ON for relay
    Serial.printf("🔥 Heater ON 🔥 (Pin %d = LOW)\n", HEATER_RELAY_PIN);
    Serial.printf("🔍 Pin state: %d\n", digitalRead(HEATER_RELAY_PIN));
}

void Heater_off()
{
    digitalWrite(HEATER_RELAY_PIN, HIGH);// HIGH = OFF for relay
    Serial.printf("🧊 Heater OFF 🧊 (Pin %d = HIGH)\n", HEATER_RELAY_PIN);
    Serial.printf("🔍 Pin state: %d\n", digitalRead(HEATER_RELAY_PIN));
}