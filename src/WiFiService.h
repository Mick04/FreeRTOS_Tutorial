// WiFiService.h
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

// Functions
String TimeService_getIsoTimestamp();
uint32_t TimeService_getEpoch();

// enum WiFiState {
//     WIFI_DISCONNECTED,
//     WIFI_CONNECTING,
//     WIFI_CONNECTED
// };

namespace WiFiService {
    void inti();
    WiFiState getState();
}
