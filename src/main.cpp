#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include "secrets.h"  // WiFi credentials and AWS certificates

// ============== AWS IOT CONFIG ==============
const char* AWS_IOT_ENDPOINT = "a2iwaullg29s71-ats.iot.us-west-1.amazonaws.com";
const char* CLIENT_ID = "GreenGuardian";
const char* TOPIC_PUBLISH = "greenguardian/sensors";

// ============== PIN DEFINITIONS ==============
const int sensorPin = 34;      // Soil moisture sensor
const int relayPin = 25;       // Water pump relay

// ============== SENSOR OBJECT ==============
Adafruit_AHTX0 aht;

// ============== WATERING SETTINGS ==============
const int dryThreshold = 40;
const int checkInterval = 15000;
const int pumpDuration = 1000;

// ============== CALIBRATION VALUES ==============
int dryValue = 4095;
int wetValue = 0;

// ============== TIMING VARIABLES ==============
unsigned long lastCheckTime = 0;
unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 30000;  // Publish to AWS every 30 seconds

// ============== AWS OBJECTS ==============
WiFiClientSecure net;
MQTTClient client(512);

// ============== SENSOR DATA ==============
int hydration = 0;
float tempC = 0;
float tempF = 0;
float humidity = 0;
String moistureStatus = "";

// ============== CONNECT TO WIFI ==============
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ============== CONNECT TO AWS IOT ==============
void connectAWS() {
  net.setCACert(AWS_ROOT_CA);
  net.setCertificate(AWS_DEVICE_CERT);
  net.setPrivateKey(AWS_PRIVATE_KEY);

  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  Serial.print("Connecting to AWS IoT");
  
  while (!client.connect(CLIENT_ID)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println();
  Serial.println("AWS IoT connected!");
}

// ============== PUBLISH SENSOR DATA TO AWS ==============
void publishSensorData() {
  String payload = "{";
  payload += "\"soilHydration\":" + String(hydration) + ",";
  payload += "\"moistureStatus\":\"" + moistureStatus + "\",";
  payload += "\"temperatureF\":" + String(tempF, 1) + ",";
  payload += "\"temperatureC\":" + String(tempC, 1) + ",";
  payload += "\"humidity\":" + String(humidity, 1);
  payload += "}";

  if (client.publish(TOPIC_PUBLISH, payload)) {
    Serial.println(">>> Published to AWS IoT <<<");
    Serial.println(payload);
  } else {
    Serial.println("AWS publish failed!");
  }
}

// ============== READ ALL SENSORS ==============
void readSensors() {
  int rawSoil = analogRead(sensorPin);
  hydration = map(rawSoil, dryValue, wetValue, 0, 100);
  hydration = constrain(hydration, 0, 100);
  
  sensors_event_t humidity_event, temp_event;
  aht.getEvent(&humidity_event, &temp_event);
  
  tempC = temp_event.temperature;
  tempF = (tempC * 9.0 / 5.0) + 32.0;
  humidity = humidity_event.relative_humidity;
  
  if (hydration < 15) {
    moistureStatus = "VERY DRY";
  } else if (hydration < 30) {
    moistureStatus = "DRY";
  } else if (hydration < 40) {
    moistureStatus = "SLIGHTLY DRY";
  } else {
    moistureStatus = "MOIST";
  }
}

// ============== DISPLAY READINGS ==============
void displayReadings() {
  Serial.println("--- Current Readings ---");
  Serial.print("Soil Hydration: ");
  Serial.print(hydration);
  Serial.print("% [");
  Serial.print(moistureStatus);
  Serial.println("]");
  
  Serial.print("Temperature: ");
  Serial.print(tempF, 1);
  Serial.print("F (");
  Serial.print(tempC, 1);
  Serial.println("C)");
  
  Serial.print("Humidity: ");
  Serial.print(humidity, 1);
  Serial.println("%");
}

// ============== SETUP ==============
void setup() {
  Serial.begin(9600);
  delay(2000);
  
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);  // Pump OFF (inverted relay)
  
  Wire.begin(21, 22);
  
  Serial.println("=================================");
  Serial.println("Auto Plant Watering System v1.0");
  Serial.println("=================================");
  
  if (!aht.begin()) {
    Serial.println("ERROR: Could not find ATH21B sensor!");
    Serial.println("Check wiring:");
    Serial.println("  VDD -> 3.3V");
    Serial.println("  SDA -> GPIO 21");
    Serial.println("  GND -> GND");
    Serial.println("  SCL -> GPIO 22");
    while (1) delay(10);
  }
  
  Serial.println("ATH21B sensor initialized!");
  Serial.println();
  
  connectWiFi();
  connectAWS();
  
  Serial.println();
  Serial.println("System ready! Monitoring plant...");
  Serial.println();
}

// ============== LOOP ==============
void loop() {
  client.loop();
  
  if (!client.connected()) {
    Serial.println("AWS IoT disconnected. Reconnecting...");
    connectAWS();
  }
  
  readSensors();
  displayReadings();
  
  unsigned long currentTime = millis();
  
  if (currentTime - lastCheckTime >= checkInterval) {
    lastCheckTime = currentTime;
    
    if (hydration < dryThreshold) {
      Serial.println();
      Serial.println(">>> WATERING ACTIVATED <<<");
      Serial.println("Pumping water for 1 second...");
      
      digitalWrite(relayPin, LOW);
      delay(pumpDuration);
      digitalWrite(relayPin, HIGH);
      
      Serial.println("Watering complete. Waiting 15 seconds...");
    } else {
      Serial.println("Soil moisture OK - no watering needed");
    }
  }
  
  if (currentTime - lastPublishTime >= publishInterval) {
    lastPublishTime = currentTime;
    publishSensorData();
  }
  
  Serial.println();
  delay(2000);
}