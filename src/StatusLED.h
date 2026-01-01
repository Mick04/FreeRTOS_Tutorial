#pragma once
#include <Arduino.h>
#include "config.h"
#include <FastLED.h>

#define LED_PIN 25 // Pin for WS2811 LED strip

// LED indices for different statuses
#define NUM_LEDS 4  // Or however many you have
#define WIFI_LED 0
#define MQTT_LED 1  // Add this
#define HEATER_LED 2

extern CRGB leds[NUM_LEDS];

// External declarations
extern LEDStatus ledStates[NUM_LEDS];

// Function prototypes
void StatusLED_init();
void StatusLED_Task(void *parameter);