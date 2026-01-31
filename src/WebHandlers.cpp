#include "WebHandlers.h"

#include <Arduino.h>
#include <WiFi.h>

#include "WiFiStorage.h"

static WebServer* s_server = nullptr;
static int s_relayPin1 = -1;
static int s_relayPin2 = -1;
static const char* AUTH_USER = "admin";
static const char* AUTH_PASS = "admin123";
static const char* AP_SSID   = "ESP32-Setup";
static const char* AP_PASS   = "12345678";
static String s_lastScanJson;

static void handleConfig();
static void handleSave();
static void handleRoot();
static void handleSwitch1On();
static void handleSwitch1Off();
static void handleSwitch2On();
static void handleSwitch2Off();
static void handleStatus1();
static void handleStatus2();
static void handleScanWiFi();
static void handleSetStaticIP();
static bool ensureAuth();
static String jsonEscape(const String& input);
static int runWiFiScan(String& jsonOut, bool forceStaOnly);
static void ledOn();
static void ledOff();
static void ledToggle();
void primeScan(bool forceStaOnly) {
  if (s_lastScanJson.length() == 0 || forceStaOnly) {
    String tmp;
    runWiFiScan(tmp, forceStaOnly);
  }
}

void setupConfigRoutes(WebServer& server) {
  s_server = &server;
  server.on("/config", handleConfig);
  server.on("/save", handleSave);
}

void setupControlRoutes(WebServer& server, int relayPin1, int relayPin2) {
  s_server = &server;
  s_relayPin1 = relayPin1;
  s_relayPin2 = relayPin2;
  server.on("/", handleRoot);
  server.on("/1/on", handleSwitch1On);
  server.on("/1/off", handleSwitch1Off);
  server.on("/2/on", handleSwitch2On);
  server.on("/2/off", handleSwitch2Off);
  server.on("/1/status", handleStatus1);
  server.on("/2/status", handleStatus2);
}

void setupAdditionalRoutes(WebServer& server, int relayPin1, int relayPin2) {
  s_server = &server;
  s_relayPin1 = relayPin1;
  s_relayPin2 = relayPin2;
  // Routes tambahan untuk WiFi scan & IP setting
  server.on("/api/scan-wifi", handleScanWiFi);
  server.on("/api/set-static-ip", HTTP_POST, handleSetStaticIP);
  server.on("/api/set-static-ip", HTTP_GET, handleSetStaticIP);
}

// Helper: Basic Auth untuk semua endpoint sensitif
static bool ensureAuth() {
  if (!s_server) return false;
  if (!s_server->authenticate(AUTH_USER, AUTH_PASS)) {
    s_server->requestAuthentication();
    return false;
  }
  return true;
}

// Helper: escape string untuk JSON sederhana
static String jsonEscape(const String& input) {
  String out;
  out.reserve(input.length() + 4);
  for (unsigned int i = 0; i < input.length(); i++) {
    char c = input[i];
    switch (c) {
      case '\"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          // Lewati karakter kontrol lain
        } else {
          out += c;
        }
    }
  }
  return out;
}

// Handler untuk halaman konfigurasi (dengan WiFi scan & static IP)
static void handleConfig() {
  if (!ensureAuth()) return;
  // Auto scan saat halaman diakses jika belum ada cache
  if (s_lastScanJson.length() == 0) {
    String tmp;
    runWiFiScan(tmp, false);
  }
  String html = "<html>"
    "<head>"
    "<title>WiFi Setup</title>"
    "<style>"
    "body { font-family: Arial; text-align: center; margin-top: 30px; padding: 20px; }"
    "input, select { padding: 10px; font-size: 16px; width: 300px; margin: 10px; display: block; margin-left: auto; margin-right: auto; }"
    "button { padding: 10px 30px; font-size: 16px; cursor: pointer; margin: 10px; }"
    ".section { margin: 30px 0; border: 1px solid #ccc; padding: 20px; border-radius: 5px; }"
    ".advanced { background-color: #f0f0f0; }"
    "#ssidList { display: block; width: 300px; margin: 10px auto; }"
    ".info { color: #666; font-size: 14px; margin-top: 5px; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>Konfigurasi WiFi ESP32</h1>"
    
    // WiFi Selection
    "<div class='section'>"
    "<h2>1. Pilih Jaringan WiFi</h2>"
    "<button id='scanBtn' onclick='scanWiFi(this)'>Scan Jaringan WiFi</button>"
    "<p class='info'>Atau pilih dari list:</p>"
    "<select id='ssidList'>"
    "<option value=''>-- Tekan Scan untuk list WiFi --</option>"
    "</select>"
    "</div>"
    
    // WiFi Credentials
    "<div class='section'>"
    "<h2>2. Masukkan Kredensial</h2>"
    "<form id='configForm' action='/save' method='POST'>"
    "<div>"
    "<label>SSID (Nama WiFi):</label><br>"
    "<input type='text' id='ssidInput' name='ssid' placeholder='Masukkan atau pilih dari list' required>"
    "</div>"
    "<div>"
    "<label>Password WiFi:</label><br>"
    "<input type='password' name='password' placeholder='Password WiFi' required>"
    "</div>"
    
    // Static IP (Advanced)
    "<div class='advanced'>"
    "<h3>Konfigurasi IP Static (Optional)</h3>"
    "<label>"
    "<input type='checkbox' name='useStatic' id='useStatic' onchange='toggleStaticIP()'>"
    "Gunakan Static IP"
    "</label>"
    
    "<div id='staticIPDiv' style='display:none;'>"
    "<div>"
    "<label>IP Address:</label><br>"
    "<input type='text' name='staticIp' id='staticIp' placeholder='192.168.1.105'>"
    "<p class='info'>Contoh: 192.168.1.105</p>"
    "</div>"
    "<div>"
    "<label>Gateway:</label><br>"
    "<input type='text' name='gateway' id='gateway' placeholder='192.168.1.1'>"
    "<p class='info'>Biasanya: 192.168.1.1 atau 192.168.0.1</p>"
    "</div>"
    "<div>"
    "<label>Subnet Mask:</label><br>"
    "<input type='text' name='subnet' id='subnet' placeholder='255.255.255.0'>"
    "<p class='info'>Contoh: 255.255.255.0</p>"
    "</div>"
    "<div>"
    "<label>DNS (opsional):</label><br>"
    "<input type='text' name='dns1' id='dns1' placeholder='8.8.8.8'>"
    "<p class='info'>Kosongkan untuk memakai Gateway sebagai DNS</p>"
    "</div>"
    "</div>"
    "</div>"
    
    "<button type='submit'>Simpan &amp; Koneksi</button>"
    "</form>"
    "</div>"
    
    "<script>"
    "function toggleStaticIP() {"
    "  var staticDiv = document.getElementById('staticIPDiv');"
    "  var checkbox = document.getElementById('useStatic');"
    "  staticDiv.style.display = checkbox.checked ? 'block' : 'none';"
    "}"
    
    "function scanWiFi(btn, silent) {"
    "  if (btn) { btn.disabled = true; btn.textContent = 'Scanning...'; }"
    "  fetch('/api/scan-wifi')"
    "    .then(r => r.json())"
    "    .then(data => {"
    "      var select = document.getElementById('ssidList');"
    "      select.innerHTML = '<option value=\"\">-- Pilih SSID --</option>';"
    "      data.networks.forEach(net => {"
    "        var opt = document.createElement('option');"
    "        opt.value = net.ssid;"
    "        opt.text = net.ssid + ' (Signal: ' + net.rssi + ' dBm)';"
    "        select.appendChild(opt);"
    "      });"
    "      if (btn) { btn.disabled = false; btn.textContent = 'Scan Ulang'; }"
    "    })"
    "    .catch(err => {"
    "      if (!silent) alert('Scan gagal: ' + err);"
    "      if (btn) { btn.disabled = false; btn.textContent = 'Scan Jaringan WiFi'; }"
    "    });"
    "}"
    
    "document.getElementById('ssidList').addEventListener('change', function() {"
    "  document.getElementById('ssidInput').value = this.value;"
    "});"

    "window.addEventListener('load', function(){"
    "  var btn = document.getElementById('scanBtn');"
    "  scanWiFi(btn, true);"
    "});"
    
    "document.getElementById('configForm').addEventListener('submit', function(e) {"
    "  var ip = document.getElementById('staticIp').value;"
    "  var gateway = document.getElementById('gateway').value;"
    "  var subnet = document.getElementById('subnet').value;"
    "  var useStatic = document.getElementById('useStatic').checked;"
    "  if (useStatic && (!ip || !gateway)) {"
    "    alert('Masukkan IP dan Gateway jika menggunakan Static IP');"
    "    e.preventDefault();"
    "  }"
    "  if (useStatic && subnet && subnet.split('.').length !== 4) {"
    "    alert('Subnet tidak valid');"
    "    e.preventDefault();"
    "  }"
    "});"
    "</script>"
    "</body>"
    "</html>";

  s_server->send(200, "text/html", html);
}

// Handler untuk menyimpan konfigurasi
static void handleSave() {
  if (!ensureAuth()) return;
  if (s_server->hasArg("ssid") && s_server->hasArg("password")) {
    String newSsid = s_server->arg("ssid");
    String newPassword = s_server->arg("password");
    bool useStatic = s_server->hasArg("useStatic");
    String staticIp = s_server->arg("staticIp");
    String gateway = s_server->arg("gateway");
    String subnet = s_server->arg("subnet");
    String dns1 = s_server->arg("dns1");

    if (newSsid.length() >= WIFI_SSID_MAX_LEN) {
      s_server->send(400, "text/plain", "SSID terlalu panjang (maks 31 karakter)");
      return;
    }
    if (newPassword.length() >= WIFI_PASS_MAX_LEN) {
      s_server->send(400, "text/plain", "Password terlalu panjang (maks 63 karakter)");
      return;
    }
    auto ipTooLong = [](const String& val) {
      return val.length() >= WIFI_IP_MAX_LEN;
    };
    if (ipTooLong(staticIp) || ipTooLong(gateway) || ipTooLong(subnet) || ipTooLong(dns1)) {
      s_server->send(400, "text/plain", "IP/Gateway/Subnet/DNS terlalu panjang");
      return;
    }

    // Save WiFi credentials
    saveWiFiCredentials(newSsid, newPassword);

    // Save static IP if enabled
    if (useStatic && staticIp.length() > 0 && gateway.length() > 0) {
      if (subnet.length() == 0) subnet = "255.255.255.0";
      saveStaticIP(staticIp, gateway, subnet, dns1, true);
      Serial.println("Static IP configuration saved");
    } else {
      clearStaticIP();
      Serial.println("Static IP disabled");
    }

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
    "<button class='on' onclick='fetch(\"/1/on\")'>Saklar 1 ON</button>"
    "<button class='off' onclick='fetch(\"/1/off\")'>Saklar 1 OFF</button>"
    "<button class='on' onclick='fetch(\"/2/on\")'>Saklar 2 ON</button>"
    "<button class='off' onclick='fetch(\"/2/off\")'>Saklar 2 OFF</button>"
    "<div class='status'>"
    "<p>Status Saklar 1: <span id='status1'>Loading...</span></p>"
    "<p>Status Saklar 2: <span id='status2'>Loading...</span></p>"
    "</div>"
    "<script>"
    "function updateStatus() {"
    "  fetch('/1/status').then(r => r.text()).then(data => {"
    "    document.getElementById('status1').textContent = data;"
    "  });"
    "  fetch('/2/status').then(r => r.text()).then(data => {"
    "    document.getElementById('status2').textContent = data;"
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
static void handleSwitch1On() {
  digitalWrite(s_relayPin1, LOW);  // Relay 1 ON
  Serial.println("Relay 1 ON");
  s_server->send(200, "text/plain", "Relay 1 ON");
}

// Handler untuk /1/off
static void handleSwitch1Off() {
  digitalWrite(s_relayPin1, HIGH);  // Relay 1 OFF
  Serial.println("Relay 1 OFF");
  s_server->send(200, "text/plain", "Relay 1 OFF");
}

// Handler untuk /2/on
static void handleSwitch2On() {
  digitalWrite(s_relayPin2, LOW);  // Relay 2 ON
  Serial.println("Relay 2 ON");
  s_server->send(200, "text/plain", "Relay 2 ON");
}

// Handler untuk /2/off
static void handleSwitch2Off() {
  digitalWrite(s_relayPin2, HIGH);  // Relay 2 OFF
  Serial.println("Relay 2 OFF");
  s_server->send(200, "text/plain", "Relay 2 OFF");
}

// Handler untuk /1/status
static void handleStatus1() {
  String status = digitalRead(s_relayPin1) == LOW ? "ON" : "OFF";
  s_server->send(200, "text/plain", status);
}

// Handler untuk /2/status
static void handleStatus2() {
  String status = digitalRead(s_relayPin2) == LOW ? "ON" : "OFF";
  s_server->send(200, "text/plain", status);
}

// ============ NEW FEATURES ============

// Handler untuk scan WiFi network
static void handleScanWiFi() {
  if (!ensureAuth()) return;
  Serial.println("Scanning available WiFi networks...");

  String json;
  int networks = runWiFiScan(json, false);
  s_server->send(200, "application/json", json);
  Serial.print("Found ");
  Serial.print(networks);
  Serial.println(" networks");
}

// Jalankan scan WiFi dengan mematikan AP sementara agar bisa lintas kanal
static int runWiFiScan(String& jsonOut, bool forceStaOnly) {
  wifi_mode_t prevMode = WiFi.getMode();
  bool apWasOn = (prevMode == WIFI_AP || prevMode == WIFI_AP_STA);
  // Jika diminta STA only, matikan AP sementara agar bisa lintas kanal penuh
  if (forceStaOnly && apWasOn) {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
  } else if (prevMode == WIFI_AP) {
    WiFi.mode(WIFI_AP_STA);  // tetap siarkan AP tapi STA aktif
  }

  // Async scan supaya LED bisa berkedip selama pencarian
  WiFi.scanDelete();
  WiFi.scanNetworks(true /* async */, true /* show_hidden */);

  unsigned long lastToggle = millis();
  ledOn();

  int networks = 0;
  while (true) {
    int res = WiFi.scanComplete();
    if (res >= 0) { networks = res; break; }
    if (res == WIFI_SCAN_FAILED) { networks = 0; break; }
    unsigned long now = millis();
    if (now - lastToggle >= 150) {
      ledToggle();
      lastToggle = now;
    }
    delay(10);
  }
  ledOff();

  String json = "{\"networks\":[";
  for (int i = 0; i < networks; i++) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + jsonEscape(WiFi.SSID(i)) + "\",\"rssi\":" + WiFi.RSSI(i) + "}";
  }
  json += "]}";

  // nyalakan kembali AP jika sebelumnya aktif dan dimatikan
  if (forceStaOnly && apWasOn) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.println("Access Point dinyalakan kembali setelah scan STA-only");
  } else if (!apWasOn) {
    WiFi.mode(prevMode);
  } else if (prevMode == WIFI_AP) {
    WiFi.mode(WIFI_AP);  // kembali ke AP saja bila sebelumnya AP only
  } // jika AP_STA, biarkan tetap AP_STA

  s_lastScanJson = json;
  jsonOut = json;
  return networks;
}

static void ledOn() {
  digitalWrite(STATUS_LED_PIN, LED_ACTIVE_LOW ? LOW : HIGH);
}

static void ledOff() {
  digitalWrite(STATUS_LED_PIN, LED_ACTIVE_LOW ? HIGH : LOW);
}

static void ledToggle() {
  digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
}

// Handler untuk set static IP
static void handleSetStaticIP() {
  if (!ensureAuth()) return;
  String ip = "";
  String gateway = "";
  String subnet = "";
  String dns1 = "";

  if (s_server->hasArg("ip") || s_server->hasArg("gateway")) {
    ip = s_server->arg("ip");
    gateway = s_server->arg("gateway");
    subnet = s_server->arg("subnet");
    dns1 = s_server->arg("dns1");
  } else if (s_server->hasArg("plain")) {
    String body = s_server->arg("plain");
    // Parse JSON: {"ip":"192.168.1.105","gateway":"192.168.1.1"}
    int ipStart = body.indexOf("\"ip\":\"") + 6;
    int ipEnd = body.indexOf("\"", ipStart);
    ip = body.substring(ipStart, ipEnd);
    
    int gwStart = body.indexOf("\"gateway\":\"") + 11;
    int gwEnd = body.indexOf("\"", gwStart);
    gateway = body.substring(gwStart, gwEnd);

    int subnetStart = body.indexOf("\"subnet\":\"");
    if (subnetStart != -1) {
      subnetStart += 10;
      int subnetEnd = body.indexOf("\"", subnetStart);
      subnet = body.substring(subnetStart, subnetEnd);
    }

    int dnsStart = body.indexOf("\"dns1\":\"");
    if (dnsStart != -1) {
      dnsStart += 8;
      int dnsEnd = body.indexOf("\"", dnsStart);
      dns1 = body.substring(dnsStart, dnsEnd);
    }
  } else {
    s_server->send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data provided\"}");
    return;
  }

  if (ip.length() >= WIFI_IP_MAX_LEN || gateway.length() >= WIFI_IP_MAX_LEN ||
      subnet.length() >= WIFI_IP_MAX_LEN || dns1.length() >= WIFI_IP_MAX_LEN) {
    s_server->send(400, "application/json", "{\"status\":\"error\",\"message\":\"IP/Gateway/Subnet/DNS terlalu panjang\"}");
    return;
  }

  if (ip.length() > 0 && gateway.length() > 0) {
    if (subnet.length() == 0) subnet = "255.255.255.0";
    saveStaticIP(ip, gateway, subnet, dns1, true);
    
    String response = "{\"status\":\"ok\",\"message\":\"Static IP saved. Reconnecting...\"}";
    s_server->send(200, "application/json", response);
    
    // Restart to apply new IP
    delay(1000);
    ESP.restart();
  } else {
    s_server->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid IP or gateway\"}");
  }
}
