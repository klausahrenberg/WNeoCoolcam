#ifndef W_NEO_COOLCAM_H
#define W_NEO_COOLCAM_H

#include "WDevice.h"
#include "WSwitch.h"
#include "WRelay.h"
#include "WProperty.h"
#include "W2812Led.h"

#define COUNT_DEVICE_TYPES 4
#define COUNT_DEVICE_MODES 3
#define COUNT_MAX_RELAIS 4
#define NO_PIN 255
#define MODE_BUTTON_LINKED_TO_RELAY 0
#define MODE_SEPARATE_RELAY_PROPERTY 1
#define MODE_NO_RELAY_USAGE 2

#define PIN_SDA 3 //RX // green
#define PIN_SCL 1 //TX // yellow
#define PIN_W2812 4

struct SwitchDevices{
	byte statusLed;
    byte relayPins[COUNT_MAX_RELAIS];
    byte switchPins[COUNT_MAX_RELAIS];
    byte switchModes[COUNT_MAX_RELAIS];
};

static struct SwitchDevices supportedDevices [4] =
{
//LED  RELAY		                 SWITCHES
 { 4, {12, NO_PIN, NO_PIN, NO_PIN}, {13, NO_PIN, NO_PIN, NO_PIN}, {MODE_BUTTON, NO_PIN, NO_PIN, NO_PIN}}, //Neo Coolcam
 {13, {12, NO_PIN, NO_PIN, NO_PIN}, { 0,      4, NO_PIN, NO_PIN}, {MODE_BUTTON, MODE_SWITCH, NO_PIN, NO_PIN}}, //Sonoff Mini
 {13, {12, NO_PIN, NO_PIN, NO_PIN}, { 0, NO_PIN, NO_PIN, NO_PIN}, {MODE_BUTTON, NO_PIN, NO_PIN, NO_PIN}}, //Sonoff Basic
 //{13, {12,      5,      4,     15}, { 0,      9,     10,     14}, {MODE_BUTTON, MODE_BUTTON, MODE_BUTTON, MODE_BUTTON}}, //Sonoff 4-channel
 { 2, { 5, NO_PIN, NO_PIN, NO_PIN}, { 0, NO_PIN, NO_PIN, NO_PIN}, {MODE_BUTTON, NO_PIN, NO_PIN, NO_PIN}} //Wemos: Relay at D1, Switch at D3
};

class WNeoDevice: public WDevice {
public:
	WNeoDevice(WNetwork* network)
	    	: WDevice(network, "switch", "switch", DEVICE_TYPE_ON_OFF_SWITCH) {

		this->deviceType = network->getSettings()->setByte("deviceType", 0);
		this->deviceMode = network->getSettings()->setByte("deviceMode", 0);
		this->supportingW2812 = network->getSettings()->setBoolean("supportingW2812", false);
		this->ledProgram = network->getSettings()->setByte("ledProgram", 0);
		this->ledProgram->addEnumByte(0);
		this->ledProgram->addEnumByte(1);
		this->ledProgram->addEnumByte(2);
		this->ledProgram->addEnumByte(3);
		mainLedRelay = nullptr;
		onOffProperty = nullptr;

		this->showAsWebthingDevice = network->getSettings()->setBoolean("showAsWebthingDevice", true);
    this->showAsWebthingDevice->setReadOnly(true);
		this->showAsWebthingDevice->setVisibility(NONE);
		this->addProperty(showAsWebthingDevice);
    this->setVisibility(this->showAsWebthingDevice->getBoolean() ? ALL : MQTT);
		//HtmlPages
    WPage* configPage = new WPage(this->getId(), "Configure switch");
    configPage->setPrintPage(std::bind(&WNeoDevice::printConfigPage, this, std::placeholders::_1, std::placeholders::_2));
    configPage->setSubmittedPage(std::bind(&WNeoDevice::saveConfigPage, this, std::placeholders::_1, std::placeholders::_2));
    network->addCustomPage(configPage);
		//StatusLed
		if (supportedDevices[getDeviceType()].statusLed != NO_PIN) {
			this->statusLed = new WLed(supportedDevices[getDeviceType()].statusLed);
		}
		for (int i = 0; i < COUNT_MAX_RELAIS; i++) {
			if (supportedDevices[getDeviceType()].relayPins[i] != NO_PIN) {
				//Property
				onOffProperty = WProperty::createOnOffProperty("on", "Switch");
				this->addProperty(onOffProperty);
				if (getDeviceMode() != MODE_NO_RELAY_USAGE) {
					//Relay
					WRelay* relay = new WRelay(supportedDevices[getDeviceType()].relayPins[i], true);
					if (isSupportingW8212()) {
						mainLedRelay = WProperty::createOnOffProperty("relay", "Relay");
						relay->setProperty(mainLedRelay);
					} else if (getDeviceMode() == MODE_SEPARATE_RELAY_PROPERTY) {
						WProperty* relayProperty = WProperty::createOnOffProperty("relay", "Relay");
						this->addProperty(relayProperty);
						relay->setProperty(relayProperty);
					} else {
						relay->setProperty(onOffProperty);
					}
					this->addPin(relay);
				}
			}
			if ((supportedDevices[getDeviceType()].switchPins[i] != NO_PIN) && (onOffProperty != nullptr)) {
				//Button
				//If more buttons than relais, than assign all buttons to last relay, e.g. SonoffBasic
				WSwitch* button = new WSwitch(supportedDevices[getDeviceType()].switchPins[i], supportedDevices[getDeviceType()].switchModes[i]);
				button->setProperty(onOffProperty);
				this->addPin(button);
			}

		}
		if (isSupportingW8212()) {
			ledStrip = new W2812Led(network, PIN_W2812, 80);
			ledStrip->setLedProgram(ledProgram->getByte() - 1);
			this->addPin(ledStrip);
			this->addProperty(ledProgram);
			this->addProperty(ledStrip->getColor());
			this->addProperty(ledStrip->getBrightness());
			onOffProperty->setOnChange([this](WProperty* property){
				switch (ledProgram->getByte()) {
				case 0:
					if (mainLedRelay != nullptr) {
						mainLedRelay->setBoolean(onOffProperty->getBoolean());
					}
					break;
				default:
					ledStrip->setLedProgram(ledProgram->getByte() - 1);
					ledStrip->setOn(onOffProperty->getBoolean());
				}
			});
			ledProgram->setOnChange([this](WProperty* property){
				if (onOffProperty->getBoolean()) {
					switch (ledProgram->getByte()) {
					case 0:
						ledStrip->setOn(false);
						if (mainLedRelay != nullptr) {
							mainLedRelay->setBoolean(true);
						}
						break;
					default:
						if (mainLedRelay != nullptr) {
							mainLedRelay->setBoolean(false);
						}
						ledStrip->setLedProgram(ledProgram->getByte() - 1);
						ledStrip->setOn(true);
					}
				}
			});
		}
	}

	virtual void printConfigPage(ESP8266WebServer* webServer, WStringStream* page) {
	    network->notice(F("NeoDevice config page"));
    	page->printAndReplace(FPSTR(HTTP_CONFIG_PAGE_BEGIN), getId());
    	//deviceType
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_BEGIN), "Model:", "dt");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "0", (getDeviceType() == 0 ? "selected" : ""), "Neo Coolcam");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "1", (getDeviceType() == 1 ? "selected" : ""), "Sonoff Mini");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "2", (getDeviceType() == 2 ? "selected" : ""), "Sonoff Basic");
    	//page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "3", (getDeviceType() == 3 ? "selected" : ""), "Sonoff 4-channel");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "3", (getDeviceType() == 3 ? "selected" : ""), "Wemos: Relay at D1, Switch at D3");
    	page->print(FPSTR(HTTP_COMBOBOX_END));
    	//deviceMode
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_BEGIN), "Device Mode:", "dm");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "0", (getDeviceMode() == 0 ? "selected" : ""), "Button switches relay on or off");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "1", (getDeviceMode() == 1 ? "selected" : ""), "Separate relay property. Button doesn't switch relay.");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "2", (getDeviceMode() == 2 ? "selected" : ""), "No relay usage. Only button usage.");
    	page->print(FPSTR(HTTP_COMBOBOX_END));
			//showAsWebthingDevice
			page->printAndReplace(FPSTR(HTTP_CHECKBOX_OPTION), "sa", "sa", (showAsWebthingDevice->getBoolean() ? HTTP_CHECKED : ""), "", "Show as Mozilla Webthing device");
    	//supportingW2812
    	page->printAndReplace(FPSTR(HTTP_CHECKBOX), "sw", (this->isSupportingW8212() ? "checked" : ""), "Supports LED strip at GPIO04");

    	page->print(FPSTR(HTTP_CONFIG_SAVE_BUTTON));
	}

	void saveConfigPage(ESP8266WebServer* webServer, WStringStream* page) {
	  network->notice(F("Save NeoDevice config page"));
		this->showAsWebthingDevice->setBoolean(webServer->arg("sa") == HTTP_TRUE);
		this->deviceType->setByte(webServer->arg("dt").toInt());
		this->deviceMode->setByte(webServer->arg("dm").toInt());
		this->supportingW2812->setBoolean(webServer->arg("sw") == "true");
	}

protected:

	byte getDeviceType() {
		return this->deviceType->getByte();
	}

	byte getDeviceMode() {
		return this->deviceMode->getByte();
	}

	bool isSupportingW8212() {
		return supportingW2812->getBoolean();
	}

private:
	WProperty* deviceType;
	WProperty* deviceMode;
	WProperty* supportingW2812;
	WProperty* ledProgram;
	W2812Led* ledStrip;
	WProperty* onOffProperty;
	WProperty* mainLedRelay;
	WProperty* showAsWebthingDevice;
};


#endif
