# Solid-State Peltier Thermal Cycling & Cooling Pad (CP01)

An IoT-enabled, high-precision Peltier thermal cycling platform designed for biological and bioscience laboratory testing applications[cite: 3, 4]. Powered by an ESP32 microcontroller, the system features dynamic PID temperature control (0.0°C to 50.0°C)[cite: 3], a 4-tile TEC1-12705 Peltier grid array[cite: 3, 6], active liquid-block cooling with an external DC water pump[cite: 3, 6], an integrated SSD1306/SH1106 OLED display, internal DC forced-air ventilation[cite: 3, 6], and a custom 3D-printed enclosure[cite: 3, 4].

The device embeds a web interface directly hosted on the ESP32, allowing users to program multi-step time/temperature profiles, adjust PID parameters, monitor real-time telemetry, and view live step metrics.

---

## Key Features

* **Closed-Loop PID Temperature Control:** Smooth, real-time control algorithm operating through an H-bridge driver with ±0.25°C stabilization accuracy[cite: 3, 6].
* **Multi-Tile Peltier Array:** 4x TEC1-12705 thermoelectric tiles clamped under liquid-jacketed aluminium heat sinks for rapid cooling and heating rates.
* **Active Water-Cooling Loop:** External 12V DC water pump circulating cooling liquid to remove hot-side Peltier heat during intensive cooling cycles[cite: 3, 6].
* **Web-Based Profile Manager:** On-device WebServer portal to configure multi-step temperature/duration profiles, update parameters dynamically, and trigger step restarts.
* **OLED Visual Status Display:** Local 0.91-inch OLED screen displaying Setpoint, Live Temperature, Step Count, Remaining Time, Wi-Fi SSID, and IP address[cite: 3, 5].
* **Safety & Interlocks:** Onboard buzzer alerts upon profile completion[cite: 5], with fail-safe features to stop power outputs during sensor faults[cite: 3, 6].

---

## Hardware Components

* ESP32 Microcontroller Board[cite: 3, 4]
* BTS7960 43A High-Current Motor Driver Module
* 4x TEC1-12705 Thermoelectric Peltier Modules (40x40mm)
* Anodized Aluminium Active Surface Cooling Plate & Liquid Water Blocks
* 12V DC Brushless Water Pump[cite: 3, 4, 6]
* 100k NTC Thermistor / Dallas Temperature Sensor[cite: 3, 5, 6]
* 0.91-inch SSD1306 I2C OLED Display (128x32)[cite: 3, 4, 5, 6]
* 12V DC Exhaust Cooling Fan[cite: 3, 4, 6]
* 12V 30A (360W) SMPS Power Supply[cite: 3, 4, 6]
* Custom 3D-Printed Desktop Enclosure[cite: 3, 4]

---

## Pin & Hardware Configuration

| Component / Function | Microcontroller Connection | Description |
| :--- | :--- | :--- |
| **OLED SDA** | I2C SDA Pin | Data Line for OLED Screen[cite: 3, 5] |
| **OLED SCL** | I2C SCL Pin | Clock Line for OLED Screen[cite: 3, 5] |
| **Driver RPWM** | GPIO 26 | Forward PWM signal to BTS7960 Driver[cite: 5] |
| **Driver LPWM** | GPIO 27 | Reverse PWM signal to BTS7960 Driver[cite: 5] |
| **Driver R_EN / L_EN** | GPIO 33 / GPIO 32 | Enable lines for BTS7960 Driver[cite: 5] |
| **Temperature Sensor** | GPIO 4 | OneWire / Analog Input Signal[cite: 5] |
| **Alert Buzzer** | GPIO 2 | Output trigger for cycle completion sound[cite: 5] |

---

## Software Stack & Dependencies

* **Platform:** Arduino Framework / ESP-IDF (ESP32 Board Package)[cite: 3, 5]
* **Core Libraries:** `WiFi`, `WebServer`, `PID_v1`, `Wire`, `Adafruit_GFX`, `Adafruit_SSD1306`, `OneWire`, `DallasTemperature`[cite: 5]
* **Frontend:** Embedded HTML5 / CSS3 web UI served natively over HTTP[cite: 3, 5]
