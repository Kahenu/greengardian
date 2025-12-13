# GreenGuardian

IoT-based automated plant watering system using ESP32, AWS IoT Core, and a web dashboard.

## Features

- Automatic watering when soil moisture drops below 40%
- Real-time temperature and humidity monitoring
- Cloud connectivity via AWS IoT Core (MQTT over TLS)
- Web dashboard with live data and historical charts

## Hardware

| Component | Pin |
|-----------|-----|
| Soil Moisture Sensor | GPIO 34 |
| AHT21B Temp/Humidity | SDA→GPIO 21, SCL→GPIO 22 |
| Water Pump Relay | GPIO 25 |

## Setup

1. Install [PlatformIO](https://platformio.org/)
2. Copy `src/secrets.h.example` to `src/secrets.h`
3. Fill in your WiFi credentials and AWS IoT certificates
4. Build and upload:
   ```
   pio run -t upload
   ```

## Configuration

Edit `src/secrets.h`:
```cpp
#define WIFI_SSID "your-wifi"
#define WIFI_PASSWORD "your-password"

const char AWS_ROOT_CA[] = "-----BEGIN CERTIFICATE-----\n...";
const char AWS_DEVICE_CERT[] = "-----BEGIN CERTIFICATE-----\n...";
const char AWS_PRIVATE_KEY[] = "-----BEGIN RSA PRIVATE KEY-----\n...";
```

## Dashboard

Open `dashboard.html` in a browser to view real-time sensor data.

