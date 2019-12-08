#include <Arduino.h>
#include "../../WAdapter/Wadapter/WNetwork.h"
#include "WNeoDevice.h"


#define APPLICATION "Neo Coolcam"
#define VERSION "1.07"
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
	device = new WNeoDevice(network);
	network->addDevice(device);
}

void loop() {
	network->loop(millis());
	delay(50);
}

