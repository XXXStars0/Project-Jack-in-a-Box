# 💻 Software Setup & Usage

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
Assemble the components on a breadboard. Ensure the 220Ω current-limiting resistors are correctly placed for the RGB LED. Detailed pin wiring can be found in the [Design & Assembly](design_and_assembly.md) documentation.

### 3. Flash the Firmware
1. Install the **Raspberry Pi Pico/RP2040** board support in the [Arduino IDE](https://www.arduino.cc/en/software/).
2. Open `Jack/Jack.ino`.
3. Select the correct board (**Raspberry Pi Pico W**) and COM port.
4. Upload the sketch to the Pico W.

> The firmware operates in Wi-Fi mode only. Wired (USB/Processing) mode has been deprecated.

### 4. Create API Credentials
Open `Jack/keys.h` (you can copy and rename the provided `Jack/keys_template.h` reference file) and fill in Wi-Fi credentials and Trello API keys.

### 5. Wi-Fi Mode Configuration
The Pico W connects to the internet directly and handles all API requests on-device.

1. **Cornell University (RedRover) Support:** Since the Pico W does not natively support WPA2 Enterprise (eduroam), it is adapted for **RedRover**.
   - Go to [it.cornell.edu/wifi](https://it.cornell.edu/wifi).
   - Select **"Register an IoT Device on RedRover"**.
   - Register the **Device MAC Address** of your Pico W (printed in the Serial Monitor during startup).
   - In `Jack/keys.h`, set the SSID to `"RedRover"` and leave the password empty:
     ```cpp
     #define SECRET_SSID "RedRover"
     #define SECRET_PASS ""
     ```
2. Re-upload the sketch.

## 🧰 Usage Instructions & Setup Flow

1. **Clone the Project**:
   ```bash
   git clone https://github.com/XXXStars0/Project-Jack-in-a-Box
   ```
   Get all design files (SVG), assembly tips, and source code.

2. **Assemble Hardware**: Ensure use of a **half-size breadboard** for the enclosure.

3. **Configure Keys**:
   - Reference `Jack/keys_template.h`.
   - Create a local `Jack/keys.h` and fill in your WiFi SSID, Password, and Trello API credentials.

4. **Flash Firmware**: Open `Jack/Jack.ino` in the Arduino IDE and upload it to the Pico W.

5. **Verify Connection**:
   Open the **Serial Monitor** at **9600 baud**. A successful connection and run will output:
   ```text
   Time synchronized!
   Fetching board Lists...
     [0] To Do
     [1] Doing
     [2] Done
     [3] List 4
     Found 4 lists.

   ========================================
   ---> Tracking list: 'Done'
    There are 0 cards in this list.
   Total pressure: 0
   Pressure Ratio: 0.00
   Target Servo State: IDLE (Blue/Green)
   ```

### 🎮 Operation
- **Switching Lists**: Gently rotate the handle to change tracked lists and trigger a refresh.
- **Sleep Mode**: If the handle is not rotated for **60 seconds**, the device will automatically enter a power-saving state (retracting the Jack and turning off LEDs). Gently turning the handle will wake it up immediately.
- **Vibration (Shake)**: Triggered during **Medium Pressure** (Yellow status).
- **Pop-out**: The lid will pop open when pressure reaches the **Red** threshold.
- **Reset**: The Jack will automatically retract and the lid will close when pressure returns to **Idle/Low**.

### Visual Status Indicators (LED)
The Pico W uses dual-core architecture for non-blocking LED animations (Core 1) during API requests (Core 0):
- ⚪ **BOOTING / LOADING:** Breathing effect. Startup or fetching new data.
- 🔴 **ERROR:** Rapid red double-blink. Wi-Fi disconnection or API failure.
- ⚫ **SLEEP:** LEDs off. Sleep mode is active after 60 seconds of inactivity.
- **TRACKING:** Solid color based on accumulated "Pressure Score" (card due dates):
  - 🔵 **Blue:** Idle / Empty list.
  - 🟢 **Green:** Low pressure. (Note: The `Done` list and tasks marked with `dueComplete` are explicitly overridden to **0 Pressure/Green**).
  - 🟡 **Yellow:** Medium pressure.
  - 🔴 **Red:** High pressure (urgent/overdue).

## API Testing (Optional)
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
