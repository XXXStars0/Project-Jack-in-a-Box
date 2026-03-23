//------------------------
// Assignment: A Jack in a Box
//------------------------

#include "keys.h"
#include <Servo.h>

// RGB LED Type: Uncomment if using Common Anode RGB LED
// #define RGB_COMMON_ANODE

// Pin Setup - Output
const int PIN_LED_R = 13;
const int PIN_LED_G = 14;
const int PIN_LED_B = 15;
const int PIN_SERVO = 16;

// Pin Setup - Input
const int PIN_POT = 27;
// const int PIN_LDR = 27;  // TODO: Future sensor input
// const int PIN_IR = 28;   // TODO: Future sensor input

// --- Servo Config ---
const int SERVO_POS_CLOSED = 0;
const int SERVO_POS_OPEN = 180;
const float PRESSURE_SHAKE_THRESHOLD = 0.5;
const float PRESSURE_POP_THRESHOLD = 0.8;

// --- Interaction Config ---
const String DONE_LIST_NAME = "Done"; // List name that unconditionally reports 0 pressure (Green light)
const unsigned long SLEEP_TIMEOUT = 60000; // Inactivity timeout in ms (60 seconds)

// --- State Machine ---
enum SystemState {
  STATE_BOOTING = 0,
  STATE_LOADING = 1,
  STATE_ERROR = 2,
  STATE_TRACKING = 3,
  STATE_SLEEP = 4
};

enum ServoState {
  SERVO_IDLE = 0,
  SERVO_SHAKING = 1,
  SERVO_OPEN = 2
};

volatile SystemState currentState = STATE_BOOTING;
volatile int targetR = 0;
volatile int targetG = 0;
volatile int targetB = 0;
volatile float pressureRatio = 0.0;

Servo boxServo;
ServoState servoState = SERVO_IDLE;
unsigned long lastShakeTime = 0;
bool shakeDirection = false;
int shakeCount = 0;

// WiFi and Trello globals
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>

const float MAX_PRESSURE_THRESHOLD = 50.0;
const int MAX_LISTS = 10;
String listIDs[MAX_LISTS];
String listNames[MAX_LISTS];
int listCount = 0;

// ---------------------------------------------------------
// Core 1 (LED Animation Engine) - Runs on the second thread
// ---------------------------------------------------------
void setup1() {
  // Give Core 0 time to init pins
  delay(100);
}

void loop1() {
  unsigned long ms = millis();

  switch (currentState) {
  case STATE_BOOTING:
  case STATE_LOADING:
    // White breathing for both booting and loading new data
    {
      float fade = (sin(ms / 300.0) + 1.0) / 2.0; // 0.0 to 1.0
      int val = (int)(255 * fade);
#ifdef RGB_COMMON_ANODE
      ledWrite(PIN_LED_R, 255 - val);
      ledWrite(PIN_LED_G, 255 - val);
      ledWrite(PIN_LED_B, 255 - val);
#else
      analogWrite(PIN_LED_R, val);
      analogWrite(PIN_LED_G, val);
      analogWrite(PIN_LED_B, val);
#endif
    }
    break;

  case STATE_ERROR:
    // Red rapid double-blink
    {
      int cycle = ms % 1000;
      if ((cycle > 0 && cycle < 100) || (cycle > 200 && cycle < 300)) {
#ifdef RGB_COMMON_ANODE
        ledWrite(PIN_LED_R, 255);
        ledWrite(PIN_LED_G, 0);
        ledWrite(PIN_LED_B, 0);
#else
        analogWrite(PIN_LED_R, 255);
        analogWrite(PIN_LED_G, 0);
        analogWrite(PIN_LED_B, 0);
#endif
      } else {
#ifdef RGB_COMMON_ANODE
        ledOff(PIN_LED_R);
        ledOff(PIN_LED_G);
        ledOff(PIN_LED_B);
#else
        analogWrite(PIN_LED_R, 0);
        analogWrite(PIN_LED_G, 0);
        analogWrite(PIN_LED_B, 0);
#endif
      }
    }
    break;

  case STATE_TRACKING:
#ifdef RGB_COMMON_ANODE
    ledWrite(PIN_LED_R, targetR);
    ledWrite(PIN_LED_G, targetG);
    ledWrite(PIN_LED_B, targetB);
#else
    analogWrite(PIN_LED_R, targetR);
    analogWrite(PIN_LED_G, targetG);
    analogWrite(PIN_LED_B, targetB);
#endif
    break;

  case STATE_SLEEP:
#ifdef RGB_COMMON_ANODE
    ledOff(PIN_LED_R);
    ledOff(PIN_LED_G);
    ledOff(PIN_LED_B);
#else
    digitalWrite(PIN_LED_R, LOW);
    digitalWrite(PIN_LED_G, LOW);
    digitalWrite(PIN_LED_B, LOW);
#endif
    break;
  }
  delay(10);
}

// ---------------------------------------------------------
// Core 0 (Logic Engine) - Main Thread
// ---------------------------------------------------------

void setup() {
  Serial.begin(9600);
  
  // GP13/14/15 are shared with SPI1 by default on Pico W
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_POT, INPUT);
  analogReadResolution(12);

  // Servo init
  boxServo.attach(PIN_SERVO);
  boxServo.write(SERVO_POS_CLOSED);

  currentState = STATE_BOOTING;

  // Wait to allow Serial Monitor to open so the user can see connection status
  delay(2000); 

  // Connect to network, sync time, pre-fetch lists
  Serial.print("Connecting to WiFi (SSID: ");
  Serial.print(SECRET_SSID);
  Serial.println(")");
  
  if (String(SECRET_PASS).length() > 0) {
    WiFi.begin(SECRET_SSID, SECRET_PASS); // WPA2 Personal
  } else {
    WiFi.begin(SECRET_SSID); // Open Network (e.g., RedRover)
  }
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  Serial.print("Syncing Time");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nTime synchronized!");

  currentState = STATE_LOADING;
  fetchTrelloLists();

  if (listCount == 0) {
    currentState = STATE_ERROR;
  } else {
    currentState =
        STATE_TRACKING; // Requires initial loop logic to calculate color
  }
}

void loop() {
  // WiFi connection check
  if (WiFi.status() != WL_CONNECTED) {
    if (currentState != STATE_ERROR) {
      Serial.println("WiFi disconnected!");
      currentState = STATE_ERROR;
    }
    delay(1000);
    return;
  }
  if (listCount == 0) {
    if (currentState != STATE_ERROR) {
      Serial.println("No lists found or API failed.");
      currentState = STATE_ERROR;
    }
    delay(1000);
    return;
  }

  static int lastSelectedIndex = -1;
  static unsigned long lastApiRequestTime = 0;
  static unsigned long lastPotInteractionTime = millis();
  const unsigned long apiRequestInterval = 30000; // 30s between auto-refreshes

  if (currentState == STATE_SLEEP) {
    boxServo.write(SERVO_POS_CLOSED);
    // Wake on pot movement
    int potNow = analogRead(PIN_POT);
    int idxNow = map(potNow, 0, 4096, 0, listCount);
    idxNow = constrain(idxNow, 0, listCount - 1);
    if (idxNow != lastSelectedIndex) {
      currentState = STATE_BOOTING;
      Serial.println("POT: Wake");
      lastPotInteractionTime = millis();
    } else {
      delay(50);
      return;
    }
  }

  // --- Edge Detection ---
  int potValue = analogRead(PIN_POT);
  int currentSelectedIndex = map(potValue, 0, 4096, 0, listCount);
  currentSelectedIndex = constrain(currentSelectedIndex, 0, listCount - 1);

  bool timeToUpdate = (millis() - lastApiRequestTime >= apiRequestInterval);
  bool listChanged = (currentSelectedIndex != lastSelectedIndex);

  if (listChanged) {
    lastPotInteractionTime = millis();
  } else if (millis() - lastPotInteractionTime >= SLEEP_TIMEOUT && currentState != STATE_SLEEP) {
    Serial.println("Inactivity timeout: Going to Sleep.");
    currentState = STATE_SLEEP;
    return;
  }

  if (listChanged || timeToUpdate) {
    currentState = STATE_LOADING;
    // TODO: Future - pause servo during loading if needed

    Serial.println("\n========================================");
    Serial.print("---> Tracking list: '");
    Serial.print(listNames[currentSelectedIndex]);
    Serial.println("'");

    bool isDoneList = listNames[currentSelectedIndex].equalsIgnoreCase(DONE_LIST_NAME);
    int totalPressure = 0;

    if (isDoneList) {
      Serial.println("  -> List is marked as 'Done'. Pressure overridden to 0.");
    } else {
      totalPressure = calculateListPressure(listIDs[currentSelectedIndex]);
      Serial.print("Total pressure: ");
      Serial.println(totalPressure);
    }

    // Color mapping: Blue(idle) → Green(low) → Yellow(mid) → Red(high)
    int r, g, b;
    float ratio = 0.0;
    
    if (isDoneList) {
      // Direct green for Done lists
      r = 0;
      g = 255;
      b = 0;
    } else if (totalPressure == 0) {
      r = 0;
      g = 0;
      b = 255;
    } else {
      ratio = (float)totalPressure / MAX_PRESSURE_THRESHOLD;
      if (ratio > 1.0)
        ratio = 1.0;
      if (ratio <= 0.5) {
        float t = ratio / 0.5;
        r = (int)(255 * t);
        g = 255;
      } else {
        float t = (ratio - 0.5) / 0.5;
        r = 255;
        g = (int)(255 * (1.0 - t));
      }
      b = 0;
    }

    targetR = r;
    targetG = g;
    targetB = b;
    pressureRatio = ratio;

    Serial.print("Pressure Ratio: ");
    Serial.println(ratio);
    Serial.print("Target Servo State: ");
    if (ratio >= PRESSURE_POP_THRESHOLD) {
      Serial.println("OPEN (Red)");
    } else if (ratio >= PRESSURE_SHAKE_THRESHOLD) {
      Serial.println("SHAKING (Yellow)");
    } else {
      Serial.println("IDLE (Blue/Green)");
    }

    lastSelectedIndex = currentSelectedIndex;
    lastApiRequestTime = millis();

    delay(200);
    currentState = STATE_TRACKING;
  }

  // --- Servo Update ---
  // TODO: Future - add sensor-based servo override here
  updateServo();

  delay(50);
}

// ---------------------------------------------------------
// Servo Control
// ---------------------------------------------------------
void updateServo() {
  float r = pressureRatio;

  switch (servoState) {
  case SERVO_IDLE:
    boxServo.write(SERVO_POS_CLOSED);
    if (r >= PRESSURE_POP_THRESHOLD) {
      servoState = SERVO_OPEN;
    } else if (r >= PRESSURE_SHAKE_THRESHOLD) {
      servoState = SERVO_SHAKING;
      lastShakeTime = millis();
      shakeCount = 0;
    }
    break;

  case SERVO_SHAKING:
    if (r >= PRESSURE_POP_THRESHOLD) {
      servoState = SERVO_OPEN;
    } else if (r < PRESSURE_SHAKE_THRESHOLD) {
      servoState = SERVO_IDLE;
    } else {
      // Burst shake: shake quickly 6 times, then pause 2 seconds
      unsigned long interval = (shakeCount < 6) ? 150 : 2000;
      if (millis() - lastShakeTime >= interval) {
        if (shakeCount < 6) {
          // Larger amplitude shake: 0 or 30 degrees
          int pos = shakeDirection ? SERVO_POS_CLOSED + 30 : SERVO_POS_CLOSED;
          boxServo.write(pos);
          shakeDirection = !shakeDirection;
          shakeCount++;
        } else {
          // Pause finished, reset burst
          shakeCount = 0;
        }
        lastShakeTime = millis();
      }
    }
    break;

  case SERVO_OPEN:
    boxServo.write(SERVO_POS_OPEN);
    if (r < PRESSURE_SHAKE_THRESHOLD) {
      servoState = SERVO_IDLE;
    }
    break;
  }
}

// ---------------------------------------------------------
// WiFi Helper Functions
// ---------------------------------------------------------

void fetchTrelloLists() {
  WiFiClientSecure client;
  client.setInsecure(); // Skip SSL certificate verification on Pico W
  
  HTTPClient http;
  String url = "https://api.trello.com/1/boards/" + String(TRELLO_BOARD_ID) +
               "/lists?key=" + String(TRELLO_API_KEY) +
               "&token=" + String(TRELLO_TOKEN);

  Serial.println("Fetching board Lists...");
  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();

    // Use ArduinoJson v7 to parse
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      JsonArray array = doc.as<JsonArray>();
      listCount = 0;
      for (JsonObject v : array) {
        if (listCount >= MAX_LISTS)
          break;
        listIDs[listCount] = v["id"].as<String>();
        listNames[listCount] = v["name"].as<String>();
        Serial.print("  [");
        Serial.print(listCount);
        Serial.print("] ");
        Serial.println(listNames[listCount]);
        listCount++;
      }
      Serial.print("  Found ");
      Serial.print(listCount);
      Serial.println(" lists.");
    } else {
      Serial.println("Error: JSON Parsing failed!");
    }
  } else {
    Serial.print("Error: Request failed, code: ");
    Serial.println(httpCode);
  }
  http.end();
}

int calculateListPressure(String listId) {
  WiFiClientSecure client;
  client.setInsecure();
  
  HTTPClient http;
  String url = "https://api.trello.com/1/lists/" + listId +
               "/cards?key=" + String(TRELLO_API_KEY) +
               "&token=" + String(TRELLO_TOKEN);

  http.begin(client, url);
  int httpCode = http.GET();
  int pressure = 0;

  if (httpCode == 200) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, http.getStream());

    if (!error) {
      JsonArray cards = doc.as<JsonArray>();
      Serial.print(" There are ");
      Serial.print(cards.size());
      Serial.println(" cards in this list.");

      time_t currentTime = time(nullptr);

      for (JsonObject card : cards) {
        if (!card["dueComplete"].isNull() && card["dueComplete"].as<bool>()) {
          // Task completes logic: ignore due date calculations completely
          continue; 
        }

        if (!card["due"].isNull()) {
          String dueStr = card["due"].as<String>();
          time_t dueDate = parseISO8601(dueStr.c_str());

          double hoursLeft = difftime(dueDate, currentTime) / 3600.0;

          if (hoursLeft < 0) {
            pressure += 20;
          } else if (hoursLeft <= 24.0) {
            pressure += 10;
          } else if (hoursLeft <= 24.0 * 7.0) {
            pressure += 5;
          } else {
            pressure += 2;
          }
        } else {
          pressure += 1;
        }
      }
    } else {
      Serial.println("Error: JSON Parsing failed for cards!");
    }
  } else {
    Serial.println("Error: Failed to fetch cards.");
  }

  http.end();
  return pressure;
}

// Simple ISO8601 date string parser to UNIX time
time_t parseISO8601(const char *dateStr) {
  struct tm t = {0};
  int y, M, d, h, m;
  float s;

  // Format: 2024-05-20T10:00:00.000Z
  sscanf(dateStr, "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);

  t.tm_year = y - 1900;
  t.tm_mon = M - 1;
  t.tm_mday = d;
  t.tm_hour = h;
  t.tm_min = m;
  t.tm_sec = (int)s;

  // Default timezone mktime conversion
  return mktime(&t);
}
