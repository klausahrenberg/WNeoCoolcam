#include <Arduino.h>
#include "WNetwork.h"
#include "WNeoDevice.h"


#define APPLICATION "Neo Coolcam"
#define VERSION "1.20"
#define FLAG_SETTINGS 0x17
#define DEBUG false

WNetwork* network;
WDevice* device;

void setup() {
	if (DEBUG) {
		Serial.begin(9600);
	}
	//Network
	network = new WNetwork(DEBUG, APPLICATION, VERSION, NO_LED, FLAG_SETTINGS);
	//Device
	device = new WNeoDevice(network);
	network->addDevice(device);
}

void loop() {
	network->loop(millis());
	delay(50);
}
