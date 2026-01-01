#include <M5StickC.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "wifi_config.h" // Include your WiFi credentials

// Polling delay (5 minutes)
const unsigned long pollingDelay = 300000; // milliseconds

// API endpoint
const char* apiEndpoint = "https://members.heatsynclabs.org/space_api.json";

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

// Draw a proportionally scaled Christmas tree (68 pixels tall, 40 pixels wide)
void drawTree(int x, int y) {
  // Star on top (yellow) - scaled 2x
  M5.Lcd.fillRect(x+18, y, 4, 4, YELLOW);
  M5.Lcd.fillRect(x+16, y+2, 8, 4, YELLOW);

  // Top section of tree (green) - scaled 2x
  M5.Lcd.fillRect(x+14, y+8, 12, 4, GREEN);
  M5.Lcd.fillRect(x+12, y+12, 16, 4, GREEN);
  M5.Lcd.fillRect(x+10, y+16, 20, 4, GREEN);

  // Middle section (green) - scaled 2x
  M5.Lcd.fillRect(x+12, y+20, 16, 4, GREEN);
  M5.Lcd.fillRect(x+10, y+24, 20, 4, GREEN);
  M5.Lcd.fillRect(x+8, y+28, 24, 4, GREEN);
  M5.Lcd.fillRect(x+6, y+32, 28, 4, GREEN);

  // Bottom section (green) - scaled 2x
  M5.Lcd.fillRect(x+8, y+36, 24, 4, GREEN);
  M5.Lcd.fillRect(x+6, y+40, 28, 4, GREEN);
  M5.Lcd.fillRect(x+4, y+44, 32, 4, GREEN);
  M5.Lcd.fillRect(x+2, y+48, 36, 4, GREEN);
  M5.Lcd.fillRect(x, y+52, 40, 4, GREEN);

  // Trunk (brown) - scaled 2x
  M5.Lcd.fillRect(x+14, y+56, 12, 12, 0x8400);

  // Ornaments (colorful decorations) - scaled 2x
  M5.Lcd.fillRect(x+14, y+12, 4, 4, RED);      // Red - top
  M5.Lcd.fillRect(x+22, y+16, 4, 4, BLUE);     // Blue - top

  M5.Lcd.fillRect(x+14, y+24, 4, 4, MAGENTA);  // Magenta - middle
  M5.Lcd.fillRect(x+22, y+28, 4, 4, CYAN);     // Cyan - middle
  M5.Lcd.fillRect(x+10, y+32, 4, 4, RED);      // Red - middle

  M5.Lcd.fillRect(x+12, y+40, 4, 4, BLUE);     // Blue - bottom
  M5.Lcd.fillRect(x+24, y+44, 4, 4, YELLOW);   // Yellow - bottom
  M5.Lcd.fillRect(x+8, y+48, 4, 4, MAGENTA);   // Magenta - bottom
  M5.Lcd.fillRect(x+28, y+52, 4, 4, CYAN);     // Cyan - bottom
}

// Update the full display with door status
void updateStatusDisplay() {
  M5.Lcd.fillScreen(BLACK);

  if (isOpen) {
    // Display "OPEN for HAXMAS" in clean layout
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("OPEN");

    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(15, 42);
    M5.Lcd.println("for");

    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(5, 60);
    M5.Lcd.println("HAXMAS");

    // Draw proportionally scaled Christmas tree on the right side
    drawTree(117, 6);
  } else {
    // Display "CLOSED" in red text
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(5, 30);
    M5.Lcd.println("CLOSED");
  }
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
  // Initialize M5StickC
  M5.begin();
  M5.Lcd.setRotation(3); // Rotate display for better readability
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.println("Connecting");
  M5.Lcd.println("to WiFi...");

  Serial.begin(115200);
  delay(10);

  // WiFi setup - prefer HeatSync Labs WiFi if available
  Serial.println("\nScanning for WiFi networks...");
  int networksFound = WiFi.scanNetworks();
  bool hslFound = false;

  for (int i = 0; i < networksFound; i++) {
    if (WiFi.SSID(i) == "heatsynclabs") {
      hslFound = true;
      Serial.println("HeatSync Labs WiFi found!");
      break;
    }
  }

  const char* connectSSID;
  const char* connectPassword;

  if (hslFound) {
    connectSSID = "heatsynclabs";
    connectPassword = "hacktheplanet";
    Serial.println("Connecting to HeatSync Labs WiFi...");
  } else {
    connectSSID = ssid;
    connectPassword = password;
    Serial.println("Connecting to configured WiFi...");
  }

  WiFi.begin(connectSSID, connectPassword);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  // If HSL WiFi failed and we tried it, fall back to configured WiFi
  if (WiFi.status() != WL_CONNECTED && hslFound) {
    Serial.println("\nHSL WiFi failed, trying configured WiFi...");
    WiFi.begin(ssid, password);
    attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    printWifiStatus();
  } else {
    Serial.println("\nFailed to connect to WiFi!");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("WiFi Failed!");
    while(true) { delay(1000); } // Halt
  }

  // Show connected status
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("WiFi");
  M5.Lcd.println("Connected!");
  delay(1000);

  // First API call
  makeApiCall();
  lastState = isOpen; // Initialize lastState
  lastApiCall = millis();

  // Update display
  updateStatusDisplay();

  // Initial message
  unsigned long secondsUntilNext = (pollingDelay - (millis() - lastApiCall)) / 1000;
  Serial.printf("Door %s. Checking again in %lu seconds\n", isOpen ? "Open" : "Closed", secondsUntilNext);
}

void loop() {
  M5.update(); // Update button state

  // Periodic API call
  if (millis() - lastApiCall >= pollingDelay) {
    makeApiCall();
    lastApiCall = millis();

    unsigned long secondsUntilNext = pollingDelay / 1000;
    Serial.printf("Door %s. Checking again in %lu seconds\n", isOpen ? "Open" : "Closed", secondsUntilNext);

    // Check if status changed
    if (isOpen != lastState) {
      lastState = isOpen;
      updateStatusDisplay(); // Update display if status changed
    }
  }

  // Button A: Manual refresh
  if (M5.BtnA.wasPressed()) {
    Serial.println("Button pressed - refreshing status");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("Updating...");
    makeApiCall();
    lastApiCall = millis();

    // Check if status changed
    if (isOpen != lastState) {
      lastState = isOpen;
    }
    updateStatusDisplay(); // Full update after manual refresh
  }

  delay(50);
}
