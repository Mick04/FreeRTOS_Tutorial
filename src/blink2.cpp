// blink2.cpp
#include <Arduino.h>
#include "blink2.h"
#include "config.h"

void Blink2Task(void *parameter)
{
  for (;;)
  {
    digitalWrite(LED2_PIN, LOW);
    Serial.println("blink2: LED2 ON");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(LED2_PIN, HIGH);
    Serial.println("blink2: LED2 OFF");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.print("Blink2Task running on core ");
    Serial.println(xPortGetCoreID());
    Serial.println("");
    Serial.println("-----------------------");
    Serial.printf("Task2 Stack Free: %u bytes\n", uxTaskGetStackHighWaterMark(NULL));
    Serial.println("");
    Serial.println("-----------------------");
  }
}