// config.h
#include <Arduino.h>
#pragma once

// #define DEBUG // Uncomment to enable debug logs

// Global debug
// #define DEBUG_ALL

// Subsystem debug
//#define DEBUG_WIFI
////#define DEBUG_TEMPERATURE
//#define DEBUG_MQTT
//#define DEBUG_FIREBASE
// #define DEBUG_LED
// #define DEBUG_HEATER

// Track previous connection states for status publishing
static int prevWifi = -1;
// static int prevMqtt = -1;
// static int prevFirebase = -1;

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
//                         MQTT State Enum
// =============================================================
enum MQTTState
{
    MQTT_STATE_DISCONNECTED,
    MQTT_STATE_CONNECTING,
    MQTT_STATE_CONNECTED
};

//============================================================
//                  Firebase State Enum
//============================================================
enum FirebaseState
{
    FIREBASE_DISCONNECTED,
    FIREBASE_CONNECTING,
    FIREBASE_CONNECTED
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

// ================= Hardware Pins =================
#define HEATER_RELAY_PIN 26
#define ONE_WIRE_BUS 27

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