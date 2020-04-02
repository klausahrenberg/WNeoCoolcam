# WNeoCoolcam
Replaces original Tuya firmware on NeoCoolcam or Sonoff devices with ESP8266 wifi module. The firmware is tested with following devices:
* Neo Coolcam
* Sonoff Mini
* Sonoff Basic
* Sonoff 4-channel
## Features
* Enables switches to communicate via MQTT and/or Mozilla Webthings
* Configuration of connection and device parameters via web interface
* Reading and setting of all parameters via MQTT
* Reading and setting of main parameters via Webthing
* Switch and relay can be controlled separatly. For example, a Sonoff Mini can used use for a second switch to control a relay of another Sonoff Mini. Logic must be done via MQTT, but an unused relay can be taken out of function.
## Installation
To install the firmware, follow instructions here:  
https://github.com/klausahrenberg/WThermostatBeca/blob/master/Flashing.md
## Initial configuration
After installation, the device configuration is available at `http://<device_ip>/config`  
## Integration in Webthings
This firmware supports Mozilla Webthings directly. With Webthings you can control the device via the Gateway - inside and also outside of your home network. No clunky VPN, dynDNS solutions needed to access your home devices. I recommend to run the gateway in parallel to an MQTT server and for example Node-Red. Via MQTT you can control the device completely and logic can be done by Node-Red. Webthings is used for outside control of main parameters.  
## Json structure
Firmware provides different json messages:
1. State report  
2. Device information (at start of device to let you know the topics and ip)
### 1. State report 
**MQTT:** State report is provided every 5 minutes, at change of a parameter or at request via message with empty payload to `<your_mqtt_topic>/switch/<your_state_topic>`  
**Webthing:** State report can be requested by: `http://<device_ip>/things/switch/properties`  
```json
{
    "idx":"<your_idx>",
    "ip":"192.168.x.x",
    "firmware":"x.xx",
    "on":false
}
```
### 3. Device information
**MQTT:** At start of device to let you know the topics and ip to `devices/switch`  
**Webthing:** n.a.
```json
{
    "url":"http://192.168.x.x/things/switch",
    "ip":"192.168.x.x",
    "stateTopic":"<your_mqtt_topic>/switch/<your_state_topic>",
    "setTopic":"<your_mqtt_topic>/switch/<your_set_topic>"
}
```
## Modifying parameters via MQTT
Send a complete json structure with changed parameters to `<your_mqtt_topic>/thermostat/<your_set_topic>`, e.g. `neocoolcam/switch/set`. Alternatively you can set single values, modify the topic to `<your_mqtt_topic>/thermostat/<your_set_topic>/<parameter>`, e.g. `neocoolcam/switch/set/on`. The payload contains the new value. 

### Build this firmware from source
For build from sources you can use the Arduino-IDE, Atom IDE or other. All sources needed are inside the folder 'WNeoCoolcam' and my other library: https://github.com/klausahrenberg/WAdapter. Additionally you will need some other libraries: DNSServer, EEPROM (for esp8266), ESP8266HTTPClient, ESP8266mDNS, ESP8266WebServer, ESP8266WiFi, Hash, NTPClient, Time - It's all available via board and library manager inside of ArduinoIDE