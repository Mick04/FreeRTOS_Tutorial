#pragma once
#include <Arduino.h>
#include "config.h"
#include <FastLED.h>

#define LED_PIN 25 // Pin for WS2811 LED strip

extern CRGB leds[NUM_LEDS];

// External declarations
extern LEDStatus ledStates[NUM_LEDS];

// Function prototypes
void StatusLED_init();
void StatusLED_Task(void *parameter);