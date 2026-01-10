// TemperatureService.cpp
#include "TemperatureService.h"
#include "HeaterControl.h"

static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature sensors(&oneWire);

// Shared data
static TemperatureData currentTemps;

// Mutex to protect it
static SemaphoreHandle_t tempMutex;

// Task handle
static TaskHandle_t tempTaskHandle = nullptr;

static void temperatureTask(void *pvParameters)
{
    sensors.begin();
    Serial.println("🌡️ Temperature Service started");
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

    TemperatureData previousTemps = {-999.0, -999.0, -999.0}; // ✅ Add this variable

    for (;;)
    {
        // ✅ Read all sensors at once
        sensors.requestTemperatures();

        TemperatureData temps;
        temps.heater = sensors.getTempCByIndex(0);
        temps.outside = sensors.getTempCByIndex(1);
        temps.coolside = sensors.getTempCByIndex(2);

        // ✅ Check if readings are valid (not -127 or 85)
        if (temps.heater > -127 && temps.heater < 85 &&
            temps.coolside > -127 && temps.coolside < 85 &&
            temps.outside > -127 && temps.outside < 85)
        {
            // Store temperatures
            if (xSemaphoreTake(tempMutex, portMAX_DELAY))
            {
                currentTemps = temps;
                xSemaphoreGive(tempMutex);
            }

            // ✅ Call heater control every temperature reading
            HeaterControl_update();

            // Check for significant changes
            if (abs(temps.heater - previousTemps.heater) >= 0.5)
            {
                Serial.printf("🌡️ Temps: H=%.2f°C C=%.2f°C O=%.2f°C\n",
                              temps.heater, temps.coolside, temps.outside);
                previousTemps = temps;
            }

#if defined(DEBUG) || defined(DEBUG_TEMPERATURE)
            // Serial.print("Outside: ");
            // Serial.println(temps.outside);
            // Serial.print("CoolSide: ");
            // Serial.println(temps.coolside);
            Serial.print("Heater: ");
            Serial.println(temps.heater);
#endif
        }
        else
        {
            Serial.println("❌ Invalid temperature readings - sensor error");
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // every 2 seconds
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