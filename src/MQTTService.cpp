// MQTTService.cpp

#include "MQTTService.h"

// Global state
MQTTState mqttState = MQTT_STATE_DISCONNECTED;
TaskHandle_t mqttTaskHandle = NULL;

// Mutex for thread safety
SemaphoreHandle_t mqttMutex = NULL;

// Connect to MQTT broker
bool connectToMqtt(PubSubClient &client)
{
    // Check WiFi first
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    int attempts = 0;
    const int MAX_ATTEMPTS = 3;

    while (!client.connected() && attempts < MAX_ATTEMPTS)
    {
        Serial.println("Connecting to MQTT...");

        // Create unique client ID
        String clientId = "ESP32_Tortoise_" + String(random(0xffff), HEX);

        // Yield to watchdog
        vTaskDelay(pdMS_TO_TICKS(100));

        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD))
        {
            Serial.println("MQTT Connected!");
            return true;
        }
        else
        {
            attempts++;
            int rc = client.state();
            Serial.print("MQTT connection failed, rc=");
            Serial.print(rc);
            Serial.print(" (");

            // Print human-readable error
            switch (rc)
            {
            case -4:
                Serial.print("connection timeout");
                break;
            case -3:
                Serial.print("connection lost");
                break;
            case -2:
                Serial.print("connect failed");
                break;
            case -1:
                Serial.print("disconnected");
                break;
            case 0:
                Serial.print("connected");
                break;
            case 1:
                Serial.print("bad protocol version");
                break;
            case 2:
                Serial.print("bad client id");
                break;
            case 3:
                Serial.print("unavailable");
                break;
            case 4:
                Serial.print("bad credentials");
                break;
            case 5:
                Serial.print("unauthorized");
                break;
            default:
                Serial.print("unknown");
                break;
            }

            Serial.print(") attempt ");
            Serial.print(attempts);
            Serial.print("/");
            Serial.println(MAX_ATTEMPTS);

            // Clean up failed connection
            client.disconnect();

            // Shorter delay between attempts
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    Serial.println("MQTT connection failed after max attempts");
    return false;
}

// Initialize MQTT Service
void MQTTService_init()
{
    // Create mutex for thread safety
    mqttMutex = xSemaphoreCreateMutex();
    
        // Add this check:
    if (mqttMutex == NULL)
    {
        Serial.println("Failed to create MQTT mutex!");
        return;
    }

    // Create the FreeRTOS task
    xTaskCreate(
        MQTTService_task,
        "MQTT Task",
        16384, // Increased stack size for SSL/TLS operations
        NULL,
        1,
        &mqttTaskHandle);
}

MQTTState MQTTService_getState()
{
    MQTTState state;
    if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        state = mqttState;
        xSemaphoreGive(mqttMutex);
    }
    else
    {
        state = MQTT_STATE_DISCONNECTED; // Safe default
    }
    return state;
}

// FreeRTOS Task
void MQTTService_task(void *pvParameters)
{
    // Create local WiFiClientSecure and PubSubClient instances for this task
    WiFiClientSecure localWifiClient;
    PubSubClient localMqttClient(localWifiClient);

    // Configure SSL/TLS (disable certificate validation for simplicity)
    localWifiClient.setInsecure();

    // Configure MQTT client
    localMqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    localMqttClient.setBufferSize(512);  // Set buffer size
    localMqttClient.setSocketTimeout(5); // 5 second socket timeout
    localMqttClient.setKeepAlive(60);    // 60 second keepalive

    TemperatureData temps;
    TemperatureData previousTemps = {-999.0, -999.0, -999.0}; // Initialize to invalid values
    unsigned long lastPublish = 0;
    unsigned long lastConnectionAttempt = 0;
    const unsigned long MIN_PUBLISH_INTERVAL = 10000;      // Minimum 10 seconds between publishes
    const unsigned long MAX_PUBLISH_INTERVAL = 60000;      // Force publish every 60 seconds
    const float TEMP_CHANGE_THRESHOLD = 0.5;               // 0.5°C minimum change to trigger publish
    const unsigned long CONNECTION_RETRY_INTERVAL = 15000; // Increased retry interval

    Serial.println("MQTT Task started");

    for (;;)
    {
        unsigned long currentTime = millis();

        // Yield to watchdog and other tasks frequently
        vTaskDelay(pdMS_TO_TICKS(100));

        // Check WiFi connection status (no mutex needed for WiFi status)
        if (WiFi.status() != WL_CONNECTED)
        {
            // Take mutex only to update state
            if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                if (mqttState != MQTT_STATE_DISCONNECTED)
                {
                    Serial.println("WiFi lost, disconnecting MQTT");
                    mqttState = MQTT_STATE_DISCONNECTED;
                }
                xSemaphoreGive(mqttMutex);
            }
            localMqttClient.disconnect();
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        // Handle MQTT connection (network operations without mutex)
        if (!localMqttClient.connected())
        {
            // Only attempt connection if enough time has passed
            if (currentTime - lastConnectionAttempt >= CONNECTION_RETRY_INTERVAL)
            {
                // Update state before attempting connection
                if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    mqttState = MQTT_STATE_CONNECTING;
                    xSemaphoreGive(mqttMutex);
                }

                if (connectToMqtt(localMqttClient))
                {
                    lastConnectionAttempt = currentTime;
                    Serial.println("MQTT connection successful");

                    // Update state after successful connection
                    if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                    {
                        mqttState = MQTT_STATE_CONNECTED;
                        xSemaphoreGive(mqttMutex);
                    }
                }
                else
                {
                    lastConnectionAttempt = currentTime;
                    Serial.println("MQTT connection failed, will retry later");

                    // Update state after failed connection
                    if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                    {
                        mqttState = MQTT_STATE_DISCONNECTED;
                        xSemaphoreGive(mqttMutex);
                    }
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    continue;
                }
            }
            else
            {
                // Wait before retrying
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }
        }

        // Maintain MQTT connection (non-blocking, no mutex needed)
        if (localMqttClient.connected())
        {
            if (!localMqttClient.loop())
            {
                Serial.println("MQTT loop failed, connection lost");
                localMqttClient.disconnect();

                // Update state after connection loss
                if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    mqttState = MQTT_STATE_DISCONNECTED;
                    xSemaphoreGive(mqttMutex);
                }
                continue;
            }

            // Check if we should publish (either on change or max interval)
            if (currentTime - lastPublish >= MIN_PUBLISH_INTERVAL)
            {
                // Get temperatures safely with timeout
                if (TemperatureService::getTemperatures(temps))
                {
                    bool shouldPublish = false;

                    // Check if this is the first reading
                    if (previousTemps.heater == -999.0)
                    {
                        shouldPublish = true;
                        Serial.println("First temperature reading, publishing...");
                    }
                    else
                    {
                        // Check for significant changes
                        bool heaterChanged = abs(temps.heater - previousTemps.heater) >= TEMP_CHANGE_THRESHOLD;
                        bool coolsideChanged = abs(temps.coolside - previousTemps.coolside) >= TEMP_CHANGE_THRESHOLD;
                        bool outsideChanged = abs(temps.outside - previousTemps.outside) >= TEMP_CHANGE_THRESHOLD;

                        if (heaterChanged || coolsideChanged || outsideChanged)
                        {
                            shouldPublish = true;
                            Serial.println("Temperature change detected, publishing...");

#if defined(DEBUG) || defined(DEBUG_MQTT)
                            if (heaterChanged)
                            {
                                Serial.print("Heater: ");
                                Serial.print(previousTemps.heater);
                                Serial.print(" -> ");
                                Serial.println(temps.heater);
                            }
                            if (coolsideChanged)
                            {
                                Serial.print("CoolSide: ");
                                Serial.print(previousTemps.coolside);
                                Serial.print(" -> ");
                                Serial.println(temps.coolside);
                            }
                            if (outsideChanged)
                            {
                                Serial.print("Outside: ");
                                Serial.print(previousTemps.outside);
                                Serial.print(" -> ");
                                Serial.println(temps.outside);
                            }
#endif
                        }
                        // Force publish after max interval even if no change
                        else if (currentTime - lastPublish >= MAX_PUBLISH_INTERVAL)
                        {
                            shouldPublish = true;
                            Serial.println("Max interval reached, force publishing...");
                        }
                    }

                    if (shouldPublish)
                    {
#if defined(DEBUG) || defined(DEBUG_MQTT)
                        Serial.println("💥 Publishing Temperatures 💥");
                        Serial.print("Heater: ");
                        Serial.print(temps.heater);
                        Serial.print(", CoolSide: ");
                        Serial.print(temps.coolside);
                        Serial.print(", Outside: ");
                        Serial.println(temps.outside);
#endif

                        // Publish with error checking (no mutex needed for publishing)
                        bool publishSuccess = true;
                        publishSuccess &= localMqttClient.publish("tortoise/heater", String(temps.heater).c_str(), true);
                        vTaskDelay(pdMS_TO_TICKS(10)); // Small delay between publishes

                        publishSuccess &= localMqttClient.publish("tortoise/coolside", String(temps.coolside).c_str(), true);
                        vTaskDelay(pdMS_TO_TICKS(10));

                        publishSuccess &= localMqttClient.publish("tortoise/outside", String(temps.outside).c_str(), true);

                        if (!publishSuccess)
                        {
                            Serial.println("MQTT publish failed");
                            localMqttClient.disconnect();

                            // Update state after publish failure
                            if (xSemaphoreTake(mqttMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                            {
                                mqttState = MQTT_STATE_DISCONNECTED;
                                xSemaphoreGive(mqttMutex);
                            }
                        }
                        else
                        {
                            Serial.println("MQTT publish successful");
                            // Update previous temperatures after successful publish
                            previousTemps = temps;
                            lastPublish = currentTime;
                        }
                    }
                    else
                    {
#if defined(DEBUG) || defined(DEBUG_MQTT)
                        Serial.println("No significant temperature change, skipping publish");
#endif
                    }
                }
                else
                {
                    Serial.println("Failed to get temperature data");
                }
            }
        }

        // Small delay to prevent task from consuming too much CPU
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}