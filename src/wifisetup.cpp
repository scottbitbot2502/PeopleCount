#include "wifisetup.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "globals.h"
#include "configmanager.h"
#include <Preferences.h>
#include "ota.h"
#include "display.h"

#define DNS_PORT 53
#define SETUP_AP_SSID "PAX-Setup"
#define SETUP_AP_PASS "12345678"  // Default password

static WebServer server(80);
static DNSServer dnsServer;
static bool setupMode = false;
static Preferences nvram;
bool isWiFiSetupInProgress = false; // Global flag for WiFi setup state

// Function to connect to WiFi
bool connectWifi(const String& ssid, const String& password) {
    isWiFiSetupInProgress = true; // Set the flag when starting WiFi connection
    WiFi.disconnect(); // Ensure the Wi-Fi module is disconnected before attempting to connect
    WiFi.mode(WIFI_STA); // Set Wi-Fi mode to Station
    ESP_LOGI(TAG, "Attempting to connect to SSID: %s, Length: %d", ssid.c_str(), ssid.length());
    ESP_LOGI(TAG, "Password Length: %d", password.length());
    ESP_LOGI(TAG, "Current WiFi Status: %d", WiFi.status());
    
    if (ssid.isEmpty() || password.isEmpty() || ssid.length() > 32) {
        ESP_LOGE(TAG, "SSID or Password is empty or SSID is too long.");
        isWiFiSetupInProgress = false; // Reset the flag on failure
        return false;
    }

    if (WiFi.status() == WL_NO_SSID_AVAIL) {
        ESP_LOGE(TAG, "No SSID available. Please check the Wi-Fi module.");
        isWiFiSetupInProgress = false; // Reset the flag on failure
        return false;
    }
    
    if (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_IDLE_STATUS) {
        ESP_LOGE(TAG, "Wi-Fi module is not in a valid state for connection.");
        isWiFiSetupInProgress = false; // Reset the flag on failure
        return false;
    }
    
    WiFi.begin(ssid.c_str(), password.c_str());
    delay(100); // Allow some time for the Wi-Fi module to initialize
    int attempts = 0;
    const int maxAttempts = 10; // Increased maximum number of connection attempts
    const int retryDelay = 3000; // Increased delay before retrying

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        ESP_LOGE(TAG, "Connection attempt %d failed, status: %d", attempts + 1, WiFi.status());
        attempts++;
        delay(retryDelay); // Delay before retrying
    }

    if (WiFi.status() == WL_CONNECTED) {
        ESP_LOGI(TAG, "Successfully connected to WiFi.");
        isWiFiSetupInProgress = false; // Reset the flag on success
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to connect to WiFi after %d attempts.", maxAttempts);
        ESP_LOGE(TAG, "Current WiFi Status: %d", WiFi.status()); // Log the current status
        isWiFiSetupInProgress = false; // Reset the flag on failure
        return false;
    }
}
// HTML page for the setup interface
const char SETUP_HTML[] PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Sceene People Counter</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 0 auto; max-width: 400px; padding: 1em; }
        input, button { display: block; width: 100%; margin: 10px 0; padding: 10px; box-sizing: border-box; }
        button { background:rgb(120, 25, 198); color: white; border: none; border-radius: 4px; }
    </style>
</head>
<body>
    <h1>PAX WiFi Setup</h1>
    <form method="POST" action="/save">
        <input type="text" name="ssid" placeholder="WiFi SSID" required>
        <input type="password" name="password" placeholder="WiFi Password" required>
        <button type="submit">Save and Restart</button>
    </form>
</body>
</html>
)";

void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGI(TAG, "WiFi connection lost, attempting to reconnect...");
        WiFi.disconnect();
        WiFi.begin();  // Reconnect using saved credentials
    } else {
        ESP_LOGI(TAG, "WiFi is connected.");
    }
}

void startWiFiSetup() {
    if (setupMode) return;
    
    setupMode = true;
    isWiFiSetupInProgress = true; // Set the flag when starting WiFi setup
    ESP_LOGI(TAG, "Starting WiFi Setup Mode...");

    // Initialize WiFi
    ESP_LOGI(TAG, "Resetting Wi-Fi module...");
    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);
    
    ESP_LOGI(TAG, "Setting WiFi mode to AP...");
    // Set AP mode
    WiFi.mode(WIFI_AP);
    delay(100);

    // Configure AP
    ESP_LOGI(TAG, "Configuring AP...");
    if (WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0))) {
        ESP_LOGI(TAG, "AP Config successful, starting web server...");

        // Start AP with password
        ESP_LOGI(TAG, "Starting AP: %s", SETUP_AP_SSID);
        if (WiFi.softAP(SETUP_AP_SSID, SETUP_AP_PASS)) {
            delay(10000);  // Increased delay for AP to start
            ESP_LOGI(TAG, "AP Started at IP: %s", WiFi.softAPIP().toString().c_str());

            // Start DNS server
            dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
            dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
            ESP_LOGI(TAG, "DNS Server started");

            // Start web server
            setupWebServer();
            ESP_LOGI(TAG, "Web Server started on port 80.");

            // Handle incoming client requests
            while (setupMode) {
                server.handleClient(); // Handle incoming client requests
            }
        } else {
            ESP_LOGE(TAG, "Failed to start AP");
        }
    } else {
        ESP_LOGE(TAG, "AP Config Failed");
    }
}

bool isInSetupMode() {
    return setupMode;
}

void setupWebServer() {
    ESP_LOGI(TAG, "Initializing web server...");

    server.on("/", HTTP_GET, []() {
        ESP_LOGI(TAG, "Received GET request for root route from IP: %s", server.client().remoteIP().toString().c_str());
        server.send(200, "text/html", SETUP_HTML);
    });

    server.on("/save", HTTP_POST, []() {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        ESP_LOGI(TAG, "Received SSID: %s, Password: %s", ssid.c_str(), password.c_str());
        if (ssid.length() > 0 && password.length() > 0) {
            // Save WiFi credentials to NVRAM
            saveWiFiConfig(ssid, password);
            ESP_LOGI(TAG, "Settings saved. Restarting device...");
            // Exit setup mode and reboot
            setupMode = false; // Exit setup mode
            ESP.restart(); // Reboot the device
        } else {
            ESP_LOGE(TAG, "Invalid SSID or Password");
            server.send(200, "text/html", "Invalid SSID or Password");
        }
        delay(2000);
        ESP.restart();
    });

    server.onNotFound([]() {
        ESP_LOGI(TAG, "Redirecting to AP IP: %s", WiFi.softAPIP().toString().c_str());
        server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        server.send(302, "text/plain", "");
    });

    server.begin();
    ESP_LOGI(TAG, "Web Server started on port 80.");
    ESP_LOGI(TAG, "Server is active and waiting for client connections.");
}

void initWiFiSetup() {
    String ssid, password;
    loadWiFiConfig(ssid, password); // Load Wi-Fi credentials before any connection attempts

    if (ssid.isEmpty() || ssid == "default") {
        ESP_LOGI(TAG, "Invalid WiFi credentials, starting setup mode...");
        startWiFiSetup();
    } else {
        ESP_LOGI(TAG, "Found WiFi SSID: %s", ssid.c_str());
        ESP_LOGI(TAG, "Starting WiFi initialization process...");
    connectWifi(ssid, password); // Attempt to connect using loaded credentials
    }
}