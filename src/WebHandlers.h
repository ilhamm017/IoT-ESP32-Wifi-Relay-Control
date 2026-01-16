#pragma once

#include <WebServer.h>

void setupConfigRoutes(WebServer& server);
void setupControlRoutes(WebServer& server, int relayPin);
