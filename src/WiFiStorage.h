#pragma once

#include <Arduino.h>

void loadWiFiCredentials(String& ssid, String& password);
void saveWiFiCredentials(const String& ssid, const String& password);
void clearWiFiCredentials();
