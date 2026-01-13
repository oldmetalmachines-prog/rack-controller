/*
 * ESP32-S3 Rack Controller Firmware
 * 
 * Implements the standard boot contract:
 * - Serial JSON output within 10 seconds
 * - MQTT status publication (retained)
 * 
 * Device ID is derived from MAC address.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Build-time constants (injected by platformio.ini)
#ifndef FW_VERSION
#define FW_VERSION "unknown"
#endif

#ifndef TARGET_NAME
#define TARGET_NAME "s3"
#endif

// MQTT topic prefix
#define MQTT_TOPIC_PREFIX "lab/"
#define MQTT_TOPIC_SUFFIX "/status"

// Timing
#define WIFI_TIMEOUT_MS 15000
#define MQTT_TIMEOUT_MS 5000
#define BOOT_CONTRACT_DEADLINE_MS 10000

// Global objects
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Device ID (derived from MAC)
String deviceId;

// Boot status
bool selftestPassed = true;
String errorCode = "";

/**
 * Generate device ID from MAC address
 * Format: "esp32-XXXXXXXXXXXX" (lowercase hex, no colons)
 */
String getDeviceId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char id[20];
    snprintf(id, sizeof(id), "esp32-%02x%02x%02x%02x%02x%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(id);
}

/**
 * Run self-tests
 * Add hardware-specific tests here
 */
bool runSelfTest() {
    // TODO: Add actual hardware tests
    // Example tests:
    // - Check GPIO states
    // - Verify sensor connectivity
    // - Test peripheral communication
    
    // For now, always pass
    return true;
}

/**
 * Connect to WiFi with timeout
 */
bool connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
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
        // Use millis as pseudo-timestamp if no NTP
        // In production, sync with NTP and use real epoch
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
    
    // Publish with retain flag
    mqttClient.publish(topic.c_str(), payload.c_str(), true);
}

void setup() {
    unsigned long bootStart = millis();
    
    // Initialize serial
    Serial.begin(115200);
    delay(100);  // Allow serial to stabilize
    
    // Get device ID from MAC
    deviceId = getDeviceId();
    
    // Run self-tests
    selftestPassed = runSelfTest();
    if (!selftestPassed) {
        errorCode = "SELFTEST_FAIL";
    }
    
    // Print boot contract to serial (must be within 10 seconds)
    // Do this before WiFi to ensure we meet the deadline
    printBootContract();
    
    // Connect to WiFi
    if (!connectWiFi()) {
        Serial.println("WiFi connection failed");
        // Continue anyway - serial contract already printed
    } else {
        Serial.print("WiFi connected, IP: ");
        Serial.println(WiFi.localIP());
        
        // Connect to MQTT and publish status
        if (connectMQTT()) {
            publishMQTTStatus();
            Serial.println("MQTT status published");
        } else {
            Serial.println("MQTT connection failed");
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
        // Attempt reconnect
        connectMQTT();
    }
    
    // TODO: Add main application logic here
    // - Sensor readings
    // - Status updates
    // - Command handling
    
    delay(100);
}
