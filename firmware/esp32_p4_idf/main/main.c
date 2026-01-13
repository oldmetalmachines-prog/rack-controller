/*
 * ESP32-P4 Rack Controller Firmware
 * 
 * PLACEHOLDER - Implementation pending hardware availability
 * 
 * This file provides the scaffolding for the ESP32-P4 native ESP-IDF
 * implementation of the rack controller boot contract.
 * 
 * Boot Contract:
 * - Serial JSON output within 10 seconds
 * - MQTT status publication (retained)
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Placeholder - actual implementation when P4 is available
// #include "esp_wifi.h"
// #include "esp_event.h"
// #include "mqtt_client.h"

static const char *TAG = "rack_p4";

// Build-time version (injected by build system)
#ifndef FW_VERSION
#define FW_VERSION "unknown"
#endif

#define TARGET_NAME "p4"

/**
 * Get device ID from MAC address
 * Placeholder implementation
 */
static void get_device_id(char *id_out, size_t len) {
    // TODO: Implement when P4 SDK is available
    // uint8_t mac[6];
    // esp_read_mac(mac, ESP_MAC_WIFI_STA);
    // snprintf(id_out, len, "p4-%02x%02x%02x%02x%02x%02x", ...);
    
    snprintf(id_out, len, "p4-placeholder");
}

/**
 * Run self-tests
 */
static bool run_selftest(void) {
    // TODO: Add hardware-specific tests
    return true;
}

/**
 * Print boot contract JSON to serial
 */
static void print_boot_contract(const char *device_id, bool selftest_passed) {
    // Boot contract JSON - must be printed within 10 seconds of boot
    if (selftest_passed) {
        printf("{\"device\":\"%s\",\"fw\":\"%s\",\"target\":\"%s\",\"selftest\":\"pass\"}\n",
               device_id, FW_VERSION, TARGET_NAME);
    } else {
        printf("{\"device\":\"%s\",\"fw\":\"%s\",\"target\":\"%s\",\"selftest\":\"fail\",\"err\":\"SELFTEST_FAIL\"}\n",
               device_id, FW_VERSION, TARGET_NAME);
    }
}

/**
 * Publish MQTT status (placeholder)
 */
static void publish_mqtt_status(const char *device_id, bool selftest_passed) {
    // TODO: Implement MQTT publishing when P4 SDK is available
    // Topic: lab/<device_id>/status
    // Payload: JSON with device, fw, target, selftest, ts fields
    
    ESP_LOGI(TAG, "MQTT publish placeholder - implementation pending");
}

void app_main(void) {
    ESP_LOGI(TAG, "ESP32-P4 Rack Controller Starting...");
    ESP_LOGW(TAG, "This is a PLACEHOLDER build - P4 support not yet implemented");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Get device ID
    char device_id[32];
    get_device_id(device_id, sizeof(device_id));
    ESP_LOGI(TAG, "Device ID: %s", device_id);
    
    // Run self-tests
    bool selftest_passed = run_selftest();
    
    // Print boot contract (must be within 10 seconds)
    print_boot_contract(device_id, selftest_passed);
    
    // TODO: Initialize WiFi and MQTT when P4 support is ready
    // wifi_init();
    // mqtt_init();
    // publish_mqtt_status(device_id, selftest_passed);
    
    ESP_LOGI(TAG, "Boot complete (placeholder mode)");
    
    // Main loop placeholder
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
