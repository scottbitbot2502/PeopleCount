#ifndef _WIFIMANAGER_H
#define _WIFIMANAGER_H

#include <rcommand.h> // must be included here so that Arduino library object file references work
#include "globals.h"
#include "timekeeper.h"

// Define the DeviceInfo structure
struct DeviceInfo {
    String macAddress;
    int rssi;
};

// Declare the extern variables
extern DeviceInfo detectedDevices[]; // Expose detectedDevices array
extern int deviceCount; // Expose deviceCount variable

bool connectWifi();
void analyzeDetectedDevices(); // Declare the function for testing
void startWifiScan();
void stopWifiScan();

#endif // _WIFIMANAGER_H
