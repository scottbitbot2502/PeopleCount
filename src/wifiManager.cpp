#include <wifiManager.h> // Include the final header file
#include <WiFi.h>  // Include the Wi-Fi library
#include "globals.h"  // Include globals for constants
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <RtcDS3231.h>

#define WIFI_MAX_TRY 10 // Define maximum attempts to connect to WiFi

// Array to store detected devices
DeviceInfo detectedDevices[100]; // Adjust size as needed
int deviceCount = 0;

bool connectWifi() {
    stopWifiScan();
    WiFi.disconnect(true);
    
    // Configure Wi-Fi with appropriate parameters
    if (WiFi.getMode() == WIFI_AP) {
        WiFi.config(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); // Align with AP settings
    }

    WiFi.setHostname(clientId);
    WiFi.mode(WIFI_STA);

    // Load WiFi credentials from NVRAM
    String ssid, password;
    loadWiFiConfig(ssid, password); // Load WiFi credentials from NVRAM
    WiFi.begin(ssid.c_str(), password.c_str()); // Use loaded credentials

    // Attempt to connect to WiFi network
    uint8_t attempts = 0;
    uint32_t startTime = millis(); // Start the timer
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_TRY) {
        if (millis() - startTime >= 300000) { // Check if 300 seconds have passed
            ESP_LOGE(TAG, "Connection timed out. Resetting to factory defaults.");
            do_reset(false); // Reset to factory defaults
            return false;
        }
        ESP_LOGI(TAG, "Attempting to connect to %s, attempt %u of %u", ssid.c_str(), attempts + 1, WIFI_MAX_TRY);
        delay(3000); // wait for stable connect
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        ESP_LOGI(TAG, "Successfully connected to WiFi.");
        ESP_LOGI(TAG, "Current WiFi Status: %d", WiFi.status()); // Log the current status
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to connect to WiFi after %u attempts", WIFI_MAX_TRY);
        return false;
    }
}

void disconnectWifi() {}

void stopWifiScan() {
    uint8_t val[] = {0};
    set_wifiscan(val);
}

void startWifiScan() {
    uint8_t val[] = {1};
    set_wifiscan(val);
    
    // Clear previous device data
    deviceCount = 0;

    // Start scanning and collect device information
    // This is a placeholder for the actual scanning logic
    // In a real implementation, you would use WiFi.scanNetworks() and collect MAC and RSSI
    for (int i = 0; i < WiFi.scanNetworks(); i++) {
        String macAddress = WiFi.BSSIDstr(i); // Get MAC address
        int rssi = WiFi.RSSI(i); // Get RSSI
        detectedDevices[deviceCount++] = {macAddress, rssi}; // Store device info
    }

    // Implement logic to analyze detectedDevices for duplicates based on RSSI and MAC
}

void analyzeDetectedDevices() {
    for (int i = 0; i < deviceCount; i++) {
        for (int j = i + 1; j < deviceCount; j++) {
            // Check if MAC addresses are the same
            if (detectedDevices[i].macAddress == detectedDevices[j].macAddress) {
                // Handle duplicate device logic, e.g., log or ignore
                ESP_LOGI(TAG, "Duplicate device detected: %s with RSSI %d and %d", 
                         detectedDevices[i].macAddress.c_str(), 
                         detectedDevices[i].rssi, 
                         detectedDevices[j].rssi);
            }
            // Additional logic can be added to handle RSSI thresholds for duplicates
        }
    }
}
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // NTP server and update interval

RtcDS3231<TwoWire> rtc(Wire); // Create an RTC object