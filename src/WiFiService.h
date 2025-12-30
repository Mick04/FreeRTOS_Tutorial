#pragma once
#include <Arduino.h>
#include "config.h"

// enum WiFiState {
//     WIFI_DISCONNECTED,
//     WIFI_CONNECTING,
//     WIFI_CONNECTED
// };

namespace WiFiService {
    void inti();
    WiFiState getState();
}