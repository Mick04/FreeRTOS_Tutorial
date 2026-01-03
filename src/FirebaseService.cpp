
// FirebaseService.cpp - REST API version
#include "FirebaseService.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "TemperatureService.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "TimeService.h"

// -------------------- Globals --------------------
FirebaseState firebaseState = FIREBASE_DISCONNECTED;
SemaphoreHandle_t firebaseMutex = NULL;

// Auth token (will be obtained once)
String idToken = "";
unsigned long tokenExpiry = 0;

// -------------------- Forward Declarations --------------------
bool getAuthToken();
bool writeToFirebase(const char *path, float value);

// -------------------- Initialization --------------------
void FirebaseService_init()
{
    Serial.println("\n===============================");
    Serial.println("🔥 Firebase REST API Service 🔥");
    Serial.println("===============================\n");

    firebaseMutex = xSemaphoreCreateMutex();
    if (!firebaseMutex)
    {
        Serial.println("❌ Failed to create Firebase mutex!");
        return;
    }

    xTaskCreatePinnedToCore(
        FirebaseService_task,
        "Firebase Task",
        8192,
        NULL,
        1,
        NULL,
        1);

    Serial.println("✅ Firebase task created");
}

// -------------------- Task Function --------------------
void FirebaseService_task(void *pvParameters)
{
    TemperatureData temps;
    unsigned long lastWrite = 0;
    unsigned long lastAuthAttempt = 0;
    bool authenticated = false;

    Serial.println("🔥 Firebase Task started");

    for (;;)
    {
        // Check WiFi connection
        if (WiFi.status() != WL_CONNECTED)
        {
            if (xSemaphoreTake(firebaseMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                firebaseState = FIREBASE_DISCONNECTED;
                xSemaphoreGive(firebaseMutex);
            }
            authenticated = false;
            idToken = "";
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        // Authenticate every 30s if needed
        if (!authenticated && (millis() - lastAuthAttempt > 30000))
        {
            if (xSemaphoreTake(firebaseMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                firebaseState = FIREBASE_CONNECTING;
                xSemaphoreGive(firebaseMutex);
            }

            lastAuthAttempt = millis();
            authenticated = getAuthToken();

            if (authenticated)
            {
                if (xSemaphoreTake(firebaseMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    firebaseState = FIREBASE_CONNECTED;
                    xSemaphoreGive(firebaseMutex);
                }
                Serial.println("✅ Firebase authenticated and ready!");
            }
            else
            {
                Serial.println("❌ Firebase authentication failed, will retry");
                vTaskDelay(pdMS_TO_TICKS(5000));
                continue;
            }
        }

        // Write data every 10 seconds
        if (authenticated && (millis() - lastWrite >= 10000))
        {
            if (TemperatureService::getTemperatures(temps))
            {
                Serial.println("\n📤 Writing to Firebase...");
                bool success = true;
                uint32_t epoch = TimeService_getEpoch();
                String isoTime = TimeService_getIsoTimestamp();
                success &= writeToFirebase("/tortoise/presenttemperature/heater", temps.heater);
                success &= writeToFirebase("/tortoise/presenttemperature/coolside", temps.coolside);
                success &= writeToFirebase("/tortoise/presenttemperature/outside", temps.outside);
                success &= writeToFirebase("/tortoise/presenttemperature/epoch", (float)epoch);

                String formattedTime = TimeService_getFormattedDateTime();

                // Write formatted timestamp
                HTTPClient http;
                String url = String(DATABASE_URL) +
                             "/tortoise/presenttemperature/timestamp.json?auth=" + idToken;
                http.begin(url);
                http.addHeader("Content-Type", "application/json");
                http.PUT("\"" + formattedTime + "\"");
                http.end();

                if (success)
                {
                    Serial.printf("✅ Firebase: H=%.2f, C=%.2f, O=%.2f\n",
                                  temps.heater, temps.coolside, temps.outside);
                    lastWrite = millis();
                }
                else
                {
                    Serial.println("❌ Some writes failed, token may be expired");
                    authenticated = false;
                    idToken = "";
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// -------------------- Auth Function --------------------
bool getAuthToken()
{
    if (idToken.length() > 0 && millis() < tokenExpiry)
    {
        return true;
    }

    Serial.println("🔑 Getting new Firebase auth token...");

    HTTPClient http;
    http.begin("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + String(API_KEY));
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"email\":\"" + String(USER_EMAIL) +
                     "\",\"password\":\"" + String(USER_PASSWORD) +
                     "\",\"returnSecureToken\":true}";

    int httpCode = http.POST(payload);

    if (httpCode == 200)
    {
        String response = http.getString();

        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error)
        {
            idToken = doc["idToken"].as<String>();
            int expiresIn = doc["expiresIn"].as<int>();
            tokenExpiry = millis() + (expiresIn * 1000) - 60000;

            Serial.println("✅ Auth token obtained!");
            Serial.printf("   Expires in %d seconds\n", expiresIn);
            http.end();
            return true;
        }
        else
        {
            Serial.println("❌ Failed to parse auth response");
        }
    }
    else
    {
        Serial.printf("❌ Auth failed with code: %d\n", httpCode);
        if (httpCode > 0)
        {
            Serial.println(http.getString());
        }
    }

    http.end();
    return false;
}

// -------------------- REST Write Function --------------------
bool writeToFirebase(const char *path, float value)
{
    if (idToken.length() == 0)
    {
        Serial.println("❌ No auth token");
        return false;
    }

    HTTPClient http;
    String url = String(DATABASE_URL) + path + ".json?auth=" + idToken;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String payload = String(value, 2);
    int httpCode = http.PUT(payload);

    bool success = (httpCode == 200);
    if (!success)
    {
        Serial.printf("❌ Write failed: %s = %.2f (HTTP %d)\n", path, value, httpCode);
    }

    http.end();
    return success;
}

// -------------------- State Getter --------------------
FirebaseState FirebaseService_getState()
{
    FirebaseState state;
    if (xSemaphoreTake(firebaseMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        state = firebaseState;
        xSemaphoreGive(firebaseMutex);
    }
    else
    {
        state = FIREBASE_DISCONNECTED;
    }
    return state;
}
