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

// Draw a tall, narrow pixel-art Christmas tree (70 pixels tall, 30 pixels wide)
void drawTree(int x, int y) {
  // Star on top (yellow) - 3px tall
  M5.Lcd.fillRect(x+14, y, 3, 2, YELLOW);
  M5.Lcd.fillRect(x+13, y+2, 5, 2, YELLOW);

  // Top section of tree (green) - 16px tall
  M5.Lcd.fillRect(x+12, y+5, 7, 2, GREEN);
  M5.Lcd.fillRect(x+11, y+7, 9, 2, GREEN);
  M5.Lcd.fillRect(x+10, y+9, 11, 2, GREEN);
  M5.Lcd.fillRect(x+9, y+11, 13, 2, GREEN);
  M5.Lcd.fillRect(x+8, y+13, 15, 2, GREEN);
  M5.Lcd.fillRect(x+7, y+15, 17, 2, GREEN);
  M5.Lcd.fillRect(x+6, y+17, 19, 2, GREEN);
  M5.Lcd.fillRect(x+5, y+19, 21, 2, GREEN);

  // Middle section (green) - 24px tall
  M5.Lcd.fillRect(x+8, y+23, 15, 2, GREEN);
  M5.Lcd.fillRect(x+7, y+25, 17, 2, GREEN);
  M5.Lcd.fillRect(x+6, y+27, 19, 2, GREEN);
  M5.Lcd.fillRect(x+5, y+29, 21, 2, GREEN);
  M5.Lcd.fillRect(x+4, y+31, 23, 2, GREEN);
  M5.Lcd.fillRect(x+3, y+33, 25, 2, GREEN);
  M5.Lcd.fillRect(x+2, y+35, 27, 2, GREEN);
  M5.Lcd.fillRect(x+2, y+37, 27, 2, GREEN);
  M5.Lcd.fillRect(x+3, y+39, 25, 2, GREEN);
  M5.Lcd.fillRect(x+4, y+41, 23, 2, GREEN);
  M5.Lcd.fillRect(x+5, y+43, 21, 2, GREEN);
  M5.Lcd.fillRect(x+6, y+45, 19, 2, GREEN);

  // Bottom section (green) - 20px tall
  M5.Lcd.fillRect(x+5, y+48, 21, 2, GREEN);
  M5.Lcd.fillRect(x+4, y+50, 23, 2, GREEN);
  M5.Lcd.fillRect(x+3, y+52, 25, 2, GREEN);
  M5.Lcd.fillRect(x+2, y+54, 27, 2, GREEN);
  M5.Lcd.fillRect(x+1, y+56, 29, 2, GREEN);
  M5.Lcd.fillRect(x, y+58, 30, 2, GREEN);
  M5.Lcd.fillRect(x, y+60, 30, 2, GREEN);
  M5.Lcd.fillRect(x+1, y+62, 29, 2, GREEN);
  M5.Lcd.fillRect(x+2, y+64, 27, 2, GREEN);
  M5.Lcd.fillRect(x+3, y+66, 25, 2, GREEN);

  // Trunk (brown) - 8px tall
  M5.Lcd.fillRect(x+11, y+68, 9, 8, 0x8400);

  // Ornaments (colorful decorations)
  M5.Lcd.fillRect(x+10, y+11, 2, 2, RED);     // Red - top
  M5.Lcd.fillRect(x+17, y+15, 2, 2, BLUE);    // Blue - top

  M5.Lcd.fillRect(x+7, y+27, 2, 2, MAGENTA);  // Magenta - middle
  M5.Lcd.fillRect(x+19, y+31, 2, 2, CYAN);    // Cyan - middle
  M5.Lcd.fillRect(x+5, y+37, 2, 2, RED);      // Red - middle
  M5.Lcd.fillRect(x+22, y+41, 2, 2, YELLOW);  // Yellow - middle

  M5.Lcd.fillRect(x+6, y+52, 2, 2, BLUE);     // Blue - bottom
  M5.Lcd.fillRect(x+13, y+56, 2, 2, MAGENTA); // Magenta - bottom
  M5.Lcd.fillRect(x+20, y+60, 2, 2, CYAN);    // Cyan - bottom
  M5.Lcd.fillRect(x+10, y+64, 2, 2, YELLOW);  // Yellow - bottom
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

    // Draw tall, narrow Christmas tree on the right side
    drawTree(127, 3);
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

  // WiFi setup
  Serial.println("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  printWifiStatus();

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
