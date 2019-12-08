#ifndef W_NEO_COOLCAM_H
#define W_NEO_COOLCAM_H

#include "../../WAdapter/Wadapter/WDevice.h"
#include "../../WAdapter/Wadapter/WSwitch.h"
#include "../../WAdapter/Wadapter/WRelay.h"
#include "../../WAdapter/Wadapter/WProperty.h"

const static char HTTP_CONFIG_PAGE[]         PROGMEM = R"=====(
<form method='get' action='saveDeviceConfiguration_{di}'>
        	<div>
        		<select name="dt">        		
					<option value="0" {0}>Neo Coolcam</option>
					<option value="1" {1}>Sonoff Mini</option>
                    <option value="2" {2}>Sonoff Basic</option>
                    <option value="3" {3}>Sonoff 4-channel</option>
                    <option value="4" {4}>Wemos: Relay at D1, Switch at D3</option>			
				</select>
        	</div>
		<div>
        	<select name="dm">        		
				<option value="0" {m0}>Button switches relay on or off</option>
				<option value="1" {m1}>Separate relay property. Button doesn't switch relay.</option>
                <option value="2" {m2}>No relay usage. Only button usage.</option>			
			</select>
        </div>
		<div>
			<button type='submit'>Save configuration</button>
		</div>
</form>
)=====";

#define COUNT_DEVICE_TYPES 5
#define COUNT_DEVICE_MODES 3
#define COUNT_MAX_RELAIS 4
#define NO_PIN 255
#define MODE_BUTTON_LINKED_TO_RELAY 0
#define MODE_SEPARATE_RELAY_PROPERTY 1
#define MODE_NO_RELAY_USAGE 2

#define PIN_SDA 3 //RX // green
#define PIN_SCL 1 //TX // yellow

struct SwitchDevices{
	byte statusLed;
    byte relayPins[COUNT_MAX_RELAIS];
    byte switchPins[COUNT_MAX_RELAIS];
    byte switchModes[COUNT_MAX_RELAIS];
};

static struct SwitchDevices supportedDevices [5] =
{
//LED  RELAY		                 SWITCHES
 { 4, {12, NO_PIN, NO_PIN, NO_PIN}, {13, NO_PIN, NO_PIN, NO_PIN}, {MODE_BUTTON, NO_PIN, NO_PIN, NO_PIN}}, //Neo Coolcam
 {13, {12, NO_PIN, NO_PIN, NO_PIN}, { 0,      4, NO_PIN, NO_PIN}, {MODE_BUTTON, MODE_SWITCH, NO_PIN, NO_PIN}}, //Sonoff Mini
 {13, {12, NO_PIN, NO_PIN, NO_PIN}, { 0, NO_PIN, NO_PIN, NO_PIN}, {MODE_BUTTON, NO_PIN, NO_PIN, NO_PIN}}, //Sonoff Basic
 {13, {12,      5,      4,     15}, { 0,      9,     10,     14}, {MODE_BUTTON, MODE_BUTTON, MODE_BUTTON, MODE_BUTTON}}, //Sonoff 4-channel
 { 2, { 5, NO_PIN, NO_PIN, NO_PIN}, { 0, NO_PIN, NO_PIN, NO_PIN}, {MODE_BUTTON, NO_PIN, NO_PIN, NO_PIN}} //Wemos: Relay at D1, Switch at D3
};

class WNeoDevice: public WDevice {
public:
	WNeoDevice(WNetwork* network)
	    	: WDevice(network, "switch", "switch", DEVICE_TYPE_ON_OFF_SWITCH) {
		this->providingConfigPage = true;
		this->deviceType = network->getSettings()->registerByte("deviceType", 0);
		this->deviceMode = network->getSettings()->registerByte("deviceMode", 0);
		//StatusLed
		if (supportedDevices[getDeviceType()].statusLed != NO_PIN) {
			this->statusLed = new WLed(supportedDevices[getDeviceType()].statusLed);
		}
		WOnOffProperty* onOffProperty = nullptr;
		for (int i = 0; i < COUNT_MAX_RELAIS; i++) {
			if (supportedDevices[getDeviceType()].relayPins[i] != NO_PIN) {
				//Property
				WStringStream pid(3, true);
				pid.print("on");
				WStringStream pname(8, true);
				pname.print("Switch");
				WStringStream rid(6, true);
				rid.print("relay");
				WStringStream rname(7, true);
				rname.print("Relay");
				if (i > 0) {
					char buffer[1];
					itoa(i, buffer, 10);
					pid.print(buffer);
					pname.print(" ");
					pname.print(buffer);
					rid.print(buffer);
					rname.print(" ");
					rname.print(buffer);
				}
				onOffProperty = new WOnOffProperty(pid.c_str(), pname.c_str());
				this->addProperty(onOffProperty);
				if (getDeviceMode() != MODE_NO_RELAY_USAGE) {
					//Relay
					WRelay* relay = new WRelay(supportedDevices[getDeviceType()].relayPins[i], true);
					if (getDeviceMode() == MODE_SEPARATE_RELAY_PROPERTY) {
						WOnOffProperty* relayProperty = new WOnOffProperty(rid.c_str(), rname.c_str());
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
	}

	virtual void printConfigPage(WStringStream* page) {
	    network->log()->notice(F("NeoDevice config page"));
    	page->printAndReplace(FPSTR(HTTP_CONFIG_PAGE_BEGIN), getId());
    	//deviceType
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_BEGIN), "Model:", "dt");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "0", (getDeviceType() == 0 ? "selected" : ""), "Neo Coolcam");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "1", (getDeviceType() == 1 ? "selected" : ""), "Sonoff Mini");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "2", (getDeviceType() == 1 ? "selected" : ""), "Sonoff Basic");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "3", (getDeviceType() == 1 ? "selected" : ""), "Sonoff 4-channel");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "4", (getDeviceType() == 1 ? "selected" : ""), "Wemos: Relay at D1, Switch at D3");
    	page->print(FPSTR(HTTP_COMBOBOX_END));
    	//deviceMode
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_BEGIN), "Device Mode:", "dm");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "0", (getDeviceType() == 0 ? "selected" : ""), "Button switches relay on or off");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "1", (getDeviceType() == 1 ? "selected" : ""), "Separate relay property. Button doesn't switch relay.");
    	page->printAndReplace(FPSTR(HTTP_COMBOBOX_ITEM), "2", (getDeviceType() == 1 ? "selected" : ""), "No relay usage. Only button usage.");
    	page->print(FPSTR(HTTP_COMBOBOX_END));

    	page->print(FPSTR(HTTP_CONFIG_SAVE_BUTTON));
	}

	void saveConfigPage(ESP8266WebServer* webServer) {
	    network->log()->notice(F("Save NeoDevice config page"));
		this->deviceType->setByte(webServer->arg("dt").toInt());
		this->deviceMode->setByte(webServer->arg("dm").toInt());
	}

protected:

	byte getDeviceType() {
		return this->deviceType->getByte();
	}

	byte getDeviceMode() {
		return this->deviceMode->getByte();
	}

private:
	WProperty* deviceType;
	WProperty* deviceMode;

};


#endif
