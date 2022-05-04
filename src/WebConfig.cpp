#include "WebConfig.h"
#include "MqttConfig.h"
#include <mbedtls/md.h>

// khai bao wifi, luu y chỉ kết nối 2.4GHz
const char *ssid = "TUNG";
const char *password = "123456789";
// khai bao webserver authenticate
const char *www_username = "admin";
const char *www_password = "public";
int led_state = 0;
// config static IP
IPAddress local_IP(192, 168, 137, 8);
IPAddress gateway(192, 168, 137, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WebConfig TungWeb;

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
bool is_authenticated(AsyncWebServerRequest *request);
bool handleFileMQTT(AsyncWebServerRequest *request, const char **myMQTTServer, const char **myMQTTUsername, const char **myMQTTPassword, const char **myClientID);
void handleLogin(AsyncWebServerRequest *request);
bool handleFileRead(AsyncWebServerRequest *request, String path);
String sha1(String payloadStr);

void WebConfig::init()
{
    // register websocket
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    // middleware webserver
    server.on("/global.css", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              request->send(SPIFFS, "/global.css", "text/css");
              Serial.println("CSS global sent!"); });
    // send unzip css and js file
    server.on("/build/bundle.css", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              request->send(SPIFFS, "/build/bundle.css", "text/css");
              Serial.println("CSS bundle sent!"); });
    server.on("/build/bundle.js", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              request->send(SPIFFS, "/build/bundle.js", "application/javascript");
              Serial.println("JS bundle sent!"); });
    // send bundle gzip files
    server.on("/build/bundle.js.gz", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/build/bundle.js.gz", "application/x-gzip");
              response->addHeader("Content-Encoding", "gzip");
              request->send(response);
              Serial.println("JS bundle gzip sent!"); });
    server.on("/build/bundle.css.gz", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/build/bundle.css.gz", "application/x-gzip");
              response->addHeader("Content-Encoding", "gzip");
              request->send(response);
              Serial.println("JS bundle gzip sent!"); });

    server.on("/login", HTTP_POST, handleLogin);
    // first on "/", then browser click "Start Now" to send request to "/auth"
    server.on("/auth", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    Serial.println("Authenticating...");
    if (!handleFileRead(request, request->url()))
    {
        Serial.println("NOT Authenticated!");
    }
    else
      Serial.println("Authenticated!"); });

    // send root index file
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });
    // handle publish topic from UI
    server.on("/publish", HTTP_POST, [](AsyncWebServerRequest *request)
              {
              Serial.println("Receiving mqtt publish infor...");
              const char *newTopicPublish = request->arg("topic publish to").c_str();
              const char *newPublishMessage = request->arg("publish message").c_str();

              //response cho client neu ket noi thanh cong
              if (mqttClient.publish(newTopicPublish, newPublishMessage))
              {
                String json = "{\"status\":\"ok\"}";
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
                //send response cho client
                request->send(response);
              } });

    // handle subscribe topic from UI
    server.on("/subscribe", HTTP_POST, [](AsyncWebServerRequest *request)
              {
              Serial.println("Receiving mqtt subscribe infor...");
              const char *newTopicSubscribe = request->arg("topic subscribed").c_str();

              //response cho client neu ket noi thanh cong
              if (mqttClient.subscribe(newTopicSubscribe))
              {
                String json = "{\"status\":\"ok\"}";
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
                //send response cho client
                request->send(response);
              } });
}

void WebConfig::begin()
{
    // Connect to Wi-Fi
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
        Serial.println("STA failed to config");
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println(WiFi.localIP());

    server.begin();
}

// Check if header cookie is present and correct
bool is_authenticated(AsyncWebServerRequest *request)
{
    Serial.println("Enter is_authenticated");
    if (request->hasHeader("Cookie"))
    {
        Serial.print("Found cookie authenticated: ");
        String cookie = request->header("Cookie");
        Serial.println(cookie);
        String token = sha1(String(www_username) + ":" +
                            String(www_password) + ":" +
                            request->client()->remoteIP().toString());
        //  token = sha1(token);
        if (cookie.indexOf("ESPSESSIONID=" + token) != -1)
        {
            Serial.println(request->header("Cookie"));
            Serial.println("indexof cookie nek:");
            Serial.println(cookie.indexOf("ESPSESSIONID=" + token));
            Serial.println("Authentication Successful");
            return true;
        }
    }
    Serial.println("Authentication Failed");
    return false;
}

// hash function
String sha1(String payloadStr)
{
    const char *payload = payloadStr.c_str();
    int size = 20;
    byte shaResult[size];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;
    const size_t payloadLength = strlen(payload);
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *)payload, payloadLength);
    mbedtls_md_finish(&ctx, shaResult);
    mbedtls_md_free(&ctx);
    String hashStr = "";
    for (uint16_t i = 0; i < size; i++)
    {
        String hex = String(shaResult[i], HEX);
        if (hex.length() < 2)
        {
            hex = "0" + hex;
        }
        hashStr += hex;
    }
    return hashStr;
}

// handle mqtt request tu client
bool handleFileMQTT(AsyncWebServerRequest *request, const char **myMQTTServer, const char **myMQTTUsername, const char **myMQTTPassword, const char **myClientID)
{
    Serial.println("Handle MQTT request...");
    String body = request->arg("mqttUsername");
    Serial.println(body);
    String msg;
    int args = request->args();
    for (int i = 0; i < args; i++)
    {
        Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
    }
    // get specific header by name
    if (request->hasHeader("Cookie"))
    {
        // Print cookies
        Serial.print("Found cookie handlelogin: ");
        // list header cookie ra
        String cookie = request->header("Cookie");
        Serial.println(cookie);
    }
    // Check if parameter exists (Compatibility)
    if (request->hasArg("mqttUsername") && request->hasArg("mqttPassword"))
    {
        // luu thong tin ma user da gui vao cac bien tuong ung
        Serial.print("Found parameter: ");
        *myMQTTServer = request->arg("mqttServer").c_str();
        *myMQTTUsername = request->arg("mqttUsername").c_str();
        *myMQTTPassword = request->arg("mqttPassword").c_str();
        *myClientID = request->arg("mqttClientID").c_str();
        String portInt = request->arg("mqttPort");
        Serial.println("Handle mqtt login Successful");
        return true;
    }
    else
        return false;
}

// check cookie co giong voi usrname va psswd hay khong
void handleLogin(AsyncWebServerRequest *request)
{
    Serial.println("Handle login");
    String body = request->arg("Username");
    Serial.println(body);
    String msg;
    int args = request->args();
    for (int i = 0; i < args; i++)
    {
        Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
    }
    // get specific header by name
    if (request->hasHeader("Cookie"))
    {
        // Print cookies
        Serial.print("Found cookie handlelogin: ");
        // list header cookie ra
        String cookie = request->header("Cookie");
        Serial.println(cookie);
    }
    // Check if parameter exists (Compatibility)
    if (request->hasArg("Username") && request->hasArg("Password"))
    {
        Serial.print("Found parameter: ");
        if (request->arg("Username") == String(www_username) && request->arg("Password") == String(www_password))
        {
            String json = "{\"status\":\"ok\"}";
            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
            // hash password thanh token = hashed(username + : + password + : + IP cua client)
            String token = sha1(String(www_username) + ":" + String(www_password) + ":" + request->client()->remoteIP().toString());
            Serial.print("Token: ");
            Serial.println(token);
            // them cookie vao response header: set-cookie: ESPSESSIONID= token
            response->addHeader("Set-Cookie", "ESPSESSIONID=" + token);
            // send response cho client
            request->send(response);
            Serial.println("Log in Successful");
            return;
        }
        else
        {
            msg = "Wrong username/password! try again.";
            Serial.println("Log in Failed");
            // AsyncWebServerResponse *response = request->beginResponse(301); //Sends 301 redirect
            // response->addHeader("Location", "/login.html?msg=" + msg);
            // response->addHeader("Cache-Control", "no-cache");
            // request->send(response);
            String jsonNotOk = "{\"status\":\"khong ok\"}";
            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonNotOk);
            request->send(response);
            return;
        }
    }
}

// ham doc file SPIFFS de gui cho client.
bool handleFileRead(AsyncWebServerRequest *request, String path)
{
    String jsonNotOk = "{\"status\":\"khong ok\"}";
    String jsonOk = "{\"status\":\"ok\"}";
    Serial.print(F("handleFileRead: "));
    Serial.println(path);
    path = "/index.html";
    // send json "ok" if has cookie, send "khong ok" if hasn't cookie
    if (!is_authenticated(request))
    {
        Serial.println(F("Go on not auth!"));
        request->send(200, "application/json", jsonNotOk);
        return false;
    }
    else
    {
        Serial.println(F("Go on authenticated!"));
        request->send(200, "application/json", jsonOk);
        return true;
    }
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {

        Serial.println("Websocket client connection received...");
    }
    else if (type == WS_EVT_DISCONNECT)
    {

        Serial.println("Client disconnected");
    }
    else if (type == WS_EVT_DATA)
    {

        Serial.println("Client send data");
        String cmd = "";
        for (int i = 0; i < len; i++)
        {
            cmd += (char)data[i];
        }
        Serial.println(cmd);

        Serial.print("Message is: ");
        Serial.println(cmd);
        if (strcmp(cmd.c_str(), "getLEDState") == 0)
        {
            Serial.printf("Sending to client: %d\n", led_state);
            DynamicJsonDocument jsonBuffer(1024);
            JsonObject msg = jsonBuffer.to<JsonObject>();
            msg["topic"] = "led_state";
            if (led_state)
            {
                msg["msg"] = "one";
                String str;
                serializeJson(msg, str);
                client->text(str.c_str());
            }
            else
            {
                msg["msg"] = "zero";
                String str;
                serializeJson(msg, str);
                client->text(str.c_str());
            }
        }
        else if (strcmp(cmd.c_str(), "toggleLED") == 0)
        {
            led_state = led_state ? 0 : 1;
            Serial.printf("Toggling LED to: %d\n", led_state);
            digitalWrite(2, led_state);
            String cmd = "";
            for (int i = 0; i < len; i++)
            {
                cmd += (char)data[i];
            }

            Serial.print("Client message receive: ");
            Serial.println(cmd);
        }

        else
        {
            Serial.println((char *)data);
            String cmd = "";
            for (int i = 0; i < len; i++)
            {
                cmd += (char)data[i];
            }
            Serial.print("No connect and Client message receive: ");
            Serial.println(cmd);
        }
    }
}
