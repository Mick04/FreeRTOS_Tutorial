#include <Arduino.h>
#pragma once

// #define DEBUG // Uncomment to enable debug logs

// Global debug
// #define DEBUG_ALL

// Subsystem debug
#define DEBUG_WIFI
// #define DEBUG_MQTT
// #define DEBUG_FIREBASE
// #define DEBUG_LED
// #define DEBUG_HEATER

// =============================================================
//                         WiFi State Enum
// =============================================================
enum WiFiState
{
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED
};

// =============================================================
//                  Status LEDs enum
// =============================================================
enum LEDStatus
{
    OFF,
    GREEN,
    RED,
    ORANGE,
    BLUE
};
// =============================================================
//              Status LEDs Structure
// =============================================================

enum LEDIndex
{
    WIFI_LED = 0,
    MQTT_LED = 1,
    FIREBASE_LED = 2,
    HEATER_LED = 3
};

#define NUM_LEDS 4 // Number of LEDs in strip
extern LEDStatus ledStates[NUM_LEDS];

// struct StatusLED {
//     uint8_t pin;        // ESP32 GPIO connected to LED (if not using a data line for WS2811)
//     LEDStatus state;    // Current LED state
// };

// StatusLED leds[4];

// ================= Hardware Pins =================
#define HEATER_RELAY_PIN 26
#define ONEWIRE_BUS_PIN 27

//============================================================
//.             Potentiometer to LED Brightness Control
//                         end.
// ==============================================================

// // ================= MQTT =================
// #define MQTT_SERVER "test.mosquitto.org"
// #define MQTT_PORT   1883
// // #pragma once

// === MQTT (HiveMQ) Start ===
// #define MQTT_SERVER "broker.hivemq.com"
// #define MQTT_PORT 8884
// #define MQTT_USERNAME "ESP32FireBaseTortoise"
// #define MQTT_PASS "ESP32FireBaseHea1951Ter"