# 📦 Project jack in a box

![INFO](https://img.shields.io/badge/INFO-5321-B31B1B)
![Trello](https://img.shields.io/badge/Trello-%23026AA7.svg?style=flat&logo=trello&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/-Raspberry_Pi-C51A4A?logo=Raspberry-Pi&logoColor=white)
![Arduino IDE](https://img.shields.io/badge/-Arduino_IDE-00979D?logo=Arduino&logoColor=white)
![C++](https://img.shields.io/badge/-C++-00599C?logo=c%2B%2B&logoColor=white)

**Project jack in a box**: A Jack in a Box toy / desk setup based on the [Ambient Trello Aura](https://github.com/XXXStars0/Project-LED-Light). Now supports **RedRover** (Cornell IoT) for standalone connectivity. Currently under development.

## 📁 Project Structure

- `img/`: Circuit diagrams and design images (Legacy).
- `design/`: Design diagrams and documentation for the current project.
- `Jack/`: Arduino/C++ source code for the Pico W.
- `Processing_Connect/`: Legacy wired internet connection mode using Processing (Alt/Backup).
- `tests/`: API verification and testing scripts (Python).

## ⚙️ Hardware Components

- Raspberry Pi Pico W
- RGB LED (Common Cathode)
- Potentiometer 
- MG 996R (GP16 control, VBUS power)
- 100µF Capacitor (between VBUS and GND)
- 3× 220Ω Resistors (for RGB LED current limiting)
- Breadboard and jumper wires

## 🎨 Design
New design is still under development.

## 💡 Pin Mapping Reference

| Component           | Pico W Pin                   | Note         |
| ------------------- | ---------------------------- | ------------ |
| **RGB LED**         | GP13 (R), GP14 (G), GP15 (B) | PWM Support  |
| **Potentiometer**   | GP27                         | Analog Input |
| **Servo (MG 996R)** | GP16                         | PWM Control  |

## 📡 API Integration

This project uses the **[Trello API](https://developer.atlassian.com/cloud/trello/guides/rest-api/api-introduction/)** to monitor project changes and provide visual feedback via the RGB LED.

## 💻 Development Environment

- **Primary IDE**: Arduino IDE (for Pico W Sketch)
- **Secondary IDE**: VS Code (for documentation and API testing)
- **Platform**: Raspberry Pi Pico W 

## ⭐ Getting Started

### 1. Clone the Repository
```bash
git clone https://github.com/XXXStars0/Project-Jack-in-a-Box.git
cd Project-Jack-in-a-Box
```

### 2. Hardware Assembly
Assemble the components on a breadboard according to the **[Pin Mapping & Circuit Diagram](#-pin-mapping-reference)** above. Ensure the 220Ω current-limiting resistors are correctly placed for the RGB LED.

### 3. Flash the Firmware
1. Install the **Raspberry Pi Pico/RP2040** board support in the [Arduino IDE](https://www.arduino.cc/en/software/).
2. Open `Jack/Jack.ino`.
3. Select the correct board (**Raspberry Pi Pico W**) and COM port.
4. Upload the sketch to the Pico W.

> The firmware supports both Wi-Fi and Wired modes. The mode is determined by a single config flag — see the sections below.

### 4. Create API Credentials (`.env`)
Create a `.env` file in the **project root directory** with Trello API credentials. This file is required for both Wi-Fi and Wired modes:
```env
TRELLO_API_KEY=your_api_key_here
TRELLO_TOKEN=your_oauth_token_here
TRELLO_BOARD_ID=your_board_id_here
```

### 5a. Wi-Fi Mode Configuration
In this mode the Pico W connects to the internet directly and handles all API requests on-device.

1. Open `Jack/wifi_config.h` and **uncomment** the line:
   ```cpp
   #define USE_WIFI_MODE
   ```
2. Open `Jack/keys.h` (you can copy and rename the provided `Jack/keys_template.h` reference file) and fill in Wi-Fi credentials and Trello API keys.
3. **Cornell University (RedRover) Support:** Since the Pico W does not natively support WPA2 Enterprise (eduroam), it is adapted for **RedRover**.
   - Go to [it.cornell.edu/wifi](https://it.cornell.edu/wifi).
   - Select **"Register an IoT Device on RedRover"**.
   - Register the **Device MAC Address** of your Pico W (printed in the Serial Monitor during startup).
   - In `Jack/keys.h`, set the SSID to `"RedRover"` and leave the password empty:
     ```cpp
     #define SECRET_SSID "RedRover"
     #define SECRET_PASS ""
     ```
4. Re-upload the sketch.

### 5b. Wired Mode Configuration (Alternative / Legacy)
An alternative mode where a computer handles API requests and communicates with the Pico W via USB Serial. **Note:** This is currently treated as a fallback option and is not under active development.

1. Ensure `Jack/wifi_config.h` has the Wi-Fi flag **commented out** (default):
   ```cpp
   // #define USE_WIFI_MODE
   ```
2. Keep the Pico W connected to the computer via USB.
3. Open `Processing_Connect/Processing_Connect.pde` in the [Processing IDE](https://processing.org/).
4. Update the `COM_PORT` variable to match the Pico W's serial port (e.g., `"COM4"`).
5. Run the Processing sketch. It will automatically read credentials from the root `.env` file, receive potentiometer data from the Pico W, and send back real-time RGB color values.

### API Testing (Optional)
<details>
<summary>Click to expand Python API testing instructions</summary>

To verify Trello API connectivity independently using the provided Python script in `tests/`:

1. Install dependencies:
   ```bash
   pip install python-dotenv requests
   ```
2. Run the test script:
   ```bash
   python tests/trello_api_test.py
   ```

**💡 Note:** In line 29 of `trello_api_test.py` (`selected_list = trello_lists[0]`), the tracked list can be changed by modifying the index.

**Example Output:**
```text
---> Simulating potentiometer input: Currently tracking list 'To Do'

There are 1 cards in this list. Starting pressure value calculation...

   Card: Test 1
    Status: No due date -> Pressure +1

========================================
Total pressure score for the current list: 1
Pico W Pin PWM Output Instructions:
   -> GP13 (Red):   5
   -> GP14 (Green): 249
   -> GP15 (Blue):  0
```
</details>

## 🧰 Usage Instructions

### Controls
- **Potentiometer (Knob):** Rotate to scroll through Trello lists. The system detects index changes (Edge Detection) and instantly triggers a data fetch. 
- **Wake:** Rotating the potentiometer will immediately awaken the device and resume tracking.

### Visual Status Indicators (LED)
The Pico W uses dual-core architecture for non-blocking LED animations (Core 1) during API requests (Core 0):
- ⚪ **BOOTING / LOADING:** White breathing effect. Startup or fetching new data.
- 🔴 **ERROR:** Rapid red double-blink. Wi-Fi disconnection or API failure.
- ⚫ **SLEEP:** LEDs off. Power-saving mode.
- **TRACKING:** Solid color based on accumulated "Pressure Score" (card due dates):
  - 🔵 **Blue:** Idle / Empty list.
  - 🟢 **Green:** Low pressure.
  - 🟡 **Yellow:** Medium pressure.
  - 🔴 **Red:** High pressure (urgent/overdue).

## 🛠️ Troubleshooting & Fixes

### Eduroam Connection Failure
- **Issue:** Attempted to support `eduroam` (WPA2 Enterprise) directly, but the Pico W's `WiFi.h` library (arduino-pico) does not natively support the EAP authentication required for university networks.
- **Fix:** Switched to the **RedRover** IoT registration method. By registering the device's MAC address with Cornell IT, the Pico W can connect to the internet without additional enterprise headers. WiFi connection is now stable.
- **Note:** Ensure `Jack/keys.h` is correctly updated with `SECRET_SSID "RedRover"` and an empty `SECRET_PASS ""` to avoid authentication errors.

