/**
 * @file mqtt.h
 *
 */
#ifndef MQTT_H
#define MQTT_H

/*********************
 *      INCLUDES
 *********************/
#include <WiFi.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include <AsyncMqttClient.h>

/*********************
 *      DEFINES
 *********************/
#define WIFI_SSID "KELPNET_IOT"
#define WIFI_PASSWORD "GROENEPAARDENSCHOENEN"

#define MQTT_HOST IPAddress(192, 168, 1, 100)
#define MQTT_PORT 1883

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 * GLOBAL PROTOTYPES
 **********************/
void init_mqtt();
void connectToWifi();
void connectToMqtt();
void WiFiEvent(WiFiEvent_t event);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttPublish(uint16_t packetId);

#endif /* MQTT_H */