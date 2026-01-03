#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "TemperatureService.h"

// ========= Firebase config =========
#define API_KEY "AIzaSyDkJinA0K6NqBLGR4KYnX8AdDNgXp2-FDI"
#define DATABASE_URL "https://esp32-heater-controler-6d11f-default-rtdb.europe-west1.firebasedatabase.app"
#define USER_EMAIL "esp32@test.com"
#define USER_PASSWORD "test1234"


extern FirebaseState firebaseState;
extern SemaphoreHandle_t firebaseMutex;

// Init + task
void FirebaseService_init();
void FirebaseService_task(void *pvParameters);
FirebaseState FirebaseService_getState();
uint32_t TimeService_getEpoch();
String TimeService_getIsoTimestamp();