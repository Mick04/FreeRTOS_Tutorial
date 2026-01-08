// // WiFiService.cpp
// #include "WiFiService.h"

// static WiFiState wifiState = WIFI_CONNECTING;
// static TaskHandle_t wifiTaskHandle = nullptr;

// static void WiFiTask(void *pvParameters)
// {
//     WiFi.begin("Gimp", "FC7KUNPX");
//     wifiState = WIFI_CONNECTING;

//     for (;;)
//     {

//         //==============================================
//         // ---- TEST HOOK (safe to remove later) ----
//         //==============================================
//         #if defined(DEBUG) || defined(DEBUG_WIFI)
//         if (Serial.available())
//         {
//             char c = Serial.read();
//             if (c == 'd')
//             {
//                 Serial.println("Forcing WiFi disconnect (test)");
//                 WiFi.disconnect(true);
//                 wifiState = WIFI_DISCONNECTED;
//                 vTaskDelay(3000);
//             }
//         }
//         #endif
//         //==============================================
//         // ---- TEST HOOK (safe to remove later) ----
//         //==============================================

//         if (WiFi.status() == WL_CONNECTED)
//         {
//             wifiState = WIFI_CONNECTED;
//             Serial.println("WiFi Connected");
//         }
//         else
//         {
//             wifiState = WIFI_CONNECTING;
//             Serial.println("WiFi Disconnected, attempting to reconnect...");
//             WiFi.disconnect();
//             WiFi.begin("Gimp", "FC7KUNPX");
//         }

//         vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds
//     }
// }
// void WiFiService::inti()
// {
//     xTaskCreatePinnedToCore(
//         WiFiTask,
//         "WiFiTask",
//         4096,
//         nullptr,
//         2,
//         &wifiTaskHandle,
//         0);
// }

// WiFiState WiFiService::getState()
// {
//     return wifiState;
// }


// // WiFiService.cpp

#include "WiFiService.h"

static WiFiState wifiState = WIFI_CONNECTING;
static WiFiState lastState = WIFI_CONNECTING;

static TaskHandle_t wifiTaskHandle = nullptr;

static void WiFiTask(void *pvParameters)
{
    WiFi.begin("Gimp", "FC7KUNPX");
    wifiState = WIFI_CONNECTING;

    for (;;)
    {
        #if defined(DEBUG) || defined(DEBUG_WIFI)
        if (Serial.available())
        {
            char c = Serial.read();
            if (c == 'd')
            {
                Serial.println("Forcing WiFi disconnect (test)");
                WiFi.disconnect(true);
                wifiState = WIFI_DISCONNECTED;
                vTaskDelay(pdMS_TO_TICKS(3000));
            }
        }
        #endif

        if (WiFi.status() == WL_CONNECTED)
        {
            wifiState = WIFI_CONNECTED;
        }
        else
        {
            wifiState = WIFI_DISCONNECTED;
            WiFi.disconnect();
            WiFi.begin("Gimp", "FC7KUNPX");
        }

        // Log only on change
        if (wifiState != lastState)
        {
            lastState = wifiState;

            switch (wifiState)
            {
                case WIFI_CONNECTED:
                    Serial.println("📶 WiFi CONNECTED");
                    break;
                case WIFI_DISCONNECTED:
                    Serial.println("📴 WiFi DISCONNECTED");
                    break;
                default:
                    Serial.println("📡 WiFi CONNECTING");
                    break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void WiFiService::init()
{
    xTaskCreatePinnedToCore(
        WiFiTask,
        "WiFiTask",
        4096,
        nullptr,
        2,
        &wifiTaskHandle,
        0
    );
}

WiFiState WiFiService::getState()
{
    return wifiState;
}