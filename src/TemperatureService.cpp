#include "TemperatureService.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "config.h"

// #define ONE_WIRE_BUS 4 // change to your GPIO

static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature sensors(&oneWire);

// Shared data
static TemperatureData currentTemps;

// Mutex to protect it
static SemaphoreHandle_t tempMutex;

// Task handle (optional, but useful later)
static TaskHandle_t tempTaskHandle = nullptr;

static void temperatureTask(void *pvParameters)
{
    sensors.begin();
    Serial.println("Found sensors:");
    for (int i = 0; i < sensors.getDeviceCount(); i++)
    {
        DeviceAddress addr;
        sensors.getAddress(addr, i);
        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        for (uint8_t j = 0; j < 8; j++)
        {
            if (addr[j] < 16)
                Serial.print("0");
            Serial.print(addr[j], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    for (;;)
    {
        sensors.requestTemperatures();
        float heater = sensors.getTempCByIndex(0);
        float coolside = sensors.getTempCByIndex(1);
        float outside = sensors.getTempCByIndex(2);

        // Protect write access
        if (xSemaphoreTake(tempMutex, portMAX_DELAY))
        {
            currentTemps.outside = outside;
            currentTemps.coolside = coolside;
            currentTemps.heater = heater;
            xSemaphoreGive(tempMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // every 2 seconds

#if defined(DEBUG) || defined(DEBUG_TEMPERATURE)
        TemperatureData temps;

        if (TemperatureService::getTemperatures(temps))
        {
            Serial.print("Outside: ");
            Serial.println(temps.outside);
            Serial.print("CoolSide: ");
            Serial.println(temps.coolside);
            Serial.print("Heater: ");
            Serial.println(temps.heater);
        }
        else
        {
            Serial.println("Could not get temperatures (mutex busy)");
        }
#endif
    }
}

void TemperatureService::init()
{
    tempMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        temperatureTask,
        "TempTask",
        4096,
        nullptr,
        1,
        &tempTaskHandle,
        1);
}

bool TemperatureService::getTemperatures(TemperatureData &out)
{
    if (xSemaphoreTake(tempMutex, pdMS_TO_TICKS(50)))
    {
        out = currentTemps; // struct copy
        xSemaphoreGive(tempMutex);
        return true;
    }
    return false;
}