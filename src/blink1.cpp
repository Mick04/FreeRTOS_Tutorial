// blink1.cpp
#include <Arduino.h>
#include "blink1.h"
#include "config.h"

void Blink1Task(void *parameter)
{
  for (;;)
  {
    digitalWrite(LED1_PIN, HIGH);
    Serial.println("blink1: LED1 ON");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(LED1_PIN, LOW);
    Serial.println("blink1: LED1 OFF");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.print("Blink1Task running on core ");
    Serial.println(xPortGetCoreID());
    Serial.println("");
    Serial.println("-----------------------");
    Serial.printf("Task1 Stack Free: %u bytes\n", uxTaskGetStackHighWaterMark(NULL));
    Serial.println("");
    Serial.println("-----------------------");
  }
}