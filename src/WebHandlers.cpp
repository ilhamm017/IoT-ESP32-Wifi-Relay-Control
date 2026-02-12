#include "WebHandlers.h"

#include <Arduino.h>
#include <WiFi.h>

#include "WiFiStorage.h"

static WebServer* s_server = nullptr;
static const int* s_relayPins = nullptr;
static size_t s_relayCount = 0;
static const char* AUTH_USER = "admin";
static const char* AUTH_PASS = "admin123";
static const char* AP_SSID   = "ESP32-Setup";
static const char* AP_PASS   = "12345678";
static String s_lastScanJson;

static void handleConfig();
static void handleSave();
static void handleRoot();
static void handleRelayOn(size_t index);
static void handleRelayOff(size_t index);
static void handleRelayStatus(size_t index);
static void handleScanWiFi();
static void handleSetStaticIP();
static bool ensureAuth();
static String jsonEscape(const String& input);
static int runWiFiScan(String& jsonOut, bool forceStaOnly);
static void ledOn();
static void ledOff();
static void ledToggle();
static bool isRelayIndexValid(size_t index);
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

void setupControlRoutes(WebServer& server, const int* relayPins, size_t relayCount) {
  s_server = &server;
  s_relayPins = relayPins;
  s_relayCount = relayCount;
  server.on("/", handleRoot);
  for (size_t i = 0; i < relayCount; i++) {
    const size_t index = i;
    String base = "/" + String(index + 1);
    server.on(base + "/on", [index]() { handleRelayOn(index); });
    server.on(base + "/off", [index]() { handleRelayOff(index); });
    server.on(base + "/status", [index]() { handleRelayStatus(index); });
  }
}

void setupAdditionalRoutes(WebServer& server) {
  s_server = &server;
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
  String html;
  html.reserve(1800 + (s_relayCount * 220));
  html = "<html>"
    "<head>"
    "<title>Relay Control</title>"
    "<style>"
    "body { font-family: Arial; margin: 24px; }"
    ".container { max-width: 720px; margin: 0 auto; }"
    "table { width: 100%; border-collapse: collapse; margin-top: 16px; }"
    "th, td { padding: 8px 10px; border-bottom: 1px solid #ddd; text-align: left; }"
    "button { padding: 6px 12px; margin-right: 6px; cursor: pointer; }"
    ".status { font-weight: bold; }"
    "</style>"
    "</head>"
    "<body>"
    "<div class='container'>"
    "<h1>Kontrol Relay</h1>";

  if (!s_relayPins || s_relayCount == 0) {
    html += "<p>Tidak ada relay yang terdaftar.</p>";
  } else {
    html += "<table>"
            "<thead><tr><th>Relay</th><th>Kontrol</th><th>Status</th></tr></thead>"
            "<tbody>";
    for (size_t i = 0; i < s_relayCount; i++) {
      String idx = String(i + 1);
      html += "<tr>";
      html += "<td>Saklar " + idx + "</td>";
      html += "<td>";
      html += "<button onclick='fetch(\"/" + idx + "/on\")'>ON</button>";
      html += "<button onclick='fetch(\"/" + idx + "/off\")'>OFF</button>";
      html += "</td>";
      html += "<td class='status' id='status" + idx + "'>Loading...</td>";
      html += "</tr>";
    }
    html += "</tbody></table>";
    html += "<script>";
    html += "function updateStatus() {";
    for (size_t i = 0; i < s_relayCount; i++) {
      String idx = String(i + 1);
      html += "fetch('/" + idx + "/status').then(r => r.text()).then(data => {";
      html += "document.getElementById('status" + idx + "').textContent = data;});";
    }
    html += "}";
    html += "updateStatus();";
    html += "setInterval(updateStatus, 1000);";
    html += "</script>";
  }

  html += "</div></body>"
          "</html>";

  s_server->send(200, "text/html", html);
}

static bool isRelayIndexValid(size_t index) {
  return s_relayPins && index < s_relayCount;
}

static void handleRelayOn(size_t index) {
  if (!isRelayIndexValid(index)) {
    s_server->send(404, "text/plain", "Relay tidak ditemukan");
    return;
  }
  const int pin = s_relayPins[index];
  digitalWrite(pin, LOW);  // Relay ON (active low)
  Serial.print("Relay ");
  Serial.print(index + 1);
  Serial.println(" ON");
  s_server->send(200, "text/plain", "Relay " + String(index + 1) + " ON");
}

static void handleRelayOff(size_t index) {
  if (!isRelayIndexValid(index)) {
    s_server->send(404, "text/plain", "Relay tidak ditemukan");
    return;
  }
  const int pin = s_relayPins[index];
  digitalWrite(pin, HIGH);  // Relay OFF (active low)
  Serial.print("Relay ");
  Serial.print(index + 1);
  Serial.println(" OFF");
  s_server->send(200, "text/plain", "Relay " + String(index + 1) + " OFF");
}

static void handleRelayStatus(size_t index) {
  if (!isRelayIndexValid(index)) {
    s_server->send(404, "text/plain", "Relay tidak ditemukan");
    return;
  }
  const int pin = s_relayPins[index];
  String status = digitalRead(pin) == LOW ? "ON" : "OFF";
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
