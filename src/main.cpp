#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "WebHandlers.h"
#include "WiFiStorage.h"

// Konfigurasi Relay
#define RELAY_PIN1 32  // GPIO 32 untuk Relay
#define RELAY_PIN2 33 // GPIO 33 untuk Relay
#define RESET_BUTTON_PIN 0  // GPIO 0 = tombol BOOT di board ESP32

// Web Server pada port 80
WebServer server(80);

// Variabel untuk menyimpan WiFi credentials
String ssid = "";
String password = "";
String staticIp = "";
String gateway = "";
bool useStaticIP = false;
bool wifiConnected = false;

// Setup WiFi AP untuk konfigurasi
void setupConfigAP() {
  Serial.println("\nMemulai Access Point untuk konfigurasi...");
  // AP+STA supaya scan WiFi bisa jalan saat mode setup
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP32-Setup", "12345678");

  Serial.println("Access Point aktif:");
  Serial.println("SSID: ESP32-Setup");
  Serial.println("Password: 12345678");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  setupConfigRoutes(server);
  setupAdditionalRoutes(server, RELAY_PIN1, RELAY_PIN2);  // Add new routes
  server.begin();
}

// Coba koneksi ke WiFi
void connectToWiFi() {
  if (ssid.length() == 0) {
    Serial.println("SSID kosong, setup AP...");
    setupConfigAP();
    return;
  }

  Serial.println("\nMenghubung ke WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  
  // Apply static IP if configured
  if (useStaticIP && staticIp.length() > 0 && gateway.length() > 0) {
    IPAddress ip;
    IPAddress gw;
    IPAddress subnet(255, 255, 255, 0);
    
    if (ip.fromString(staticIp) && gw.fromString(gateway)) {
      WiFi.config(ip, gw, subnet);
      Serial.println("Applying static IP configuration:");
      Serial.print("IP: ");
      Serial.println(staticIp);
      Serial.print("Gateway: ");
      Serial.println(gateway);
    } else {
      Serial.println("Invalid static IP configuration, using DHCP");
      useStaticIP = false;
    }
  }
  
  WiFi.begin(ssid.c_str(), password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;

    // Setup server routes (both control + config routes accessible)
    setupControlRoutes(server, RELAY_PIN1, RELAY_PIN2);
    setupConfigRoutes(server);  // Make config accessible even when connected
    setupAdditionalRoutes(server, RELAY_PIN1, RELAY_PIN2);
    server.begin();
    Serial.println("Server dimulai! Config tetap dapat diakses di /config");
  } else {
    Serial.println("\nGagal terhubung ke WiFi, setup AP...");
    setupConfigAP();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Setup Relay
  pinMode(RELAY_PIN1, OUTPUT);
  digitalWrite(RELAY_PIN1, HIGH);  // Relay OFF
  pinMode(RELAY_PIN2, OUTPUT);
  digitalWrite(RELAY_PIN2, HIGH);  // Relay OFF

  // Setup Reset Button
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  Serial.println("\n\n=== ESP32 Relay Control Server v2 ===");
  Serial.println("Features: WiFi Scan, Static IP, Persistent Config");
  Serial.println("Ketik 'reset' di Serial Monitor untuk reset WiFi credentials");
  Serial.println("Ketik 'status' untuk lihat current status");
  Serial.println("ATAU tekan tombol BOOT selama 5 detik untuk reset");

  // Load WiFi credentials dari EEPROM
  loadWiFiCredentials(ssid, password);
  
  // Load Static IP configuration
  loadStaticIP(staticIp, gateway, useStaticIP);

  // Coba koneksi
  connectToWiFi();
}

void loop() {
  server.handleClient();

  // Deteksi tombol BOOT ditekan selama 5 detik
  static unsigned long buttonPressStart = 0;
  static bool buttonPressed = false;

  if (digitalRead(RESET_BUTTON_PIN) == LOW) {  // Tombol ditekan
    if (!buttonPressed) {
      buttonPressStart = millis();
      buttonPressed = true;
      Serial.println("\nTombol BOOT ditekan...");
    }

    // Cek apakah sudah ditekan 5 detik
    unsigned long pressDuration = millis() - buttonPressStart;
    if (pressDuration >= 5000) {  // 5 detik
      Serial.println("Tombol BOOT ditekan selama 5 detik! Mereset WiFi...");
      clearWiFiCredentials();
      Serial.println("WiFi credentials dihapus! Restarting...");
      delay(1000);
      ESP.restart();
    }
  } else {  // Tombol dilepas
    buttonPressed = false;
  }

  // Cek Serial Monitor untuk reset command
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "reset") {
      Serial.println("\nMenghapus WiFi credentials dari EEPROM...");
      clearWiFiCredentials();
      Serial.println("EEPROM direset! Restarting...");
      delay(1000);
      ESP.restart();
    }
    else if (command == "status") {
      Serial.println("\n=== Status ===");
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("WiFi Connected: ");
      Serial.println(wifiConnected ? "Yes" : "No");
      if (wifiConnected) {
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
      }
    }
  }
}
