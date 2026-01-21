#include "WebHandlers.h"

#include <Arduino.h>
#include <WiFi.h>

#include "WiFiStorage.h"

static WebServer* s_server = nullptr;
static int s_relayPin1 = -1;
static int s_relayPin2 = -1;

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
}

// Handler untuk halaman konfigurasi (dengan WiFi scan & static IP)
static void handleConfig() {
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
    "<button onclick='scanWiFi()'>Scan Jaringan WiFi</button>"
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
    "<h3>⚙️ Konfigurasi IP Static (Optional)</h3>"
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
    
    "function scanWiFi() {"
    "  var btn = event.target;"
    "  btn.disabled = true;"
    "  btn.textContent = 'Scanning...';"
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
    "      btn.disabled = false;"
    "      btn.textContent = 'Scan Ulang';"
    "    })"
    "    .catch(err => {"
    "      alert('Scan gagal: ' + err);"
    "      btn.disabled = false;"
    "      btn.textContent = 'Scan Jaringan WiFi';"
    "    });"
    "}"
    
    "document.getElementById('ssidList').addEventListener('change', function() {"
    "  document.getElementById('ssidInput').value = this.value;"
    "});"
    
    "document.getElementById('configForm').addEventListener('submit', function(e) {"
    "  var ip = document.getElementById('staticIp').value;"
    "  var gateway = document.getElementById('gateway').value;"
    "  var useStatic = document.getElementById('useStatic').checked;"
    "  if (useStatic && (!ip || !gateway)) {"
    "    alert('Masukkan IP dan Gateway jika menggunakan Static IP');"
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
  if (s_server->hasArg("ssid") && s_server->hasArg("password")) {
    String newSsid = s_server->arg("ssid");
    String newPassword = s_server->arg("password");
    bool useStatic = s_server->hasArg("useStatic");
    String staticIp = s_server->arg("staticIp");
    String gateway = s_server->arg("gateway");

    // Save WiFi credentials
    saveWiFiCredentials(newSsid, newPassword);

    // Save static IP if enabled
    if (useStatic && staticIp.length() > 0 && gateway.length() > 0) {
      saveStaticIP(staticIp, gateway, true);
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
  Serial.println("Scanning available WiFi networks...");
  
  int networks = WiFi.scanNetworks();
  
  String json = "{\"networks\":[";
  
  for (int i = 0; i < networks; i++) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + WiFi.RSSI(i) + "}";
  }
  
  json += "]}";
  
  s_server->send(200, "application/json", json);
  Serial.print("Found ");
  Serial.print(networks);
  Serial.println(" networks");
}

// Handler untuk set static IP
static void handleSetStaticIP() {
  if (s_server->hasArg("plain")) {
    String body = s_server->arg("plain");
    
    // Parse JSON: {"ip":"192.168.1.105","gateway":"192.168.1.1"}
    int ipStart = body.indexOf("\"ip\":\"") + 6;
    int ipEnd = body.indexOf("\"", ipStart);
    String ip = body.substring(ipStart, ipEnd);
    
    int gwStart = body.indexOf("\"gateway\":\"") + 11;
    int gwEnd = body.indexOf("\"", gwStart);
    String gateway = body.substring(gwStart, gwEnd);
    
    if (ip.length() > 0 && gateway.length() > 0) {
      saveStaticIP(ip, gateway, true);
      
      String response = "{\"status\":\"ok\",\"message\":\"Static IP saved. Reconnecting...\"}";
      s_server->send(200, "application/json", response);
      
      // Restart to apply new IP
      delay(1000);
      ESP.restart();
    } else {
      s_server->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid IP or gateway\"}");
    }
  } else {
    s_server->send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data provided\"}");
  }
}
