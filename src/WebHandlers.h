#pragma once

#include <Arduino.h>
#include <WebServer.h>

#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 2  // LED indikator (default: onboard LED)
#endif

#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW true
#endif

void setupConfigRoutes(WebServer& server);
void setupControlRoutes(WebServer& server, const int* relayPins, size_t relayCount);
void setupAdditionalRoutes(WebServer& server);
// Lakukan scan awal; jika forceStaOnly true, AP dimatikan sementara supaya scan lintas kanal
void primeScan(bool forceStaOnly = false);
