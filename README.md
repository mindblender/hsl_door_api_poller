# HSL Door API Poller

M5StickC-based door status monitor for HeatSync Labs. Polls the space API to check if the door is open or closed, and displays the status on the LCD screen.

## Features

- Polls the HeatSync Labs Space API every 5 minutes
- Large, easy-to-read LCD display:
  - **"OPEN"** in large green text when door is open
  - **"CLOSED"** in red text when door is closed
  - Countdown timer showing seconds until next check
- Manual refresh by pressing the front button (Button A)
- Serial output with detailed debugging information
- WiFi connectivity status monitoring
- Automatic reconnection handling

## Hardware Requirements

- M5StickC (ESP32-based device with built-in LCD)
- USB-C cable for programming and power
- WiFi network connection (2.4GHz)

## Software Requirements

- [Arduino IDE](https://www.arduino.cc/en/software) (version 1.8.x or 2.x)
- ESP32 board support package
- Required libraries:
  - M5StickC (install via Library Manager)
  - WiFi (included with ESP32 board package)
  - HTTPClient (included with ESP32 board package)
  - ArduinoJson (install via Library Manager)

## Installation

### 1. Install Arduino IDE

Download and install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software).

### 2. Add ESP32 Board Support

1. Open Arduino IDE
2. Go to **File > Preferences**
3. In "Additional Boards Manager URLs", add:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Click **OK**
5. Go to **Tools > Board > Boards Manager**
6. Search for "esp32"
7. Install **esp32 by Espressif Systems**

### 3. Install Required Libraries

1. Go to **Sketch > Include Library > Manage Libraries**
2. Search for and install these libraries:
   - **M5StickC by M5Stack** (for M5StickC support)
   - **ArduinoJson by Benoit Blanchon** (version 6.x recommended)

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

### 5. Upload to M5StickC

1. Connect your M5StickC via USB-C cable
2. In Arduino IDE, go to **Tools > Board > ESP32 Arduino** and select **M5Stick-C**
3. Go to **Tools > Port** and select the COM/serial port for your M5StickC
4. Click the **Upload** button (right arrow icon)
5. Wait for the upload to complete
6. The M5StickC display will show "Connecting to WiFi..." during connection

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

### LCD Display

The M5StickC LCD screen shows:
- **"OPEN"** in large green text (size 4 font) when the door is open
- **"CLOSED"** in red text (size 3 font) when the door is closed
- **Countdown timer** at the bottom showing seconds until next API check
- **"Updating..."** in yellow when manually refreshing

### Manual Refresh

Press **Button A** (the large button on the front of the M5StickC) to immediately check the door status without waiting for the next scheduled poll.

### Polling Interval

The default polling interval is **5 minutes (300,000 milliseconds)**. To change this, modify the `pollingDelay` constant in `hsl_door_api_poller.ino`:

```cpp
const unsigned long pollingDelay = 300000; // milliseconds
```

## Troubleshooting

### WiFi Connection Issues

If the M5StickC cannot connect to WiFi:
- Verify your SSID and password in `wifi_config.h`
- Ensure your WiFi network is 2.4GHz (M5StickC doesn't support 5GHz)
- Check that the M5StickC is within range of your router
- Look for WiFi status debug information in the Serial Monitor
- The display will show "Connecting to WiFi..." during connection attempts

### API Call Failures

If API calls are failing:
- Check the Serial Monitor for HTTP error codes
- Verify internet connectivity
- The API endpoint is: `https://members.heatsynclabs.org/space_api.json`
- Note: The code uses `setInsecure()` which bypasses SSL certificate validation

### Upload Issues

If you can't upload to the M5StickC:
- Make sure **M5Stick-C** is selected in **Tools > Board**
- Verify the correct port is selected in **Tools > Port**
- Install or update CH9102 or CP210x USB drivers (depending on your M5StickC version)
- Try a different USB-C cable (some cables are power-only)
- Make sure the M5StickC is powered on (press the power button on the side if needed)

### Display Issues

If the display isn't showing anything:
- Press the power button (side button) to turn on the M5StickC
- Check the battery level (charge via USB-C if needed)
- Verify the upload completed successfully
- Check the Serial Monitor for error messages

## Configuration

### API Endpoint

To change the API endpoint, modify the `apiEndpoint` constant:

```cpp
const char* apiEndpoint = "https://your-api-endpoint.com/api.json";
```

### Display Rotation

The display is set to rotation 3 (landscape mode) for better readability. To change the orientation, modify the rotation value in setup():

```cpp
M5.Lcd.setRotation(3); // 0-3 for different orientations
```

## License

This project is open source. Feel free to modify and distribute.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.
