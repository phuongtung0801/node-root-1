#include <Arduino.h>
#include "WebConfig.h"
#include "MqttConfig.h"
#include "MeshConfig.h"

void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

void setup()
{
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
  listDir(SPIFFS, "/", 0);

  /*Mesh truoc, mqtt sau moi chay duoc*/
  TungMesh.init();
  TungWeb.init();
  TungWeb.begin();
  TungMQTT.init();
}

void loop()
{
  TungMQTT.loop();
  TungMesh.loop();
}

// list file in SPIFFS
void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("− failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" − not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}