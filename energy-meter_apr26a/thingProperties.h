// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_OPTIONAL_PASS;    // Network password (use for WPA, or use as key for WEP)


float currentDisplay;
float energyDisplay;
float powerDisplay;

void initProperties(){

  ArduinoCloud.addProperty(currentDisplay, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(energyDisplay, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(powerDisplay, READ, ON_CHANGE, NULL);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
