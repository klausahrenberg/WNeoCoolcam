#include <Arduino.h>
#include "../../WAdapter/Wadapter/WNetwork.h"
#include "WNeoDevice.h"


#define APPLICATION "Neo Coolcam"
#define VERSION "1.06"
#define DEBUG false

WNetwork* network;
WDevice* device;

void setup() {
	if (DEBUG) {
		Serial.begin(9600);
	}
	//Network
	network = new WNetwork(DEBUG, APPLICATION, VERSION, true, NO_LED);
	//Device
	device = new WNeoCoolcam(DEBUG, network->getIdx(), network->getSettings());
	network->addDevice(device);
}

void loop() {
	unsigned long now = millis();
	network->loop(now, false);
	delay(100);
}

void log(String debugMessage) {
	if (DEBUG) {
		Serial.println(debugMessage);
	}
}

void logError(String errorMessage) {
	log(errorMessage);
}
