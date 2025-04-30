/***********************************************************************************
*                         Author: Abderrahmane Abdelouafi                          *
*                               File Name: SAI42.cpp                               *
*                      Creation Date: April 6, 2025 08:24 AM                       *
*                      Last Updated: April 23, 2025 01:45 PM                       *
*                               Source Language: cpp                               *
*                                                                                  *
*                             --- Code Description ---                             *
*  Implements the SAI class functions for managing the smart greenhouse. Handles   *
*    WiFi connection, authentication, API validation, and serving dynamic web      *
*                                    content.                                      *
***********************************************************************************/

#include "SAI42.hpp"

// Global watering state variables
bool manualWateringActive = false;
unsigned long waterEndTime = 0;
int waterDuration = 0;

// Constructor – initialize credentials and sensor object
SAI::SAI(const String& wifiSSID,
         const String& wifiPassword,
         const String& adminUser,
         const String& adminPassword,
         const String& serialKey)
  : server(80),
    ws("/ws"),
    lcd(0x27, 16, 2),
    dht(DHT_PIN, DHT_TYPE),
    currentHumidity(-1),
    currentTemperature(-1),
    currentLighting("unknown"),
    currentMoisture(-1),
    currentWeatherStatus("Unknown"),
    _wifiSSID(wifiSSID),
    _wifiPassword(wifiPassword),
    _adminUser(adminUser),
    _adminPassword(adminPassword),
    _serialKey(serialKey),
    apiKey(""),
    watering(false),
    displayState(DISPLAY_NORMAL),
    stateExpiration(0) {}

// begin: Initialize system
void SAI::begin() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup sensor pins
  pinMode(RAIN_PIN, INPUT_PULLUP);
  pinMode(PUMP_PIN, OUTPUT);  // CHANGED TO OUTPUT
  dht.begin();

  lcd.init();
  lcd.begin(21, 22);
  lcd.backlight();
  lcd.clear();
  connectToWiFi();
  randomSeed(analogRead(0));
  apiKey = generateRandomAPIKey(16);
  if (!LittleFS.begin()) {
    Serial.println("An error occurred while mounting LittleFS");
    return;
  }
  Serial.println("LittleFS mounted successfully");
  setupRoutes();

  ws.onEvent([this](AsyncWebSocket* server,
                    AsyncWebSocketClient* client,
                    AwsEventType type,
                    void* arg,
                    uint8_t* data,
                    size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo* info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len) {
        data[len] = 0;
        DynamicJsonDocument doc(256);
        if (!deserializeJson(doc, data)) {
          if (doc["command"] == "water" && doc["token"] == apiKey) {
            int duration = doc["time"] | 5;
            manualWateringActive = true;
            waterEndTime = millis() + (duration * 1000);
            Serial.printf("Watering for %ds via WS\n", duration);
          }
        }
      }
    }
  });
  server.addHandler(&ws);
  server.begin();
  Serial.println("HTTP Server started");
}

// updateSensors: Called every second
void SAI::updateSensors() {
  // Handle temporary display states
  if (displayState != DISPLAY_NORMAL) {
    if (millis() < stateExpiration) return;
    displayState = DISPLAY_NORMAL;
    lcd.clear();
  }

  // Read actual sensors
  currentTemperature = getTemperature();
  currentHumidity = getHumidity();
  currentLighting = getLighting();
  currentMoisture = getMoisture();
  currentWeatherStatus = getWeather();
  String pumpState = getPumpStatus();
  String plantState = computePlantStatus(currentMoisture);

  // decide pump control: automatic OR manual
  bool shouldAutoWater = (currentMoisture < 25) && (currentWeatherStatus == "Clear");
  bool shouldManualWater = manualWateringActive && (millis() < waterEndTime);
  bool pumpOn = shouldAutoWater || shouldManualWater;
  digitalWrite(PUMP_PIN, pumpOn ? LOW : HIGH);  // INVERTED LOGIC
  pumpState = pumpOn ? "ON" : "OFF";

  // Display on LCD
  lcd.clear();
  // line 1: temperature & humidity
  char buf1[17];
  snprintf(buf1, sizeof(buf1), "T:%dC H:%d%%", currentTemperature, currentHumidity);
  printCentered(lcd, String(buf1), 0, 16);

  // line 2: pump status AND plant status
  String plantAbbrev = plantState.substring(0, 3);
  char buf2[17];
  snprintf(buf2, sizeof(buf2), "Pmp:%s Plt:%s",
           pumpOn ? "ON" : "OFF",
           plantAbbrev.c_str());
  printCentered(lcd, String(buf2), 1, 16);

  // Broadcast via WebSocket
  unsigned long remaining = 0;
  if (manualWateringActive && waterEndTime > millis()) {
    remaining = (waterEndTime - millis()) / 1000;
  }

  DynamicJsonDocument doc(256);
  doc["temperature"] = currentTemperature;
  doc["humidity"] = currentHumidity;
  doc["lighting"] = currentLighting;
  doc["moisture"] = currentMoisture;
  doc["weather"] = currentWeatherStatus;
  doc["pumpStatus"] = pumpState;
  doc["plantStatus"] = plantState;
  doc["countdown"] = remaining;
  String out;
  serializeJson(doc, out);
  ws.textAll(out);
}

// updateWatering: Called every second
void SAI::updateWatering() {
  if (manualWateringActive && millis() >= waterEndTime) {
    manualWateringActive = false;
    Serial.println("Watering finished");
  }
}

// GETAPIKey: Returns the API key
const String SAI::getApiKey() const {
  return apiKey;
}

// sendFSContent: Read file from LittleFS into a String
void SAI::sendFSContent(AsyncWebServerRequest* request,
                        const char* filePath,
                        const char* contentType,
                        int code,
                        const Replacement* replacements,
                        int count) {
  File file = LittleFS.open(filePath, "r");
  if (!file) {
    Serial.print("Failed to open file: ");
    Serial.println(filePath);
    handleNotFound(request);
    return;
  }
  String page;
  while (file.available()) page += (char)file.read();
  file.close();
  if (replacements && count > 0) {
    for (int i = 0; i < count; i++) {
      page.replace(replacements[i].placeholder, replacements[i].value);
    }
  }
  request->send(code, contentType, page);
}

// connectToWiFi: Handles WiFi connection and LCD status
void SAI::connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(_wifiSSID.c_str(), _wifiPassword.c_str());
  lcd.clear();
  printCentered(lcd, "Connecting WiFi", 0, 16);
  printCentered(lcd, "Please wait...", 1, 16);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  printCentered(lcd, "WiFi Connected", 0, 16);
  printCentered(lcd, WiFi.localIP().toString(), 1, 16);
  displayState = DISPLAY_WIFI_CONNECTED;
  stateExpiration = millis() + 2500;
}

// printCentered: Helper to center text on LCD
void SAI::printCentered(LiquidCrystal_I2C& lcd,
                        const String& text, int row, int columns) {
  int startCol = (columns - text.length()) / 2;
  lcd.setCursor(startCol, row);
  lcd.print(text);
}

// setupRoutes: Register HTTP routes
void SAI::setupRoutes() {
  server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleRoot(request);
  });
  server.on("/login", HTTP_ANY, [this](AsyncWebServerRequest* request) {
    handleLogin(request);
  });
  server.on("/dashboard", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleDashboard(request);
  });
  server.on("/temperature", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleTemperature(request);
  });
  server.on("/humidity", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleHumidity(request);
  });
  server.on("/lighting", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleLighting(request);
  });
  server.on("/moisture", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleMoisture(request);
  });
  server.on("/weatherStatus", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleWeather(request);
  });
  server.on("/pumpStatus", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleIsWatering(request);
  });
  server.on("/plantStatus", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handlePlantStatus(request);
  });
  server.on("/water", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleWater(request);
  });
  server.on("/error", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleError(request);
  });
  server.onNotFound([this](AsyncWebServerRequest* request) {
    handleNotFound(request);
  });
}

// Authentication & error handlers
bool SAI::isAuthenticated(AsyncWebServerRequest* request) {
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie");
    int start = cookie.indexOf("ESPSESSIONID=");
    if (start != -1) {
      int end = cookie.indexOf(";", start);
      String value = (end == -1) ? cookie.substring(start) : cookie.substring(start, end);
      value.replace("ESPSESSIONID=", "");
      value.trim();
      return (value == "1");
    }
  }
  return false;
}

bool SAI::ensureUserAuthenticated(AsyncWebServerRequest* request) {
  if (!isAuthenticated(request)) {
    handleUnauthorized(request);
    return false;
  }
  return true;
}

bool SAI::validateAPIKey(AsyncWebServerRequest* request) {
  if (!request->hasArg("token") || request->arg("token") != apiKey) {
    handlePermissionDenied(request);
    return false;
  }
  return true;
}

String SAI::generateRandomAPIKey(uint8_t length) {
  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  String key;
  for (uint8_t i = 0; i < length; i++) {
    key += charset[random(0, strlen(charset))];
  }
  Serial.print("Generated API Key: ");
  Serial.println(key);
  return key;
}

// Route handler implementations
void SAI::handleRoot(AsyncWebServerRequest* request) {
  Serial.println("Handling root route");
  if (!isAuthenticated(request)) {
    request->send(LittleFS, "/index.html", "text/html");
  } else {
    Serial.println("User is authenticated, redirecting to dashboard");
    request->redirect("/dashboard");
  }
}

void SAI::handleLogin(AsyncWebServerRequest* request) {
  // Logout logic
  if (request->hasArg("action") && request->arg("action") == "logout") {
    AsyncWebServerResponse* response = request->beginResponse(301, "text/plain", "");
    response->addHeader("Set-Cookie", "ESPSESSIONID=0; path=/");
    response->addHeader("Location", "/login");
    response->addHeader("Cache-Control", "no-cache");
    request->send(response);
    return;
  }

  String message;

  // Check for error or info messages from query parameters
  if (request->hasArg("error")) {
    message = "<p style='color: red; text-align: center;'>Error: " + request->arg("error") + "</p>";
  } else if (request->hasArg("info")) {
    message = "<p style='color: cornflowerblue; text-align: center;'>Info: " + request->arg("info") + "</p>";
  }

  // Process recovery request
  if (request->hasArg("recover") && request->hasArg("key")) {
    String key = request->arg("key");
    if (key.length() == 5) {
      if (key == _serialKey) {
        Serial.print("Recovered credentials: Username: ");
        Serial.print(_adminUser);
        Serial.print(" Password: ");
        Serial.println(_adminPassword);
        lcd.clear();
        printCentered(lcd, "Username: " + _adminUser, 0, 16);
        printCentered(lcd, "Pass: " + _adminPassword, 1, 16);
        displayState = DISPLAY_RECOVERY_INFO;
        stateExpiration = millis() + 3000;

        // Redirect to login with success message
        AsyncWebServerResponse* response = request->beginResponse(301, "text/plain", "");
        response->addHeader("Location", "/login?info=Credntials%20recovered%20successfully");
        response->addHeader("Cache-Control", "no-cache");
        request->send(response);
        return;
      } else {
        message = "<p style='color: red; text-align: center;'>Error: Invalid serial key.</p>";
      }
    } else {
      message = "<p style='color: red; text-align: center;'>Error: Serial key must be 5 characters.</p>";
    }
  }

  // Process login submission
  if (request->hasArg("USERNAME") && request->hasArg("PASSWORD")) {
    if (request->arg("USERNAME") == _adminUser && request->arg("PASSWORD") == _adminPassword) {
      AsyncWebServerResponse* response = request->beginResponse(301, "text/plain", "");
      response->addHeader("Set-Cookie", "ESPSESSIONID=1; path=/");
      response->addHeader("Location", "/dashboard");
      response->addHeader("Cache-Control", "no-cache");
      request->send(response);
      return;
    } else {
      request->redirect("/login?error=Wrong%20username%20or%20password");
      return;
    }
  }

  // Render login page with message
  Replacement reps[] = {
    { "<-- ERROR_PLACEHOLDER -->",
      message }
  };
  sendFSContent(request, "/login.html", "text/html", 200, reps, sizeof(reps) / sizeof(Replacement));
}

void SAI::handleDashboard(AsyncWebServerRequest* request) {
  Serial.println("Handling dashboard route");
  if (!isAuthenticated(request)) {
    request->redirect("/login?error=Please%20log%20in%20first");
    return;
  }
  Replacement reps[] = {
    { "<-- API_KEY_PLACEHOLDER -->",
      apiKey }
  };
  sendFSContent(request, "/dashboard.html", "text/html", 200, reps, sizeof(reps) / sizeof(Replacement));
}

void SAI::handleError(AsyncWebServerRequest* request) {
  request->send(LittleFS, "/errors.html", "text/html");
}

void SAI::handleHumidity(AsyncWebServerRequest* request) {
  if (!ensureUserAuthenticated(request) || !validateAPIKey(request)) return;
  request->send(200, "text/plain", String(getHumidity()));
}

void SAI::handleTemperature(AsyncWebServerRequest* request) {
  if (!ensureUserAuthenticated(request) || !validateAPIKey(request)) return;
  request->send(200, "text/plain", String(getTemperature()));
}

void SAI::handleLighting(AsyncWebServerRequest* request) {
  if (!ensureUserAuthenticated(request) || !validateAPIKey(request)) return;
  request->send(200, "text/plain", getLighting());
}

void SAI::handleMoisture(AsyncWebServerRequest* request) {
  if (!ensureUserAuthenticated(request) || !validateAPIKey(request)) return;
  request->send(200, "text/plain", String(getMoisture()));
}

void SAI::handleWeather(AsyncWebServerRequest* request) {
  if (!ensureUserAuthenticated(request) || !validateAPIKey(request)) return;
  request->send(200, "text/plain", getWeather());
}

void SAI::handleIsWatering(AsyncWebServerRequest* request) {
  if (!ensureUserAuthenticated(request) || !validateAPIKey(request)) return;
  request->send(200, "text/plain", manualWateringActive ? "ON" : "OFF");
}

void SAI::handlePlantStatus(AsyncWebServerRequest* request) {
  if (!ensureUserAuthenticated(request) || !validateAPIKey(request)) return;
  request->send(200, "text/plain", computePlantStatus(getMoisture()));
}

void SAI::handleWater(AsyncWebServerRequest* request) {
  if (!ensureUserAuthenticated(request) || !validateAPIKey(request)) return;
  int waterTime = request->hasArg("time") ? request->arg("time").toInt() : 5;
  manualWateringActive = true;
  waterDuration = waterTime;
  waterEndTime = millis() + (waterTime * 1000);
  request->send(200, "text/plain", "Watering started");
}

void SAI::handlePermissionDenied(AsyncWebServerRequest* request) {
  request->redirect("/error?code=403");
}

void SAI::handleNotFound(AsyncWebServerRequest* request) {
  request->redirect("/error?code=404");
}

void SAI::handleUnauthorized(AsyncWebServerRequest* request) {
  request->redirect("/error?code=401");
}

void SAI::handleInternalServerErr(AsyncWebServerRequest* request) {
  request->redirect("/error?code=500");
}

// Sensor helper implementations
int SAI::readSoilPercent() {
  int raw = analogRead(SOIL_PIN);
  const int dryRaw = 3000;
  const int wetRaw = 1000;
  int pct = map(raw, dryRaw, wetRaw, 0, 100);
  return constrain(pct, 0, 100);
}

bool SAI::isDaylight() {
  return analogRead(LDR_PIN) > 2000;
}

bool SAI::isRaining() {
  return digitalRead(RAIN_PIN) == LOW;
}

bool SAI::isPumpOn() {
  return digitalRead(PUMP_PIN) == LOW;
}

// Sensor getters
int SAI::getHumidity() {
  float h = dht.readHumidity();
  return isnan(h) ? currentHumidity : int(h);
}

int SAI::getTemperature() {
  float t = dht.readTemperature();
  return isnan(t) ? currentTemperature : int(t);
}

String SAI::getLighting() {
  return isDaylight() ? "Night" : "Day";
}

int SAI::getMoisture() {
  return readSoilPercent();
}

String SAI::getWeather() {
  return isRaining() ? "Rain" : "Clear";
}

String SAI::getPumpStatus() {
  return isPumpOn() ? "ON" : "OFF";
}

// Compute plant status from moisture%
String SAI::computePlantStatus(int moisture) {
  if (moisture < 25) return "Dry";
  else if (moisture < 50) return "Thirsty";
  else if (moisture <= 75) return "Healthy";
  else return "Overwatered";
}
