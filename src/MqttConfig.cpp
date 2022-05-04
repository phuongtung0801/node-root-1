#include "MqttConfig.h"
#include "WebConfig.h"
#include "MeshConfig.h"

MqttConfig TungMQTT;

const char *mqtt_server = "phuongtung08081.tk";
const char *mqtt_username = "tungtran256";
const char *mqtt_password = "nguyenthiminhthy2304";
const char *clientID = "TUNG";
const char *endTopic = "phuongtung0801/LWT";
long int port = 8883;

bool mqttCheck = false;

// ca certificate dung de xac thuc emqx broker
char *ca_pem = \  
"-----BEGIN CERTIFICATE-----\n"
               "MIIDvTCCAqWgAwIBAgIUfquovTolUXqgafOfX1yfZG2Bs7YwDQYJKoZIhvcNAQEL\n"
               "BQAwbjELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n"
               "GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDEnMCUGCSqGSIb3DQEJARYYcGh1b25n\n"
               "dHVuZzA4MDFAZ21haWwuY29tMB4XDTIxMTAwMjEyMjgxMVoXDTMxMDkzMDEyMjgx\n"
               "MVowbjELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n"
               "GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDEnMCUGCSqGSIb3DQEJARYYcGh1b25n\n"
               "dHVuZzA4MDFAZ21haWwuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n"
               "AQEAlGvKQP7AVR0X7XqDcFkWj+HFa0cwqeqMHcKLwCRKs5q8mWQK4/cZPft0/h94\n"
               "16Nv6wL8EsXOFA2xTTKo4B/CuoRUZp6ZIXjrh4fWXvZreRB5NKpMidD2SCT+vsm5\n"
               "2/zZmcjDWbuoSuHJgJdfd93baGvQ4qBpUoLe9r0ithHdqwlQ1KMGyeUnq8qFJ5Mv\n"
               "PZDIh5vQ0Pjny9Hq6XnC1nRD+Dd+6QqcYbpvjNGwteikI26GnPgfzcsaJaMHv6K6\n"
               "XpRuD0pqS31qxnSyLunFs2JTde7Njm97t33OFKgdzkHp0uTIpEtkYfyNlsTPYM1S\n"
               "E9dZkEjM+OX6O7YdilOXawoBkwIDAQABo1MwUTAdBgNVHQ4EFgQULiZzbQ3/Lbpn\n"
               "QeYzeY6aPrGBeP8wHwYDVR0jBBgwFoAULiZzbQ3/LbpnQeYzeY6aPrGBeP8wDwYD\n"
               "VR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEATetVwl3T22yRbSEZZxjK\n"
               "dgOfpKCI8BL/JcgFhGY0Kt1nh+/1zmmcM++Tec2E/zQWO0HbGJcUuzErWtX9J6r6\n"
               "rLGf22AwRj9BSeYlaMhJV70Y5faezBI9RnyvHG9vU3CCbWiECXr+aJ0+YQB8FSLd\n"
               "0+XXsGGUf+PF+SkVurhGZHtJk/uCXburz4+iVZGnP7i1QTS9j/jAICZ+//Bpe4nw\n"
               "5tzKa9z8rwWaNg9Prp0kGD0b1z1qoOKYjdV2wCgQBGoAf6sWXvZDFd76Ugk5N5lK\n"
               "Ikf7+L1wztzLHPkwIMHmpPtBNkWJsgPJGIFjQ1qhPqzCc8Zaubzdrn0nX6qw3/j2\n"
               "gA==\n"
               "-----END CERTIFICATE-----\n";
// fingerprint tạo bằng OpenSSL, dùng cho kết nối esp8266, esp32 dùng ca_pem
const char *fingerprint = "AA:75:54:46:08:29:36:05:B8:D1:F5:87:21:AD:E0:D1:42:3A:BE:69";

void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
    String cmd = "";
    Serial.print("Message arrived [");
    Serial.print(topic);

    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        cmd += (char)payload[i];
    }
    DynamicJsonDocument jsonBuffer(1024);
    JsonObject msg = jsonBuffer.to<JsonObject>();
    msg["topic"] = topic;
    msg["msg"] = cmd;
    String str;
    serializeJson(msg, str);
    ws.printfAll(str.c_str());
    //   Serial.println(cmd);
    // living room
    if (cmd == "living room lamp on")
    {
        Serial.println("living room lamp on");
        mesh.sendSingle(99191153, "living/lamp/on");
    }
    if (cmd == "living room lamp off")
    {

        Serial.println("living room lamp off");
        mesh.sendSingle(99191153, "living/lamp/off");
    }
    //  kitchen
    if (cmd == "kitchen lamp on")
    {
        Serial.println("kitchen lamp on");
        mesh.sendSingle(99191153, "kitchen/lamp/on");
    }
    if (cmd == "kitchen lamp off")
    {
        Serial.println("kitchen lamp of");
        mesh.sendSingle(99191153, "kitchen/lamp/off");
    }
    // bedroom one
    if (cmd == "bed room one lamp on")
    {
        Serial.println("bed room one lamp on");
        mesh.sendSingle(99191153, "bed1/lamp/on");
    }
    if (cmd == "bed room one lamp off")
    {
        Serial.println("bed room one lamp off");
        mesh.sendSingle(99191153, "bed1/lamp/off");
    }
    // bedroom two
    if (cmd == "bed room two lamp on")
    {
        Serial.println("bed room two lamp on");
        mesh.sendSingle(99191153, "bed2/lamp/on");
    }
    if (cmd == "bed room two lamp off")
    {
        Serial.println("bed room two lamp off");
        mesh.sendSingle(99191153, "bed2/lamp/off");
    }
}
// khai bao object mqtt
WiFiClientSecure wifiClientSecure;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClientSecure);

void MqttConfig::init()
{

    wifiClientSecure.setCACert(ca_pem);
    //    khoi tao mqtt voi thong so ket noi mac dinh
    mqttClient.setServer(mqtt_server, port);
    mqttClient.setCallback(mqttCallback);
    if (mqttClient.connect(
            clientID, mqtt_username, mqtt_password, endTopic, 1, true, "Sensor disconnected from mqtt"))
    {
        Serial.println("Connected to MQTT Broker!");
        mqttClient.publish(endTopic, "Sensor connected!", true);
        mqttCheck = true;
    }
    else
    {
        Serial.println("Connection to MQTT Broker failed...");
    }
    // mac dinh subscribe vao cac topic nay
    mqttClient.subscribe("device/esp32root/control");
    mqttClient.subscribe(endTopic);
    mqttClient.publish("device/esp32root/status", "testing tungtran!", true);
}

void MqttConfig::loop()
{
    mqttClient.loop();
}