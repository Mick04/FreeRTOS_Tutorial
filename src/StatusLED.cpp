#include "StatusLED.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "WiFiService.h"

// Global variable definitions
CRGB leds[NUM_LEDS];
LEDStatus ledStates[NUM_LEDS];

void StatusLED_init()
{
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.show();

    // Initialize all LEDs to OFF
    for (int i = 0; i < NUM_LEDS; i++)
    {
        ledStates[i] = OFF;
    }
}
void updateLED(int index, LEDStatus status)
{
    switch (status)
    {
    case OFF:
        leds[index] = CRGB::Black;
        break;
    case GREEN:
        leds[index] = CRGB::Red;
        break;
    case RED:
        leds[index] = CRGB::Green;
        break;
    case ORANGE:
        leds[index] = CRGB::Orange;
        break;
    }
}

void StatusLED_Task(void *pvParameters)
{
    while (true)
    {
        // Update WiFi LED based on WiFi status
        WiFiState wifiStatus = WiFiService::getState();
        switch (wifiStatus)
        {
        case WIFI_DISCONNECTED:
            ledStates[WIFI_LED] = RED;
            break;
        case WIFI_CONNECTING:
            ledStates[WIFI_LED] = ORANGE;
            break;
        case WIFI_CONNECTED:
            ledStates[WIFI_LED] = GREEN;

            break;
        }

        for (int i = 0; i < NUM_LEDS; i++)
        {
            // Example: blink HEATER_LED if ORANGE
            if (i == HEATER_LED && ledStates[i] == ORANGE)
            {
                leds[i] = CRGB::Orange;
                FastLED.show();
                vTaskDelay(pdMS_TO_TICKS(500));
                leds[i] = CRGB::Black;
                FastLED.show();
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            else
            {
                updateLED(i, ledStates[i]);
            }
        }
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}