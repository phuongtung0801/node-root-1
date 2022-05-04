#include <MeshConfig.h>
#include "MqttConfig.h"

MeshConfig TungMesh;
painlessMesh mesh;
// define mesh
#define MESH_PREFIX "tung"
#define MESH_PASSWORD "phuongtung0801"
#define MESH_PORT 5555
#define HOSTNAME "MQTT_Bridge"
void newConnectionCallback(uint32_t nodeId);

// ham nay nhan callback khi chay mesh network
void receivedCallback(const uint32_t &from, const String &msg)
{
    Serial.print("Mesh received now:");
    Serial.println(millis());
    Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
    // String topic = "painlessMesh/from/2131961461";
    String topic = "device/esp32root/status" + String(from);
    mqttClient.publish(topic.c_str(), msg.c_str());
    Serial.println("Data received");
}

void MeshConfig::init()
{
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages

    // Channel set to 6. Make sure to use the same channel for your mesh and for you other
    // network (STATION_SSID)
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP, 6);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    // mesh.setHostname(HOSTNAME);
    Serial.println("Mesh root node initialized");
}

void MeshConfig::loop()
{
    mesh.update();
    // mqttClient.publish("tungtran", "fuck yoidasjfhjass");
    //  Serial.println("mesh network updating...");
}

void newConnectionCallback(uint32_t nodeId)
{
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}