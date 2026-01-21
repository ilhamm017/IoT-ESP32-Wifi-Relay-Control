#pragma once

#include <WebServer.h>

void setupConfigRoutes(WebServer& server);
void setupControlRoutes(WebServer& server, int relayPin1, int relayPin2);
void setupAdditionalRoutes(WebServer& server, int relayPin1, int relayPin2);
