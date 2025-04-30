// File: SAI42.hpp
/***********************************************************************************
*                         Author: Abderrahmane Abdelouafi                          *
*                               File Name: SAI42.hpp                               *
*                      Creation Date: April 6, 2025 08:24 AM                       *
*                      Last Updated: April 23, 2025                                *
*                               Source Language: cpp                               *
*                                                                                  *
*                             --- Code Description ---                             *
*  Defines the SAI class and all needed libraries and variables for managing the   *
*                smart automated irrigation system, now including soil moisture    *
*                    and rainfall (weather) detection.                             *
***********************************************************************************/

#ifndef SAI42_HPP
#define SAI42_HPP

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#ifndef ESP32
#error "This code only supports ESP32. Please select an ESP32 board in the IDE."
#endif

#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Sensor pin definitions
static const uint8_t DHT_PIN = 4;
static const uint8_t DHT_TYPE = DHT22;
static const uint8_t SOIL_PIN = 35;
static const uint8_t LDR_PIN = 34;
static const uint8_t RAIN_PIN = 15;
static const uint8_t PUMP_PIN = 5;

// Global watering state variables
extern bool manualWateringActive;
extern unsigned long waterEndTime;
extern int waterDuration;

class SAI {
public:
  struct Replacement {
    const char *placeholder;
    String value;
  };

  enum DisplayState {
    DISPLAY_NORMAL = 0,
    DISPLAY_WIFI_CONNECTED,
    DISPLAY_RECOVERY_INFO
  };

  SAI(const String &wifiSSID,
      const String &wifiPassword,
      const String &adminUser,
      const String &adminPassword,
      const String &serialKey);

  const String getApiKey() const;
  void begin();
  void updateSensors();
  void updateWatering();

private:
  AsyncWebServer server;
  AsyncWebSocket ws;
  LiquidCrystal_I2C lcd;
  DHT dht;

  int currentHumidity;
  int currentTemperature;
  String currentLighting;
  int currentMoisture;
  String currentWeatherStatus;

  String _wifiSSID, _wifiPassword, _adminUser, _adminPassword, _serialKey;
  String apiKey;
  bool watering;
  DisplayState displayState;
  unsigned long stateExpiration;

  // FS response
  void sendFSContent(AsyncWebServerRequest *request,
                     const char *filePath,
                     const char *contentType,
                     int code,
                     const Replacement *replacements,
                     int count);
  void sendFSContent(AsyncWebServerRequest *request,
                     const char *filePath,
                     const char *contentType,
                     int code) {
    sendFSContent(request, filePath, contentType, code, nullptr, 0);
  }

  void connectToWiFi();
  void printCentered(LiquidCrystal_I2C &lcd, const String &text, int row, int columns);
  bool isAuthenticated(AsyncWebServerRequest *request);
  bool ensureUserAuthenticated(AsyncWebServerRequest *request);
  bool validateAPIKey(AsyncWebServerRequest *request);
  String generateRandomAPIKey(uint8_t length = 16);
  String computePlantStatus(int moisturePercent);

  // hardware helpers
  int readSoilPercent();
  bool isDaylight();
  bool isRaining();
  bool isPumpOn();

  // override getters
  int getHumidity();
  int getTemperature();
  String getLighting();
  int getMoisture();
  String getWeather();
  String getPumpStatus();

  void setupRoutes();
  void handleRoot(AsyncWebServerRequest *request);
  void handleLogin(AsyncWebServerRequest *request);
  void handleDashboard(AsyncWebServerRequest *request);
  void handleError(AsyncWebServerRequest *request);
  void handleHumidity(AsyncWebServerRequest *request);
  void handleTemperature(AsyncWebServerRequest *request);
  void handleLighting(AsyncWebServerRequest *request);
  void handleIsWatering(AsyncWebServerRequest *request);
  void handlePlantStatus(AsyncWebServerRequest *request);
  void handleWater(AsyncWebServerRequest *request);
  void handleMoisture(AsyncWebServerRequest *request);
  void handleWeather(AsyncWebServerRequest *request);
  void handlePermissionDenied(AsyncWebServerRequest *request);
  void handleNotFound(AsyncWebServerRequest *request);
  void handleUnauthorized(AsyncWebServerRequest *request);
  void handleInternalServerErr(AsyncWebServerRequest *request);
};

#endif  // SAI42_HPP