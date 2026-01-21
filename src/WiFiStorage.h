#pragma once

#include <Arduino.h>

void loadWiFiCredentials(String& ssid, String& password);
void saveWiFiCredentials(const String& ssid, const String& password);
void clearWiFiCredentials();

void loadStaticIP(String& ip, String& gateway, bool& useStatic);
void saveStaticIP(const String& ip, const String& gateway, bool useStatic);
void clearStaticIP();
