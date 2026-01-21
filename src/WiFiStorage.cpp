#include "WiFiStorage.h"

#include <EEPROM.h>

#define EEPROM_SIZE 512
#define SSID_START 0
#define SSID_MAX_LEN 32
#define PASS_START 32
#define PASS_MAX_LEN 64
#define IP_START 96
#define IP_MAX_LEN 16
#define GATEWAY_START 112
#define GATEWAY_MAX_LEN 16
#define FLAGS_START 128
#define CREDENTIALS_SIZE 96

void loadWiFiCredentials(String& ssid, String& password) {
  EEPROM.begin(EEPROM_SIZE);

  // Baca SSID (0-31)
  ssid = "";
  for (int i = 0; i < SSID_MAX_LEN; i++) {
    char c = EEPROM.read(SSID_START + i);
    if (c == '\0') break;
    ssid += c;
  }

  // Baca Password (32-95)
  password = "";
  for (int i = 0; i < PASS_MAX_LEN; i++) {
    char c = EEPROM.read(PASS_START + i);
    if (c == '\0') break;
    password += c;
  }

  Serial.println("WiFi credentials loaded from EEPROM");
  Serial.print("SSID: ");
  Serial.println(ssid);
}

void saveWiFiCredentials(const String& ssid, const String& password) {
  EEPROM.begin(EEPROM_SIZE);

  // Hapus area sebelumnya
  for (int i = 0; i < CREDENTIALS_SIZE; i++) {
    EEPROM.write(i, 0);
  }

  // Tulis SSID
  for (int i = 0; i < ssid.length(); i++) {
    EEPROM.write(SSID_START + i, ssid[i]);
  }
  EEPROM.write(SSID_START + ssid.length(), '\0');

  // Tulis Password
  for (int i = 0; i < password.length(); i++) {
    EEPROM.write(PASS_START + i, password[i]);
  }
  EEPROM.write(PASS_START + password.length(), '\0');

  EEPROM.commit();
  Serial.println("WiFi credentials saved to EEPROM");
}

void clearWiFiCredentials() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < CREDENTIALS_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

// ============ Static IP Functions ============

void loadStaticIP(String& ip, String& gateway, bool& useStatic) {
  EEPROM.begin(EEPROM_SIZE);

  // Baca IP (96-111)
  ip = "";
  for (int i = 0; i < IP_MAX_LEN; i++) {
    char c = EEPROM.read(IP_START + i);
    if (c == '\0') break;
    ip += c;
  }

  // Baca Gateway (112-127)
  gateway = "";
  for (int i = 0; i < GATEWAY_MAX_LEN; i++) {
    char c = EEPROM.read(GATEWAY_START + i);
    if (c == '\0') break;
    gateway += c;
  }

  // Baca Flag use_static_ip
  useStatic = EEPROM.read(FLAGS_START) == 1;

  if (useStatic && ip.length() > 0 && gateway.length() > 0) {
    Serial.println("Static IP loaded from EEPROM");
    Serial.print("IP: ");
    Serial.println(ip);
    Serial.print("Gateway: ");
    Serial.println(gateway);
  }
}

void saveStaticIP(const String& ip, const String& gateway, bool useStatic) {
  EEPROM.begin(EEPROM_SIZE);

  // Clear previous IP & gateway
  for (int i = IP_START; i < GATEWAY_START + GATEWAY_MAX_LEN; i++) {
    EEPROM.write(i, 0);
  }

  // Tulis IP
  for (int i = 0; i < ip.length() && i < IP_MAX_LEN; i++) {
    EEPROM.write(IP_START + i, ip[i]);
  }
  EEPROM.write(IP_START + ip.length(), '\0');

  // Tulis Gateway
  for (int i = 0; i < gateway.length() && i < GATEWAY_MAX_LEN; i++) {
    EEPROM.write(GATEWAY_START + i, gateway[i]);
  }
  EEPROM.write(GATEWAY_START + gateway.length(), '\0');

  // Tulis Flag
  EEPROM.write(FLAGS_START, useStatic ? 1 : 0);

  EEPROM.commit();
  Serial.println("Static IP configuration saved to EEPROM");
}

void clearStaticIP() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = IP_START; i < FLAGS_START + 1; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  Serial.println("Static IP configuration cleared");
}
