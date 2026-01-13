/*
 * CYD (ESP32-2432S028R) Rack Controller Firmware
 * 
 * Implements the standard boot contract:
 * - Serial JSON output within 10 seconds
 * - MQTT status publication (retained)
 * - Display status on TFT screen
 * 
 * Device ID is derived from MAC address.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "secrets.h"

// Build-time constants (injected by platformio.ini)
#ifndef FW_VERSION
#define FW_VERSION "unknown"
#endif

#ifndef TARGET_NAME
#define TARGET_NAME "cyd"
#endif

// MQTT topic prefix
#define MQTT_TOPIC_PREFIX "lab/"
#define MQTT_TOPIC_SUFFIX "/status"

// Timing
#define WIFI_TIMEOUT_MS 15000
#define MQTT_TIMEOUT_MS 5000
#define BOOT_CONTRACT_DEADLINE_MS 10000

// CYD Hardware pins
#define TFT_BACKLIGHT 21
#define RGB_LED_R 4
#define RGB_LED_G 16
#define RGB_LED_B 17
#define LDR_PIN 34

// Global objects
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
TFT_eSPI tft = TFT_eSPI();

// Device ID (derived from MAC)
String deviceId;

// Boot status
bool selftestPassed = true;
String errorCode = "";

/**
 * Generate device ID from MAC address
 * Format: "cyd-XXXXXXXXXXXX" (lowercase hex, no colons)
 */
String getDeviceId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char id[20];
    snprintf(id, sizeof(id), "cyd-%02x%02x%02x%02x%02x%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(id);
}

/**
 * Set RGB LED color (active LOW)
 */
void setRGBLed(bool r, bool g, bool b) {
    digitalWrite(RGB_LED_R, !r);
    digitalWrite(RGB_LED_G, !g);
    digitalWrite(RGB_LED_B, !b);
}

/**
 * Initialize display
 */
void initDisplay() {
    // Enable backlight
    pinMode(TFT_BACKLIGHT, OUTPUT);
    digitalWrite(TFT_BACKLIGHT, HIGH);
    
    tft.init();
    tft.setRotation(1);  // Landscape
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
}

/**
 * Display boot status on screen
 */
void displayBootStatus(const char* status, uint16_t color) {
    tft.fillScreen(TFT_BLACK);
    
    // Header
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Rack Controller");
    
    // Device ID
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 40);
    tft.print("Device: ");
    tft.println(deviceId);
    
    // Firmware version
    tft.setCursor(10, 55);
    tft.print("FW: ");
    tft.println(FW_VERSION);
    
    // Status
    tft.setTextSize(2);
    tft.setTextColor(color, TFT_BLACK);
    tft.setCursor(10, 80);
    tft.println(status);
}

/**
 * Display network info
 */
void displayNetworkInfo() {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 120);
    tft.print("IP: ");
    tft.println(WiFi.localIP());
    
    tft.setCursor(10, 135);
    tft.print("RSSI: ");
    tft.print(WiFi.RSSI());
    tft.println(" dBm");
}

/**
 * Run self-tests
 */
bool runSelfTest() {
    // Test display (if we got here, it's working)
    bool displayOk = true;
    
    // Test LDR (check it reads something)
    int ldrValue = analogRead(LDR_PIN);
    bool ldrOk = (ldrValue > 0 && ldrValue < 4095);
    
    // TODO: Add more hardware tests
    // - Touch controller
    // - SD card (if used)
    
    return displayOk && ldrOk;
}

/**
 * Connect to WiFi with timeout
 */
bool connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    displayBootStatus("Connecting WiFi...", TFT_YELLOW);
    setRGBLed(true, true, false);  // Yellow
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_TIMEOUT_MS) {
            return false;
        }
        delay(100);
    }
    return true;
}

/**
 * Connect to MQTT broker with timeout
 */
bool connectMQTT() {
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    
    unsigned long startTime = millis();
    while (!mqttClient.connected()) {
        if (millis() - startTime > MQTT_TIMEOUT_MS) {
            return false;
        }
        
        String clientId = deviceId + "-" + String(random(0xffff), HEX);
        
        bool connected;
        if (strlen(MQTT_USER) > 0) {
            connected = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
        } else {
            connected = mqttClient.connect(clientId.c_str());
        }
        
        if (connected) {
            return true;
        }
        delay(100);
    }
    return true;
}

/**
 * Build boot contract JSON
 */
String buildBootJson(bool includeTimestamp = false) {
    JsonDocument doc;
    
    doc["device"] = deviceId;
    doc["fw"] = FW_VERSION;
    doc["target"] = TARGET_NAME;
    doc["selftest"] = selftestPassed ? "pass" : "fail";
    
    if (!selftestPassed && errorCode.length() > 0) {
        doc["err"] = errorCode;
    }
    
    if (includeTimestamp) {
        doc["ts"] = millis() / 1000;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * Print boot contract to serial
 */
void printBootContract() {
    String json = buildBootJson(false);
    Serial.println(json);
}

/**
 * Publish boot status to MQTT
 */
void publishMQTTStatus() {
    String topic = String(MQTT_TOPIC_PREFIX) + deviceId + String(MQTT_TOPIC_SUFFIX);
    String payload = buildBootJson(true);
    
    mqttClient.publish(topic.c_str(), payload.c_str(), true);
}

void setup() {
    unsigned long bootStart = millis();
    
    // Initialize serial
    Serial.begin(115200);
    delay(100);
    
    // Initialize RGB LED
    pinMode(RGB_LED_R, OUTPUT);
    pinMode(RGB_LED_G, OUTPUT);
    pinMode(RGB_LED_B, OUTPUT);
    setRGBLed(false, false, true);  // Blue during init
    
    // Initialize display
    initDisplay();
    displayBootStatus("Initializing...", TFT_WHITE);
    
    // Get device ID from MAC
    deviceId = getDeviceId();
    
    // Run self-tests
    selftestPassed = runSelfTest();
    if (!selftestPassed) {
        errorCode = "SELFTEST_FAIL";
        displayBootStatus("SELFTEST FAIL", TFT_RED);
        setRGBLed(true, false, false);  // Red
    }
    
    // Print boot contract to serial (must be within 10 seconds)
    printBootContract();
    
    // Connect to WiFi
    if (!connectWiFi()) {
        Serial.println("WiFi connection failed");
        displayBootStatus("WiFi FAILED", TFT_RED);
        setRGBLed(true, false, false);  // Red
    } else {
        Serial.print("WiFi connected, IP: ");
        Serial.println(WiFi.localIP());
        
        // Connect to MQTT and publish status
        if (connectMQTT()) {
            publishMQTTStatus();
            Serial.println("MQTT status published");
            displayBootStatus("ONLINE", TFT_GREEN);
            displayNetworkInfo();
            setRGBLed(false, true, false);  // Green
        } else {
            Serial.println("MQTT connection failed");
            displayBootStatus("MQTT FAILED", TFT_ORANGE);
            setRGBLed(true, true, false);  // Yellow
        }
    }
    
    unsigned long bootTime = millis() - bootStart;
    Serial.printf("Boot completed in %lu ms\n", bootTime);
}

void loop() {
    // Maintain MQTT connection
    if (mqttClient.connected()) {
        mqttClient.loop();
    } else if (WiFi.status() == WL_CONNECTED) {
        connectMQTT();
    }
    
    // TODO: Add main application logic
    // - Touch handling
    // - Display updates
    // - Sensor readings
    // - Command handling via MQTT
    
    delay(100);
}
