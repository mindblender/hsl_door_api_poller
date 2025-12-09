# HSL Door API Poller

ESP8266-based door status monitor for HeatSync Labs. Polls the space API to check if the door is open or closed, and provides visual feedback via the built-in LED.

## Features

- Polls the HeatSync Labs Space API every 5 minutes
- Visual LED indicator:
  - **Pulsating LED**: Door is open
  - **LED off**: Door is closed
- Serial output with detailed debugging information
- WiFi connectivity status monitoring
- Automatic reconnection handling

## Hardware Requirements

- ESP8266 board (NodeMCU, Wemos D1 Mini, etc.)
- USB cable for programming and power
- WiFi network connection

## Software Requirements

- [Arduino IDE](https://www.arduino.cc/en/software) (version 1.8.x or 2.x)
- ESP8266 board support package
- Required libraries:
  - ESP8266WiFi (included with ESP8266 board package)
  - ESP8266HTTPClient (included with ESP8266 board package)
  - ArduinoJson (install via Library Manager)

## Installation

### 1. Install Arduino IDE

Download and install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software).

### 2. Add ESP8266 Board Support

1. Open Arduino IDE
2. Go to **File > Preferences**
3. In "Additional Boards Manager URLs", add:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. Click **OK**
5. Go to **Tools > Board > Boards Manager**
6. Search for "esp8266"
7. Install **esp8266 by ESP8266 Community**

### 3. Install Required Libraries

1. Go to **Sketch > Include Library > Manage Libraries**
2. Search for "ArduinoJson"
3. Install **ArduinoJson by Benoit Blanchon** (version 6.x recommended)

### 4. Configure WiFi Credentials

1. Copy the example configuration file:
   ```bash
   cp wifi_config_example.h wifi_config.h
   ```
2. Edit `wifi_config.h` with your WiFi credentials:
   ```cpp
   const char* ssid = "YOUR_SSID";
   const char* password = "YOUR_PASSWORD";
   ```
3. Save the file

**Note:** `wifi_config.h` is in `.gitignore` to prevent accidentally committing your WiFi credentials.

### 5. Upload to ESP8266

1. Connect your ESP8266 board via USB
2. In Arduino IDE, go to **Tools > Board** and select your ESP8266 board (e.g., "NodeMCU 1.0")
3. Go to **Tools > Port** and select the COM/serial port for your board
4. Click the **Upload** button (right arrow icon)
5. Wait for the upload to complete

## Usage

### Serial Monitor

1. Open the Serial Monitor: **Tools > Serial Monitor**
2. Set baud rate to **115200**
3. You should see output like:
   ```
   Connecting to WiFi...
   ...
   WiFi connected.
   --- WiFi Status ---
   SSID: YourNetwork
   IP: 192.168.1.100
   RSSI: -45 dBm
   Status: 3
   ------------------
   [API] Starting HTTPS request...
   [API] HTTP GET returned: 200 (OK)
   [API] Response payload:
   {"open":true,...}
   [API] Parsed 'open' value: true
   Door Open. Checking again in 300 seconds
   ```

### LED Indicators

- **Fast pulsating LED**: The door is currently open
- **LED off**: The door is currently closed

### Polling Interval

The default polling interval is **5 minutes (300,000 milliseconds)**. To change this, modify the `pollingDelay` constant in `hsl_door_api_poller.ino`:

```cpp
const unsigned long pollingDelay = 300000; // milliseconds
```

## Troubleshooting

### WiFi Connection Issues

If the board cannot connect to WiFi:
- Verify your SSID and password in `wifi_config.h`
- Ensure your WiFi network is 2.4GHz (ESP8266 doesn't support 5GHz)
- Check that the ESP8266 is within range of your router
- Look for WiFi status debug information in the Serial Monitor

### API Call Failures

If API calls are failing:
- Check the Serial Monitor for HTTP error codes
- Verify internet connectivity
- The API endpoint is: `https://members.heatsynclabs.org/space_api.json`
- Note: The code uses `setInsecure()` which bypasses SSL certificate validation

### Upload Issues

If you can't upload to the board:
- Make sure the correct board and port are selected in **Tools** menu
- Try pressing the FLASH/BOOT button on the board during upload
- Install or update USB drivers for your specific ESP8266 board
- Try a different USB cable (some cables are power-only)

## Configuration

### API Endpoint

To change the API endpoint, modify the `apiEndpoint` constant:

```cpp
const char* apiEndpoint = "https://your-api-endpoint.com/api.json";
```

### LED Pin

By default, the built-in LED is used. To use a different pin:

```cpp
const int ledPin = D1; // or any other GPIO pin
```

## License

This project is open source. Feel free to modify and distribute.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.
