#pragma once

#include <Arduino.h>

// Batas panjang field (tidak termasuk null-terminator)
static constexpr int WIFI_SSID_MAX_LEN = 32;
static constexpr int WIFI_PASS_MAX_LEN = 64;
static constexpr int WIFI_IP_MAX_LEN   = 16;

void loadWiFiCredentials(String& ssid, String& password);
void saveWiFiCredentials(const String& ssid, const String& password);
void clearWiFiCredentials();

void loadStaticIP(String& ip, String& gateway, String& subnet, String& dns1, bool& useStatic);
void saveStaticIP(const String& ip, const String& gateway, const String& subnet, const String& dns1, bool useStatic);
void clearStaticIP();
