/*
  - add DHT library from Adafruit
  - Board is Wio Link from seeed
  - Specs here: https://wiki.seeedstudio.com/Wio_Link/
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <EEPROM.h>

// #include "DHT.h"
#include <Wire.h>
#include "SHT31.h"

uint8_t targetTemperature = 0;
float temperature = 0.0;
float humidity = 0.0;
uint8_t relayStatus = 0;
bool relayStatusChanged = true;

SHT31 sht31 = SHT31();
AsyncWebServer server(80);

int RELAYS_PIN = 14;

//
void updateHeating()
{
  if (relayStatusChanged)
  {
    Serial.print("Changing Relay status to ");
    Serial.print(relayStatus);
    Serial.println("");
    digitalWrite(RELAYS_PIN, relayStatus);
  }
  relayStatusChanged = false;
}

// first two bytes of EEPROM need to be target temperature, otherwise the default will be used
void setupTargetTemperature()
{
  uint8_t targetTemperature1, targetTemperature2;
  targetTemperature1 = EEPROM.read(0);
  targetTemperature2 = EEPROM.read(1);
  if (targetTemperature1 == targetTemperature2)
  {
    targetTemperature = targetTemperature1;
    Serial.print("Restoring target temperature from EEPROM (C) ");
    Serial.print(targetTemperature);
  }
  else
  {
    targetTemperature = 26;
    Serial.print("Using default target temperature 26C");
  }
  Serial.println();
}

void setTargetTemperature(uint8_t aTargetTemperature)
{
  targetTemperature = aTargetTemperature;
  EEPROM.write(0, targetTemperature);
  EEPROM.write(1, targetTemperature);
  EEPROM.commit();
}

void handleTemperature(AsyncWebServerRequest *request)
{
  Serial.println("Handling temperature");
  if (request->hasParam("target_temperature"))
  {
    AsyncWebParameter *p = request->getParam("target_temperature");
    setTargetTemperature(p->value().toInt());
  }

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  StaticJsonDocument<200> doc;
  doc["target_temperature"] = targetTemperature;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["heating"] = relayStatus;

  serializeJson(doc, *response);
  request->send(response);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();
  Serial.println("Booting...");

  EEPROM.begin(512);
  setupTargetTemperature();

  WiFi.mode(WIFI_STA);
  WiFi.begin("", "");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/relay/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    relayStatus = 1;
    relayStatusChanged = true;
    request->send(200, "application/json", "{ status: OK }");
  });

  server.on("/relay/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    relayStatus = 0;
    relayStatusChanged = true;
    request->send(200, "application/json", "{ status: OK }");
  });

  server.on("/temperature", HTTP_GET, handleTemperature);
  server.begin();

  pinMode(PIN_GROVE_POWER, OUTPUT);
  digitalWrite(PIN_GROVE_POWER, 1);
  pinMode(RELAYS_PIN, OUTPUT);
  updateHeating();
  sht31.begin();
}

int counter = 0;
void loop()
{
  if (counter % 100 == 0)
  {
    temperature = sht31.getTemperature();
    humidity = sht31.getHumidity();
    Serial.printf("Temp = %f C, Humidity = %f", temperature, humidity);
    Serial.println(" %");
    if (targetTemperature - temperature > 0.5 && relayStatus == 0)
    {
      relayStatus = 1;
      relayStatusChanged = 1;
    }
    if (targetTemperature - temperature <= 0.5 && relayStatus == 1)
    {
      relayStatus = 0;
      relayStatusChanged = 1;
    }
  }
  counter += 1;
  updateHeating();
  delay(50);
}
