#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "wifi_config.h" // Include your configuration file

// API endpoint
const char* apiEndpoint = "https://members.heatsynclabs.org/space_api.json";

// Certificate fingerprint
const char* fingerprint = "E6:83:40:BA:54:E9:77:E0:BF:D5:6B:18:B4:FC:0F:39:FF:B1:93:ED";

// Define LED pin
const int ledPin = LED_BUILTIN;

// Variables for pulsating LED
bool isOpen = false; // Store API result
unsigned long lastPulseTime = 0; // Track time for pulsating effect

String getFormattedTime() {
  unsigned long currentMillis = millis();
  unsigned long seconds = (currentMillis / 1000) % 60;
  unsigned long minutes = (currentMillis / (1000 * 60)) % 60;
  unsigned long hours = (currentMillis / (1000 * 60 * 60)) % 24;

  char buffer[10];
  sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(buffer);
}

void makeApiCall() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    HTTPClient http;

    // Set fingerprint for secure connection
    client.setFingerprint(fingerprint);

    // Start the HTTP GET request
    http.begin(client, apiEndpoint);

    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP GET code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.print("[");
        Serial.print(getFormattedTime());
        Serial.print("] ");
        Serial.println("Response payload:");
        Serial.println(payload);

        // Parse JSON response
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          isOpen = doc["open"];
          Serial.printf("Parsed 'open' value: %s\n", isOpen ? "true" : "false");
        } else {
          Serial.print("Failed to parse JSON: ");
          Serial.println(error.c_str());
        }
      }
    } else {
      Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(10);

  // Initialize LED pin
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // Ensure LED is off initially (inverted logic)

  // Connect to WiFi
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Make the first API call immediately
  makeApiCall();
}

void loop() {
  static unsigned long lastApiCall = 0;

  // Check and make periodic API calls every 5 minutes
  if (millis() - lastApiCall >= 300000 || lastApiCall == 0) {
    makeApiCall();
    lastApiCall = millis();
  }

  // LED behavior based on "open" value
  if (isOpen) {
      // Fast pulsating effect (LED fades in and out)
      unsigned long currentMillis = millis();
      int brightness = abs((int)(512 - (currentMillis % 1024))); // Explicit cast to int
      analogWrite(ledPin, brightness / 2); // Scale down brightness for inverted logic
  } else {
      digitalWrite(ledPin, HIGH); // Turn LED off
  }

  delay(10); // Small delay to allow smooth pulsing
}


