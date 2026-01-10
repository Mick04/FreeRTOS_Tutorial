// StatusLED.cpp
#include "StatusLED.h"
#include "HeaterControl.h"

// Global variable definitions
CRGB leds[NUM_LEDS];
LEDStatus ledStates[NUM_LEDS];
static WiFiState lastWifiStatus = WIFI_DISCONNECTED;
static MQTTState lastMqttStatus = MQTT_STATE_DISCONNECTED; // Add this

void StatusLED_init()
{
    FastLED.addLeds<WS2812, LED_PIN, RGB>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.show();

    Serial.printf("🎨 StatusLED initialized: %d LEDs on pin %d\n", NUM_LEDS, LED_PIN);

    // Initialize all LEDs to OFF
    for (int i = 0; i < NUM_LEDS; i++)
    {
        ledStates[i] = ORANGE;
    }
    // Create FreeRTOS task
    xTaskCreate(
        StatusLED_Task,
        "StatusLED",
        2048,
        NULL,
        1,
        NULL);
}
void updateLED(int index, LEDStatus status)
{
    switch (status)
    {
    case OFF:
        leds[index] = CRGB::Black;
        break;
    case GREEN:
        leds[index] = CRGB::Green;
        break;
    case RED:
        leds[index] = CRGB::Red;
        break;
    case ORANGE:
        leds[index] = CRGB::Orange;
        break;
    case BLUE:
        leds[index] = CRGB::Blue;
        break;
    }
}

void StatusLED_Task(void *pvParameters)
{
    static unsigned long wifiConnectingStart = 0;
    static unsigned long mqttConnectingStart = 0;
    static unsigned long firebaseConnectingStart = 0;
    static FirebaseState lastFirebaseState = FIREBASE_DISCONNECTED;

    const unsigned long MIN_CONNECTING_TIME = 10000; // 10 seconds

    // ✅ Initialize LED states based on current service status
    Serial.println("🎨 Initializing LED states...");
    WiFiState wifiStatus = WiFiService::getState();
    MQTTState mqttStatus = MQTTService_getState();
    FirebaseState firebaseStatus = FirebaseService_getState();
    HeaterState heaterStatus = HeaterControl_getState();
    
    // Set initial LED states
    switch (wifiStatus) {
        case WIFI_CONNECTED: ledStates[WIFI_LED] = GREEN; break;
        case WIFI_CONNECTING: ledStates[WIFI_LED] = BLUE; break;
        default: ledStates[WIFI_LED] = RED; break;
    }
    
    switch (mqttStatus) {
        case MQTT_STATE_CONNECTED: ledStates[MQTT_LED] = GREEN; break;
        case MQTT_STATE_CONNECTING: ledStates[MQTT_LED] = BLUE; break;
        default: ledStates[MQTT_LED] = RED; break;
    }
    
    switch (firebaseStatus) {
        case FIREBASE_CONNECTED: ledStates[FIREBASE_LED] = GREEN; break;
        case FIREBASE_CONNECTING: ledStates[FIREBASE_LED] = BLUE; break;
        default: ledStates[FIREBASE_LED] = RED; break;
    }
    
    switch (heaterStatus) {
        case HEATER_STATE_ON: ledStates[HEATER_LED] = RED; break;   // RED = ON
        default: ledStates[HEATER_LED] = GREEN; break;              // GREEN = OFF
    }
    
    // Update last states to current so changes are detected
    lastWifiStatus = wifiStatus;
    lastMqttStatus = mqttStatus; 
    lastFirebaseState = firebaseStatus;
    
    Serial.printf("🎨 Initial LED Status: WiFi=%s MQTT=%s Firebase=%s Heater=%s\n",
        (ledStates[WIFI_LED] == GREEN ? "GREEN" : ledStates[WIFI_LED] == RED ? "RED" : "BLUE"),
        (ledStates[MQTT_LED] == GREEN ? "GREEN" : ledStates[MQTT_LED] == RED ? "RED" : "BLUE"),
        (ledStates[FIREBASE_LED] == GREEN ? "GREEN" : ledStates[FIREBASE_LED] == RED ? "RED" : "BLUE"),
        (ledStates[HEATER_LED] == GREEN ? "GREEN" : "RED"));


    while (true)
    {
        // Update WiFi LED based on WiFi status
        WiFiState wifiStatus = WiFiService::getState();
        if (wifiStatus != lastWifiStatus)
        {

            switch (wifiStatus)
            {
            case WIFI_DISCONNECTED:
                Serial.println("");
                Serial.println("👄👄👄👄👄👄👄👄👄👄👄");
                Serial.println("WiFi Disconnected");
                Serial.println("👄👄👄👄👄👄👄👄👄👄👄");
                Serial.println("");
                ledStates[WIFI_LED] = RED;
                wifiConnectingStart = 0; // Reset timer
                break;
            case WIFI_CONNECTING:
                Serial.println("");
                Serial.println("🔶🔶🔶🔶🔶🔶🔶🔶🔶🔶🔶");
                Serial.println("WiFi Connecting");
                Serial.println("🔶🔶🔶🔶🔶🔶🔶🔶🔶🔶🔶");
                Serial.println("");
                ledStates[WIFI_LED] = BLUE;
                wifiConnectingStart = millis(); // ✅ Start timer
                break;
            case WIFI_CONNECTED:
                // ✅ Only turn green if enough time has passed
                if (wifiConnectingStart == 0 || (millis() - wifiConnectingStart) >= MIN_CONNECTING_TIME)
                {
                    Serial.println("📶 WiFi CONNECTED");
                    ledStates[WIFI_LED] = GREEN;
                    wifiConnectingStart = 0; // Reset timer
                }
                else
                {
                    // Keep blue until minimum time passes
                    ledStates[WIFI_LED] = BLUE;
                     MIN_CONNECTING_TIME - (millis() - wifiConnectingStart);
                }
                break;
            }
            lastWifiStatus = wifiStatus;
        }
        // Check if WiFi connected but still in blue phase
        if (wifiStatus == WIFI_CONNECTED && wifiConnectingStart > 0 &&
            (millis() - wifiConnectingStart) >= MIN_CONNECTING_TIME &&
            ledStates[WIFI_LED] == BLUE)
        {
            Serial.println("📶 WiFi CONNECTED (timer expired)");
            ledStates[WIFI_LED] = GREEN;
            wifiConnectingStart = 0; // Reset timer
        }

        MQTTState mqttStatus = MQTTService_getState(); // Use the new function

        if (mqttStatus != lastMqttStatus)
        {
            switch (mqttStatus)
            {
            case MQTT_STATE_DISCONNECTED:
                ledStates[MQTT_LED] = RED; // You'll need to define MQTT_LED
                mqttConnectingStart = 0;   // Reset timer
                break;
            case MQTT_STATE_CONNECTING:
                ledStates[MQTT_LED] = BLUE;
                mqttConnectingStart = millis(); // ✅ Start timer
                break;
            case MQTT_STATE_CONNECTED:
                // ✅ Only turn green if enough time has passed
                if (mqttConnectingStart == 0 || (millis() - mqttConnectingStart) >= MIN_CONNECTING_TIME)
                {
                    ledStates[MQTT_LED] = GREEN;
                    mqttConnectingStart = 0; // Reset timer
                }
                else
                {
                    // Keep blue until minimum time passes
                    ledStates[MQTT_LED] = BLUE;
                }
                break;
            }
            lastMqttStatus = mqttStatus;
        }

        // ✅ Check MQTT timer expiration
        if (mqttStatus == MQTT_STATE_CONNECTED && mqttConnectingStart > 0 &&
            (millis() - mqttConnectingStart) >= MIN_CONNECTING_TIME &&
            ledStates[MQTT_LED] == BLUE)
        {
            ledStates[MQTT_LED] = GREEN;
            mqttConnectingStart = 0; // Reset timer
        }

        // static FirebaseState lastFirebaseState = FIREBASE_DISCONNECTED; // ✅ Local tracking variable
        FirebaseState currentFirebaseState = FirebaseService_getState();
        if (currentFirebaseState != lastFirebaseState)
        {
            switch (currentFirebaseState)
            {
            case FIREBASE_DISCONNECTED:
                ledStates[FIREBASE_LED] = RED;
                firebaseConnectingStart = 0; // Reset timer
                break;
            case FIREBASE_CONNECTING:
                ledStates[FIREBASE_LED] = BLUE;
                firebaseConnectingStart = millis(); // ✅ Start timer
                break;
            case FIREBASE_CONNECTED:
                // ✅ Only turn green if enough time has passed
                if (firebaseConnectingStart == 0 || (millis() - firebaseConnectingStart) >= MIN_CONNECTING_TIME)
                {
                    ledStates[FIREBASE_LED] = GREEN;
                    firebaseConnectingStart = 0; // Reset timer
                }
                else
                {
                    // Keep blue until minimum time passes
                    ledStates[FIREBASE_LED] = BLUE;
                }
                break;
            }
            lastFirebaseState = currentFirebaseState;
        }

        // ✅ Check Firebase timer expiration
        if (currentFirebaseState == FIREBASE_CONNECTED && firebaseConnectingStart > 0 &&
            (millis() - firebaseConnectingStart) >= MIN_CONNECTING_TIME &&
            ledStates[FIREBASE_LED] == BLUE)
        {
            ledStates[FIREBASE_LED] = GREEN;
            firebaseConnectingStart = 0; // Reset timer
        }

        // Update Heater LED based on Heater state

        static HeaterState lastHeaterState = HEATER_STATE_OFF;
        HeaterState currentHeaterState = HeaterControl_getState(); // ✅ Use proper function
        if (currentHeaterState != lastHeaterState)
        {
        switch (currentHeaterState)
        {
        case HEATER_STATE_ON:
            ledStates[HEATER_LED] = RED;
            break;
        case HEATER_STATE_OFF:
            ledStates[HEATER_LED] = GREEN;
            break;
        }
        lastHeaterState = currentHeaterState;
    }

        for (int i = 0; i < NUM_LEDS; i++)
        {
            updateLED(i, ledStates[i]);
        }
        // remove after testing
        static int debugCounter = 0;
if (++debugCounter % 20 == 0) {  // Print every 10 seconds (20 * 500ms)
    Serial.printf("🎨 LED Status: [%d]=%s [%d]=%s [%d]=%s [%d]=%s\n",
        WIFI_LED, (ledStates[WIFI_LED] == GREEN ? "GREEN" : ledStates[WIFI_LED] == RED ? "RED" : ledStates[WIFI_LED] == BLUE ? "BLUE" : "OFF"),
        MQTT_LED, (ledStates[MQTT_LED] == GREEN ? "GREEN" : ledStates[MQTT_LED] == RED ? "RED" : ledStates[MQTT_LED] == BLUE ? "BLUE" : "OFF"),
        FIREBASE_LED, (ledStates[FIREBASE_LED] == GREEN ? "GREEN" : ledStates[FIREBASE_LED] == RED ? "RED" : ledStates[FIREBASE_LED] == BLUE ? "BLUE" : "OFF"),
        HEATER_LED, (ledStates[HEATER_LED] == GREEN ? "GREEN" : ledStates[HEATER_LED] == RED ? "RED" : ledStates[HEATER_LED] == BLUE ? "BLUE" : "OFF"));
}
// end remove after testing

        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}