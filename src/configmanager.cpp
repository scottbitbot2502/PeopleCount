/* configmanager persists runtime configuration using NVRAM of ESP32*/

#include "globals.h"
#include "configmanager.h"
#include "../shared/paxcounter.conf"

// namespace for device runtime preferences
#define DEVCONFIG "paxcntcfg"

Preferences nvram;

configData_t cfg; // struct holds current device configuration

static const uint8_t cfgMagicBytes[] = {0x21, 0x76, 0x87, 0x32, 0xf4};
static const size_t cfgLen = sizeof(cfg), cfgLen2 = sizeof(cfgMagicBytes);
static uint8_t buffer[cfgLen + cfgLen2];

// helper function to convert strings into lower case
bool comp(char s1, char s2) { 
    return (tolower(s1) < tolower(s2)); 
}

// populate runtime config with device factory settings
static void defaultConfig(configData_t *myconfig) {
  strncpy(myconfig->version, PROGVERSION,
          sizeof(myconfig->version) - 1); // Firmware version

  // device factory settings with default values
  myconfig->loradr = 5;      // 0-15, lora datarate
  myconfig->txpower = 14;    // 0-15, lora tx power
  myconfig->adrmode = 1;     // 0=disabled, 1=enabled
  myconfig->screensaver = 0; // 0=disabled, 1=enabled
  myconfig->screenon = 1;    // 0=disabled, 1=enabled
  myconfig->countermode = 0; // 0=cyclic, 1=cumulative, 2=cyclic confirmed
  myconfig->rssilimit = 0;   // threshold for rssilimiter
  myconfig->sendcycle = 30;  // payload send cycle [seconds/2]
  myconfig->sleepcycle = 0;  // sleep cycle [seconds/10]
  myconfig->wakesync = 300;  // wakeup sync window [seconds]
  myconfig->wifichancycle = 100; // wifi channel switch cycle [seconds/100]
  myconfig->wifichanmap = 0xFFFF; // wifi channel hopping scheme (all channels)
  myconfig->blescantime = 1;  // BT scan cycle [seconds/100]
  myconfig->blescan = 1;      // 0=disabled, 1=enabled
  myconfig->wifiscan = 1;     // 0=disabled, 1=enabled
  myconfig->wifiant = 0;      // 0=internal, 1=external
  myconfig->rgblum = 30;      // RGB Led luminosity (0..100%)
  myconfig->payloadmask = 0xFF; // all payloads enabled by default

#ifdef HAS_BME680
  // initial BSEC state for BME680 sensor
  myconfig->bsecstate[BSEC_MAX_STATE_BLOB_SIZE] = {0};
#endif
}

// migrate runtime configuration from earlier to current version
static void migrateConfig(void) {
  eraseConfig();
}

// save current configuration from RAM to NVRAM
void saveConfig(bool erase) {
  ESP_LOGI(TAG, "Storing settings to NVRAM...");

  nvram.begin(DEVCONFIG, false);

  if (erase) {
    ESP_LOGI(TAG, "Resetting device to factory settings");
    nvram.clear();
    defaultConfig(&cfg);
  }

  // Copy device runtime config cfg to byte array, padding it with magicBytes
  memcpy(buffer, &cfg, cfgLen);
  memcpy(buffer + cfgLen, &cfgMagicBytes, cfgLen2);

  // save byte array to NVRAM, padding with cfg magicbytes
  if (nvram.putBytes(DEVCONFIG, buffer, cfgLen + cfgLen2))
    ESP_LOGI(TAG, "Device settings saved");
  else
    ESP_LOGE(TAG, "NVRAM Error, device settings not saved");

  nvram.end();
}

// load configuration from NVRAM into RAM and make it current
void loadConfig(void) {
  int readBytes = 0;

  ESP_LOGI(TAG, "Loading device configuration from NVRAM...");

  if (nvram.begin(DEVCONFIG, true)) {
    // load device runtime config from nvram and copy it to byte array
    readBytes = nvram.getBytes(DEVCONFIG, buffer, cfgLen + cfgLen2);
    nvram.end();

    // check that runtime config data length matches
    if (readBytes != cfgLen + cfgLen2) {
      ESP_LOGE(TAG, "No valid configuration found");
      migrateConfig();
    }

  } else {
    ESP_LOGI(TAG, "NVRAM initialized, device starts with factory settings");
    eraseConfig();
  }

  // validate loaded configuration by checking magic bytes at end of array
  if (memcmp(buffer + cfgLen, &cfgMagicBytes, cfgLen2) != 0) {
    ESP_LOGE(TAG, "Configuration data corrupt");
    eraseConfig();
  }

  // copy loaded configuration into runtime cfg struct
  memcpy(&cfg, buffer, cfgLen);
  ESP_LOGI(TAG, "Runtime configuration v%s loaded", cfg.version);

  // check if config version matches current firmware version
  switch (version_compare(PROGVERSION, cfg.version)) {
  case -1: // device configuration belongs to newer than current firmware
    ESP_LOGE(TAG, "Incompatible device configuration");
    eraseConfig();
    break;
  case 1: // device configuration belongs to older than current firmware
    ESP_LOGW(TAG, "Device was updated, attempt to migrate configuration");
    migrateConfig();
    break;
  default: // device configuration version matches current firmware version
    break; // nothing to do here
  }
}

// helper function to lexicographically compare two versions. Returns 1 if v2
// is smaller, -1 if v1 is smaller, 0 if equal
int version_compare(const String v1, const String v2) {
  if (v1 == v2)
    return 0;

  const char *a1 = v1.c_str(), *a2 = v2.c_str();

  if (std::lexicographical_compare(a1, a1 + strlen(a1), a2, a2 + strlen(a2),
                                   comp))
    return -1;
  else
    return 1;
}

// WiFi configuration functions
void loadWiFiConfig(String &ssid, String &password) {
    Preferences wifiNvram;
    wifiNvram.begin("wifi", true);
    ssid = wifiNvram.getString("ssid", "");
    password = wifiNvram.getString("password", "");
    wifiNvram.end();

    ESP_LOGI(TAG, "Loaded SSID: %s, Password: %s", ssid.c_str(), password.c_str()); // Debug logging
    if (ssid.length() > 0) {
        ESP_LOGI(TAG, "Found stored WiFi credentials for SSID: %s", ssid.c_str());
    } else {
        ESP_LOGE(TAG, "No valid WiFi credentials found.");
    }
}

void saveWiFiConfig(const String &ssid, const String &password) {
  ESP_LOGI(TAG, "Saving WiFi credentials to NVRAM...");
  Preferences wifiNvram;
  wifiNvram.begin("wifi", false);
  wifiNvram.putString("ssid", ssid);
  wifiNvram.putString("password", password);
  wifiNvram.end();
  ESP_LOGI(TAG, "WiFi credentials saved for SSID: %s", ssid.c_str());
  ESP_LOGI(TAG, "WiFi credentials saved successfully."); // Additional debug logging
}

void eraseConfig(void) {
    ESP_LOGI(TAG, "Erasing configuration...");
    reset_rtc_vars(); // Reset any RTC variables if necessary
    saveConfig(true); // Save the configuration with erase flag set to true
}
