<h1 align="center">
SAI42 - Smart Automated Irrigation System
</h1>

![SAI42](https://socialify.git.ci/ababdelo/SAI_42/image?font=Source+Code+Pro&language=1&name=1&owner=1&pattern=Circuit+Board&theme=Light)

<p align="center">
  <img src="https://img.shields.io/github/last-commit/ababdelo/SAI_42?style=flat-square" /> &nbsp;&nbsp;
  <img src="https://img.shields.io/github/commit-activity/m/ababdelo/SAI_42?style=flat-square" /> &nbsp;&nbsp;
  <img src="https://img.shields.io/github/followers/ababdelo" /> &nbsp;&nbsp;
  <img src="https://api.visitorbadge.io/api/visitors?path=https%3A%2F%2Fgithub.com%2Fababdelo%2FSAI42&label=Repository%20Visits&countColor=%230c7ebe&style=flat&labelStyle=none"/> &nbsp;&nbsp;
  <img src="https://img.shields.io/github/stars/ababdelo/SAI_42" /> &nbsp;&nbsp;
  <img src="https://img.shields.io/github/contributors/ababdelo/SAI_42?style=flat-square" />
</p>

---

## ⚠️ Notice

This project is under active development. Features, architecture and hosting strategy may evolve over time.

---

## 📑 Table of Contents

- [⚠️ Notice](#️-notice)
- [📑 Table of Contents](#-table-of-contents)
- [📝 Overview](#-overview)
- [💡 Concept](#-concept)
- [🚀 Key Functions](#-key-functions)
- [🛠️ Materials Required](#️-materials-required)
- [🔌 Wiring Diagram](#-wiring-diagram)
- [🔧 Connection Details](#-connection-details)
- [📂 Upload Requirements](#-upload-requirements)
- [👤 Author](#-author)
- [📄 License](#-license)
- [🤝Contributing](#contributing)
- [☎️Contact](#️contact)

---

## 📝 Overview

Smart Irrigation System (SAI42) is a data-driven, ESP32-powered solution for automated plant watering. It continuously monitors:

- **Environmental metrics:** Temperature, humidity, rainfall, ambient brightness
- **Plant metrics:** Soil moisture

An intelligent threshold-based controller decides when to water, while a secure web dashboard provides:

- Live sensor readings & pump status
- Historical data logs & charts
- Manual override & alert notifications

![Project Illustration](https://raw.githubusercontent.com/edunwant42/Asset42Archive/refs/heads/main/SAI42/assets/images/SAI42.webp)

---

## 💡 Concept

The system is organized into three layers:

1. **Sensing Layer**  
   DHT22, LDR, rain sensor & capacitive soil probe sample data every second via ESP32 inputs.

2. **Processing Layer**  
   On-board filtering and threshold logic determine irrigation needs based on moisture and weather.

3. **Actuation & Control Layer**  
   A relay switches the water pump; all readings and actions are logged to LittleFS for export.

ESP32 also hosts the web UI over Wi-Fi, enabling remote monitoring and control.

---

## 🚀 Key Functions

1. **Real-Time Data Acquisition**

   - **Temperature & Humidity:** DHT22
   - **Ambient Light:** LDR Module
   - **Rain Detection:** Rain sensor
   - **Soil Moisture:** Capacitive soil probe

2. **Intelligent Irrigation Control**

   - Threshold algorithms trigger pump when soil is dry and no rain is detected
   - Manual watering override via web UI or WebSocket command

3. **Web Platform & IoT Integration**

   - **Home Page:** Project overview & features
     <div style="display:flex; gap:1rem; flex-wrap:wrap;">
    <br>
    <img src="https://raw.githubusercontent.com/edunwant42/Asset42Archive/refs/heads/main/SAI42/assets/images/SAI42%20-%20HOME.webp" alt="Home Preview" width="100%" />
   </div>

   - **Login Page:** Secure authentication & credential recovery
     <div align="center">
     <br>
     <img src="https://raw.githubusercontent.com/edunwant42/Asset42Archive/refs/heads/main/SAI42/assets/images/SAI42%20-%20LOGIN.webp" alt="Login Preview" width="45%" />
     <img src="https://raw.githubusercontent.com/edunwant42/Asset42Archive/refs/heads/main/SAI42/assets/images/SAI42%20-%20LOGIN%20-%20RECOVER.webp" alt="Credential Recovery" width="45%" />
   </div>

   - **Dashboard:** Live data, pump control, WebSocket updates
   <div align="center">
   <br>
     <img src="https://raw.githubusercontent.com/edunwant42/Asset42Archive/refs/heads/main/SAI42/assets/images/SAI42%20-%20DASHBOARD%20-%20PC.webp" alt="Dashboard Page PC Preview" width="100%">
     <img src="https://raw.githubusercontent.com/edunwant42/Asset42Archive/refs/heads/main/SAI42/assets/images/SAI42%20-%20DASHBOARD%20-%20MOBILE.webp" alt="Dashboard Page Mobile Preview" width="45%">
    </div>

---

## 🛠️ Materials Required

| Component                   | Qty | Description                       |
| --------------------------- | :-: | --------------------------------- |
| ESP32                       |  1  | Microcontroller & web server      |
| DHT22                       |  1  | Temperature & humidity sensor     |
| Soil Moisture Sensor        |  1  | Capacitive moisture probe         |
| LDR Module                  |  1  | Ambient light detection           |
| Rain Sensor Module          |  1  | Rain detection digital module     |
| 1-Channel Relay Module      |  1  | Water pump switch                 |
| Water Pump                  |  1  | 5 V irrigation pump               |
| LCD I²C Module _(optional)_ |  1  | On-site status display            |
| 3-Pin ON/OFF Switch         |  1  | Main power control                |
| LM2596 Regulator _(opt.)_   |  1  | 5 V→3.3 V power regulation (opt.) |

---

## 🔌 Wiring Diagram

![Wiring Diagram](https://raw.githubusercontent.com/edunwant42/Asset42Archive/refs/heads/main/SAI42/assets/images/SAI42%20-%20WIRING.webp)

---

## 🔧 Connection Details

| Type            | Module / Component   | Wiring Notes                                                 |
| --------------- | -------------------- | ------------------------------------------------------------ |
| **Sensor**      | DHT22                | Data → GPIO4 (pull-up), VCC→3.3 V, GND→GND                   |
| **Sensor**      | Soil Moisture Sensor | Analog Out → GPIO35, VCC→3.3 V, GND→GND                      |
| **Sensor**      | LDR Module           | Voltage Divider → GPIO34, VCC→3.3 V, GND→GND                 |
| **Sensor**      | Rain Sensor          | Digital Out → GPIO15, VCC→3.3 V, GND→GND                     |
| **Actuator**    | Relay Module         | IN → GPIO5, VCC→5 V, GND→GND (common with ESP32)             |
| **Actuator**    | Water Pump           | Powered by 5 V rail; relay NO/COM in series with pump ground |
| **Display**     | LCD I²C              | SDA→GPIO21, SCL→GPIO22, VCC→5 V, GND→GND                     |
| **Power Rails** | ESP32 VIN            | 5 V supply (LM2596 or USB) → VIN, GND→GND                    |
| **Power Rails** | 3.3 V & GND          | Shared by all sensors (do not exceed 3.3 V on ESP32 GPIOs)   |
| **Power Rails** | 5 V Rail             | Relay, pump & display power; common GND with ESP32           |

---

## 📂 Upload Requirements

1. **Arduino IDE**

   - Install latest stable version of Arduino IDE.
   - Install ESP32 USB-to-Serial Bridge Driver (either [CP2102](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers) or [CH340G](https://www.wch-ic.com/downloads/CH341SER_ZIP.html)) if not already installed.
   - Install ESP32 board support:
     [Getting Started with ESP32](https://lastminuteengineers.com/getting-started-with-esp32/)

2. **Libraries**  
   ` ArduinoJson, AsyncTCP, ESPAsyncWebServer, LiquidCrystal_I2C, DHT sensor library, LittleFS, WebSockets`

3. **Uploading Sketch**

   - Open the sketch in Arduino IDE and select the correct board and port from the Tools menu.
   - Configure your SAI42 credentials in the `SAI42.ino` file on the SAI42 object instance.

   ```cpp
    /* Create the SAI object with 5 parameters:
    WiFi SSID, WiFi Password, admin username, admin password, and the device serial key. */
    SAI SAI42("SSID", "PASSWORD", "user", "admin", "E4D2U");
   ```

   - Upload the sketch to the ESP32.

4. **LittleFS Plugin**

- Follow tutorial: [Install ESP32 LittleFS in Arduino IDE 2.0](https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/)
- After uploading your sketch to the ESP32, press `ctrl + shift + P` to open the command palette and select `Upload LittleFS` to flash web UI files to the ESP32.
- Open the Serial Monitor (Ctrl + Shift + M) and set the baud rate to 115200. You should see the ESP32 connecting to Wi-Fi and starting the web server.
- Browse to the IP address shown in the Serial Monitor or in the LCD display to access the web dashboard.

## 👤 Author

This project is developed by [Abderrahmane Abdelouafi](https://edunwant42.tech)

## 📄 License

This project is licensed under the **ED42 Non-Commercial License v1.0**. See the [LICENSE](license.md) file for more details.

## 🤝Contributing

Contributions and suggestions to enhance this project are welcome! Please feel free to submit a pull request or open an issue.

## ☎️Contact

For any inquiries or collaboration opportunities, please reach out to me at:

<p align="center" style="display: inline;">
    <a href="mailto:ababdelo.ed42@gmail.com"> <img src="https://img.shields.io/badge/Gmail-EA4335?style=flat&logo=gmail&logoColor=white"/></a>&nbsp;&nbsp;
    <a href="https://www.linkedin.com/in/ababdelo"> <img src="https://img.shields.io/badge/LinkedIn-0A66C2?style=flat&logo=linkedin&logoColor=white"/></a>&nbsp;&nbsp;
    <a href="https://github.com/ababdelo"> <img src="https://img.shields.io/badge/GitHub-181717?style=flat&logo=github&logoColor=white"/></a>&nbsp;&nbsp;
    <a href="https://www.instagram.com/edunwant42"> <img src="https://img.shields.io/badge/Instagram-E4405F?style=flat&logo=instagram&logoColor=white"/></a>&nbsp;&nbsp;
</p>

<p align="center">Thanks for stopping by and taking a peek at my work!</p>
