#include "WiFiStorage.h"

#include <EEPROM.h>

#define EEPROM_SIZE 512
#define SSID_START 0
#define SSID_MAX_LEN WIFI_SSID_MAX_LEN
#define PASS_START 32
#define PASS_MAX_LEN WIFI_PASS_MAX_LEN
#define IP_START 96
#define IP_MAX_LEN WIFI_IP_MAX_LEN
#define GATEWAY_START 112
#define GATEWAY_MAX_LEN WIFI_IP_MAX_LEN
#define SUBNET_START 128
#define SUBNET_MAX_LEN WIFI_IP_MAX_LEN
#define DNS1_START 144
#define DNS1_MAX_LEN WIFI_IP_MAX_LEN
#define FLAGS_START 160
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

  int ssidLen = min((int)ssid.length(), SSID_MAX_LEN - 1);
  int passLen = min((int)password.length(), PASS_MAX_LEN - 1);

  // Hapus area sebelumnya
  for (int i = 0; i < CREDENTIALS_SIZE; i++) {
    EEPROM.write(i, 0);
  }

  // Tulis SSID
  for (int i = 0; i < ssidLen; i++) {
    EEPROM.write(SSID_START + i, ssid[i]);
  }
  EEPROM.write(SSID_START + ssidLen, '\0');

  // Tulis Password
  for (int i = 0; i < passLen; i++) {
    EEPROM.write(PASS_START + i, password[i]);
  }
  EEPROM.write(PASS_START + passLen, '\0');

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

void loadStaticIP(String& ip, String& gateway, String& subnet, String& dns1, bool& useStatic) {
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

  // Baca Subnet (128-143)
  subnet = "";
  for (int i = 0; i < SUBNET_MAX_LEN; i++) {
    char c = EEPROM.read(SUBNET_START + i);
    if (c == '\0') break;
    subnet += c;
  }

  // Baca DNS1 (144-159)
  dns1 = "";
  for (int i = 0; i < DNS1_MAX_LEN; i++) {
    char c = EEPROM.read(DNS1_START + i);
    if (c == '\0') break;
    dns1 += c;
  }

  // Baca Flag use_static_ip
  useStatic = EEPROM.read(FLAGS_START) == 1;

  if (useStatic && ip.length() > 0 && gateway.length() > 0) {
    Serial.println("Static IP loaded from EEPROM");
    Serial.print("IP: ");
    Serial.println(ip);
    Serial.print("Gateway: ");
    Serial.println(gateway);
    if (subnet.length() > 0) {
      Serial.print("Subnet: ");
      Serial.println(subnet);
    }
    if (dns1.length() > 0) {
      Serial.print("DNS: ");
      Serial.println(dns1);
    }
  }
}

void saveStaticIP(const String& ip, const String& gateway, const String& subnet, const String& dns1, bool useStatic) {
  EEPROM.begin(EEPROM_SIZE);

  int ipLen = min((int)ip.length(), IP_MAX_LEN - 1);
  int gwLen = min((int)gateway.length(), GATEWAY_MAX_LEN - 1);
  int subnetLen = min((int)subnet.length(), SUBNET_MAX_LEN - 1);
  int dnsLen = min((int)dns1.length(), DNS1_MAX_LEN - 1);

  // Clear previous IP & gateway
  for (int i = IP_START; i < DNS1_START + DNS1_MAX_LEN; i++) {
    EEPROM.write(i, 0);
  }

  // Tulis IP
  for (int i = 0; i < ipLen; i++) {
    EEPROM.write(IP_START + i, ip[i]);
  }
  EEPROM.write(IP_START + ipLen, '\0');

  // Tulis Gateway
  for (int i = 0; i < gwLen; i++) {
    EEPROM.write(GATEWAY_START + i, gateway[i]);
  }
  EEPROM.write(GATEWAY_START + gwLen, '\0');

  // Tulis Subnet
  for (int i = 0; i < subnetLen; i++) {
    EEPROM.write(SUBNET_START + i, subnet[i]);
  }
  EEPROM.write(SUBNET_START + subnetLen, '\0');

  // Tulis DNS
  for (int i = 0; i < dnsLen; i++) {
    EEPROM.write(DNS1_START + i, dns1[i]);
  }
  EEPROM.write(DNS1_START + dnsLen, '\0');

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
