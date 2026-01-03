// MQTTService.h
#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "TemperatureService.h"
#include "config.h"

// MQTT Broker Settings
#define MQTT_SERVER "ea53fbd1c1a54682b81526905851077b.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
#define MQTT_USER "ESP32FireBaseTortoise"
#define MQTT_PASSWORD "ESP32FireBaseHea1951Ter"

extern MQTTState mqttState;

// Task Handle
extern TaskHandle_t mqttTaskHandle;

// Mutex for thread safety
extern SemaphoreHandle_t mqttMutex;

// Functions
void MQTTService_init();
void MQTTService_task(void *pvParameters);
bool connectToMqtt(PubSubClient &client);
MQTTState MQTTService_getState();