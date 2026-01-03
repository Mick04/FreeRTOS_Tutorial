// main.cpp
#include <Arduino.h>
#include "WiFiService.h"
#include "StatusLED.h"
#include "TemperatureService.h"
#include "config.h"
#include "MQTTService.h"
#include "FirebaseService.h"
#include "TimeService.h"

void setup()
{
  Serial.begin(115200);
  if (!Serial)
  {
    // Wait for Serial to initialize
    delay(1000);
  }
  WiFiService::inti();
  vTaskDelay(100);
  StatusLED_init();
  TemperatureService::init();
  MQTTService_init();
  FirebaseService_init();
  TimeService_init();
}

void loop()
{
  // Empty on purpose
  vTaskDelay(portMAX_DELAY);
}
//=============================================================
// Queue Example End
//=============================================================
