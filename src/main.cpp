#include <Arduino.h>
#include "WiFiService.h"
#include "StatusLED.h"
#include "config.h"

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

  // Create FreeRTOS task
  xTaskCreate(
      StatusLED_Task,
      "StatusLED",
      2048,
      NULL,
      1,
      NULL);
}

void loop()
{
  // Empty on purpose
  vTaskDelay(portMAX_DELAY);
}
//=============================================================
// Queue Example End
//=============================================================
