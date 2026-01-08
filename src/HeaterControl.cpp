// HeaterControl.cpp
#include "HeaterControl.h"
#include "ScheduleService.h"
#include "TemperatureService.h"
#include <Arduino.h>
#include "config.h"


void HeaterControl_update()
{
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
    digitalWrite(HEATER_RELAY_PIN, LOW);
    Serial.println("🔥 Heater ON 🔥");
}

void Heater_off()
{
    digitalWrite(HEATER_RELAY_PIN, HIGH);
    Serial.println("🧊 Heater OFF 🧊");
}