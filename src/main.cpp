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

// #include "DHT.h"
#include <Wire.h>
#include "SHT31.h"

float temperature = 0.0;
float humidity = 0.0;
SHT31 sht31 = SHT31();
AsyncWebServer server(80);

int RELAYS_PIN = 14;

void switchHeating(uint8_t on)
{
  digitalWrite(RELAYS_PIN, on);
}

void handleGetTemperature(AsyncWebServerRequest *request)
{
  Serial.println("Handling get temperature");
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  serializeJson(doc, *response);
  request->send(response);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();

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
  server.on("/temperature", HTTP_GET, handleGetTemperature);

  server.begin();

  pinMode(PIN_GROVE_POWER, OUTPUT);
  digitalWrite(PIN_GROVE_POWER, 1);
  pinMode(RELAYS_PIN, OUTPUT);
  switchHeating(1);
  sht31.begin();
}

void loop()
{
  temperature = sht31.getTemperature();
  humidity = sht31.getHumidity();
  Serial.printf("Temp = %f C, Humidity = %f", temperature, humidity);
  Serial.println(" %"); //The unit for  Celsius because original arduino don't support speical symbols
  delay(1000);

  // digitalWrite(RELAYS_PIN, 0);
  digitalWrite(LED_BUILTIN, HIGH);
  // wait for a second
  delay(1000);
  // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN, LOW);
  // wait for a second
  // digitalWrite(RELAYS_PIN, 1);
  delay(1000);
}
