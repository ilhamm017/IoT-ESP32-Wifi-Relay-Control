#include "WiFiStorage.h"

#include <EEPROM.h>

#define EEPROM_SIZE 512
#define SSID_START 0
#define SSID_MAX_LEN 32
#define PASS_START 32
#define PASS_MAX_LEN 64
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
