#include "WebHandlers.h"

#include <Arduino.h>

#include "WiFiStorage.h"

static WebServer* s_server = nullptr;
static int s_relayPin = -1;

static void handleConfig();
static void handleSave();
static void handleRoot();
static void handleOn();
static void handleOff();
static void handleStatus();

void setupConfigRoutes(WebServer& server) {
  s_server = &server;
  server.on("/config", handleConfig);
  server.on("/save", handleSave);
}

void setupControlRoutes(WebServer& server, int relayPin) {
  s_server = &server;
  s_relayPin = relayPin;
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/status", handleStatus);
}

// Handler untuk halaman konfigurasi
static void handleConfig() {
  String html = "<html>"
    "<head>"
    "<title>WiFi Setup</title>"
    "<style>"
    "body { font-family: Arial; text-align: center; margin-top: 50px; }"
    "input { padding: 10px; font-size: 16px; width: 300px; margin: 10px; }"
    "button { padding: 10px 30px; font-size: 16px; cursor: pointer; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>Konfigurasi WiFi</h1>"
    "<form action='/save' method='POST'>"
    "<div>"
    "<label>SSID:</label><br>"
    "<input type='text' name='ssid' placeholder='Nama WiFi' required>"
    "</div>"
    "<div>"
    "<label>Password:</label><br>"
    "<input type='password' name='password' placeholder='Password WiFi' required>"
    "</div>"
    "<button type='submit'>Simpan &amp; Koneksi</button>"
    "</form>"
    "</body>"
    "</html>";

  s_server->send(200, "text/html", html);
}

// Handler untuk menyimpan konfigurasi
static void handleSave() {
  if (s_server->hasArg("ssid") && s_server->hasArg("password")) {
    String newSsid = s_server->arg("ssid");
    String newPassword = s_server->arg("password");

    saveWiFiCredentials(newSsid, newPassword);

    String response = "<html>"
      "<head><title>Disimpan</title></head>"
      "<body>"
      "<h1>Konfigurasi Disimpan!</h1>"
      "<p>Menghubung ke WiFi...</p>"
      "<p>Refresh halaman dalam 10 detik</p>"
      "<script>setTimeout(function(){ location.reload(); }, 10000);</script>"
      "</body>"
      "</html>";

    s_server->send(200, "text/html", response);

    // Delay untuk response, lalu restart
    delay(2000);
    ESP.restart();
  }
}

// Handler untuk root path (kontrol relay)
static void handleRoot() {
  String html = "<html>"
    "<head>"
    "<title>Relay Control</title>"
    "<style>"
    "body { font-family: Arial; text-align: center; margin-top: 50px; }"
    "button { padding: 20px 40px; font-size: 24px; margin: 10px; cursor: pointer; }"
    ".on { background-color: #4CAF50; color: white; }"
    ".off { background-color: #f44336; color: white; }"
    ".status { margin-top: 30px; font-size: 20px; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>Kontrol Relay</h1>"
    "<button class='on' onclick='fetch(\"/on\")'>NYALAKAN (ON)</button>"
    "<button class='off' onclick='fetch(\"/off\")'>MATIKAN (OFF)</button>"
    "<div class='status'>"
    "<p>Status: <span id='status'>Loading...</span></p>"
    "</div>"
    "<script>"
    "function updateStatus() {"
    "  fetch('/status').then(r => r.text()).then(data => {"
    "    document.getElementById('status').textContent = data;"
    "  });"
    "}"
    "updateStatus();"
    "setInterval(updateStatus, 1000);"
    "</script>"
    "</body>"
    "</html>";

  s_server->send(200, "text/html", html);
}

// Handler untuk /on
static void handleOn() {
  digitalWrite(s_relayPin, LOW);  // Relay ON
  Serial.println("Relay ON");
  s_server->send(200, "text/plain", "Relay ON");
}

// Handler untuk /off
static void handleOff() {
  digitalWrite(s_relayPin, HIGH);  // Relay OFF
  Serial.println("Relay OFF");
  s_server->send(200, "text/plain", "Relay OFF");
}

// Handler untuk /status
static void handleStatus() {
  String status = digitalRead(s_relayPin) == LOW ? "ON" : "OFF";
  s_server->send(200, "text/plain", status);
}
