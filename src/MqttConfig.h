#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include <PubSubClient.h>
#include <painlessMesh.h>
#include <WiFiClientSecure.h>

class MqttConfig
{
    // PubSubClient mqttClient;

public:
    void init();
    void loop();
};
extern MqttConfig TungMQTT;
extern WiFiClientSecure wifiClientSecure;
extern WiFiClient wifiClient;
extern PubSubClient mqttClient;

#endif