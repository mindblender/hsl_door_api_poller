#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "wifi_config.h" // Include your WiFi credentials

// Polling delay (5 minutes)
const unsigned long pollingDelay = 300000; // milliseconds

// API endpoint
const char* apiEndpoint = "https://members.heatsynclabs.org/space_api.json";

// LED pin
const int ledPin = LED_BUILTIN;

// Variables
bool isOpen = false;        // Current state from API
bool lastState = false;     // Last state to detect changes
unsigned long lastApiCall = 0;

// Debug function to print WiFi status
void printWifiStatus() {
  Serial.println("--- WiFi Status ---");
  Serial.print("SSID: "); Serial.println(WiFi.SSID());
  Serial.print("IP: "); Serial.println(WiFi.localIP());
  Serial.print("RSSI: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
  Serial.print("Status: "); Serial.println(WiFi.status());
  Serial.println("------------------");
}

// Return formatted time since boot
String getFormattedTime() {
  unsigned long currentMillis = millis();
  unsigned long seconds = (currentMillis / 1000) % 60;
  unsigned long minutes = (currentMillis / (1000 * 60)) % 60;
  unsigned long hours = (currentMillis / (1000 * 60 * 60)) % 24;
  char buffer[10];
  sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(buffer);
}

// Make the HTTPS API call
void makeApiCall() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[API] WiFi not connected!");
    printWifiStatus();
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();  // Temporary: ignores certificate

  HTTPClient http;

  Serial.println("[API] Starting HTTPS request...");
  if (!http.begin(client, apiEndpoint)) {
    Serial.println("[API] http.begin() failed!");
    return;
  }

  int httpCode = http.GET();
  Serial.printf("[API] HTTP GET returned: %d (%s)\n", httpCode, http.errorToString(httpCode).c_str());

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("[API] Response payload:");
    Serial.println(payload);

    // Parse JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      isOpen = doc["open"];
      Serial.printf("[API] Parsed 'open' value: %s\n", isOpen ? "true" : "false");
    } else {
      Serial.print("[API] Failed to parse JSON: ");
      Serial.println(error.c_str());
    }
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // LED off initially (inverted logic)

  // WiFi setup
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  printWifiStatus();

  // First API call
  makeApiCall();
  lastState = isOpen; // Initialize lastState

  // Initial message
  unsigned long secondsUntilNext = (pollingDelay - (millis() - lastApiCall)) / 1000;
  Serial.printf("Door %s. Checking again in %lu seconds\n", isOpen ? "Open" : "Closed", secondsUntilNext);
}

void loop() {
  // Periodic API call
  if (millis() - lastApiCall >= pollingDelay || lastApiCall == 0) {
    makeApiCall();
    lastApiCall = millis();

    unsigned long secondsUntilNext = pollingDelay / 1000;
    Serial.printf("Door %s. Checking again in %lu seconds\n", isOpen ? "Open" : "Closed", secondsUntilNext);
  }

  // LED behavior
  if (isOpen) {
    // Fast pulsating effect
    unsigned long currentMillis = millis();
    int brightness = abs((int)(512 - (currentMillis % 1024)));
    analogWrite(ledPin, brightness / 2); // scale down for inverted logic
  } else {
    digitalWrite(ledPin, HIGH); // LED off
  }

  delay(10); // smooth pulsing
}
