
#include <Wire.h>
#include <Adafruit_AHTX0.h>

// Pin definitions
const int sensorPin = 34;      // Soil moisture sensor
const int relayPin = 25;       // Water pump relay
// ATH21B uses I2C: SDA = GPIO21, SCL = GPIO22 (default for ESP32)

Adafruit_AHTX0 aht;

// Watering settings
const int dryThreshold = 40;   // Water if below 40% hydrated
const int checkInterval = 15000;  // Wait 15 seconds between checks
const int pumpDuration = 1000;    // Pump for 1 second

// Calibration values - adjust these based on your sensor
int dryValue = 4095;   // Raw value when completely dry
int wetValue = 0;      // Raw value when in water

unsigned long lastCheckTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);  // Pump OFF (inverted relay)
  
  Wire.begin(21, 22);  // SDA = 21, SCL = 22
  
  Serial.println("=================================");
  Serial.println("Auto Plant Watering System v1.0");
  Serial.println("=================================");
  
  // Initialize ATH21B sensor
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
  delay(2000);
}

void loop() {
  // Read soil moisture
  int rawSoil = analogRead(sensorPin);
  int hydration = map(rawSoil, dryValue, wetValue, 0, 100);
  hydration = constrain(hydration, 0, 100);
  
  // Read temperature and humidity from ATH21B
  sensors_event_t humidity_event, temp_event;
  aht.getEvent(&humidity_event, &temp_event);
  
  float tempC = temp_event.temperature;
  float tempF = (tempC * 9.0 / 5.0) + 32.0;
  float humidity = humidity_event.relative_humidity;
  
  // Display all readings
  Serial.println("--- Current Readings ---");
  Serial.print("Soil Hydration: ");
  Serial.print(hydration);
  Serial.print("%");
  
  if (hydration < 15) {
    Serial.println(" [VERY DRY]");
  } else if (hydration < 30) {
    Serial.println(" [DRY]");
  } else if (hydration < 40) {
    Serial.println(" [SLIGHTLY DRY]");
  } else {
    Serial.println(" [MOIST]");
  }
  
  Serial.print("Temperature: ");
  Serial.print(tempF, 1);
  Serial.print("°F (");
  Serial.print(tempC, 1);
  Serial.println("°C)");
  
  Serial.print("Humidity: ");
  Serial.print(humidity, 1);
  Serial.println("%");
  
  // Check if it's time to evaluate watering
  unsigned long currentTime = millis();
  
  if (currentTime - lastCheckTime >= checkInterval) {
    lastCheckTime = currentTime;
    
    if (hydration < dryThreshold) {
      Serial.println();
      Serial.println(">>> WATERING ACTIVATED <<<");
      Serial.println("Pumping water for 1 second...");
      
      digitalWrite(relayPin, LOW);  // Pump ON (inverted relay)
      delay(pumpDuration);
      digitalWrite(relayPin, HIGH);   // Pump OFF (inverted relay)
      
      Serial.println("Watering complete. Waiting 15 seconds...");
    } else {
      Serial.println("Soil moisture OK - no watering needed");
    }
  }
  
  Serial.println();
  delay(2000);  // Update display every 2 seconds
}