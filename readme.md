# Building a temperature control for sour dough (Gärbox)

## Componentns

- SHT31
- Wio Link from seed with ESP8266 (any ESP8266 board will do)
- relay that can be controlled through GPIO
- heating mat, e.g. for cats or for reptiles or ... Mine was with 12V power supply, so the controller had to switch 1A 12V power to the heating

## Setup/configuration

- create wifisecrets.h file and define WIFI_SSID and WIFI_PASSWORD (look at main.cpp)
- default temperature is 26, if you set it via the webserver route, it will be stored in EEPROM of the ESP8266

## WebServer Routes

- `/temperature`: retrieve json with temperature, humidity, heating status and target temperature, if you pass in `?target_temperature=xx` as parameter, the target temperature will be set

### Test methods

- `/relay/on`: will switch on the relay (until temperature management switches it off again)
- `/relay/off`: will switch off the relay (until temperature management switches it on again)

# Implementation notes

- webserver is running async
- did produce crashes when reading sensor data in async handlers
- all sensor reads, pin writes are now triggered through the loop()
- only changes relay every x clock ticks through temperature mesaurements, to avoid flapping of relay
- last target temperature is stored in EEPROM

## SHT31

- Grove connector for I2C:
  - pin 1 - Yellow (for example, SCL on I2C Grove Connectors)
  - pin 2 - White (for example, SDA on I2C Grove Connectors)
  - pin 3 - Red - VCC on all Grove Connectors
  - pin 4 - Black - GND on all Grove Connectors

## Relays on GPIO 14

- 1 is on
- 0 is off
