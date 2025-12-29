#include <Arduino.h>
#include "config.h"
#include "blink1.h"
#include "blink2.h"

// Declare task handle
TaskHandle_t Blink1TaskHandle = NULL;
TaskHandle_t Blink2TaskHandle = NULL;



// Volatile variables for ISR
volatile bool taskSuspended = false;
volatile uint32_t lastInterruptTime = 0;
const uint32_t debounceDelay = 500; // debounce period

void IRAM_ATTR buttonISR() {
  // Debounce
  uint32_t currentTime = millis();
  if (currentTime - lastInterruptTime < debounceDelay) {
    return;
  }
  lastInterruptTime = currentTime;

  // Toggle task state
  taskSuspended = !taskSuspended;
  if (taskSuspended) {
    vTaskSuspend(Blink1TaskHandle);
    Serial.println("");
    Serial.println("=======================");
    Serial.println("BlinkTask Suspended");
    Serial.println("");
    Serial.print("=======================");
  } else {
    vTaskResume(Blink1TaskHandle);
      Serial.println("👹👹👹👹👹👹👹👹👹👹👹👹👹👹👹👹👹👹");
    Serial.println("=======================");
    Serial.println("BlinkTask Resumed");
    Serial.println("");
    Serial.println("=======================");
  }
} 

void setup() {
  Serial.begin(115200);
  if(!Serial) {
    // Wait for Serial to initialize
    delay(1000);
  }
  
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
  // Create Blink2 Task on Core 0
  xTaskCreatePinnedToCore(
    Blink1Task,         // Task function
    "Blink1Task",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &Blink1TaskHandle,  // Task handle
    1                  // Core 1
  );
  // Create Blink2 Task on Core 0
  xTaskCreatePinnedToCore(
    Blink2Task,         // Task function
    "Blink2Task",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &Blink2TaskHandle,  // Task handle
    0                  // Core 0
  );
}

void loop() {
    static uint32_t lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    Serial.printf("Free Heap: %u bytes\n", xPortGetFreeHeapSize());
    lastCheck = millis();
  // Empty because FreeRTOS scheduler runs the task
}
}