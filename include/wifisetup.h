#ifndef _WIFISETUP_H
#define _WIFISETUP_H

void startWiFiSetup();
//void handleWiFiSetup();
bool isInSetupMode();
void initWiFiSetup();
void checkWiFiConnection(); // Add declaration for checkWiFiConnection
void setupWebServer(); // Declaration for setupWebServer
bool checkOTACredentials();

extern bool isWiFiSetupInProgress; // Global flag for WiFi setup state

#endif // _WIFISETUP_H
