/***********************************************************************************
*                         Author: Abderrahmane Abdelouafi                          *
*                               File Name: SAI42.ino                               *
*                      Creation Date: April 6, 2025 08:24 AM                       *
*                      Last Updated: April 10, 2025 01:49 PM                       *
*                               Source Language: cpp                               *
*                                                                                  *
*                             --- Code Description ---                             *
*                  Main code for the ESP8266/ESP32 SAI42 system.                   *
***********************************************************************************/

#include "SAI42.hpp"

// Create the SAI object with 5 parameters: WiFi SSID, WiFi Password, admin username, admin password, and the device serial key.
SAI sai42("SAI42", "P.07eOaMoSAI42q9W_", "user", "admin", "E4D2U");
unsigned long lastSensorUpdate = 0;
static const int sensorUpdateInterval = 1000;  // 1 second

void setup() {
  sai42.begin();
}

void loop() {
  if (millis() - lastSensorUpdate >= sensorUpdateInterval) {
    lastSensorUpdate = millis();
    sai42.updateSensors();
    sai42.updateWatering();
  }
}