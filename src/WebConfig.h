#ifndef WEB_CONFIG_H
#define WEB_CONFIG_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h> //thu vien connect wifi
#include <WiFiClient.h>
#include "SPIFFS.h"

class WebConfig
{
public:
    void init();
    void begin();
};
extern WebConfig TungWeb;
extern AsyncWebSocket ws;
#endif