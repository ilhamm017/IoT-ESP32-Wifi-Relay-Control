# Dokumentasi Teknis - ESP32 WiFi Relay Control

## 📌 Daftar Isi
1. [Analisis Kode Detail](#analisis-kode-detail)
2. [Function Reference](#function-reference)
3. [Data Structure](#data-structure)
4. [Protokol HTTP](#protokol-http)
5. [Error Handling](#error-handling)
6. [Performance Notes](#performance-notes)

---

## Analisis Kode Detail

### **main.cpp - File Utama**

#### Global Variables
```cpp
WebServer server(80);              // Web server di port 80
String ssid = "";                  // SSID WiFi yang tersimpan
String password = "";              // Password WiFi yang tersimpan
bool wifiConnected = false;        // Status koneksi WiFi
```

#### Function: setupConfigAP()
```cpp
void setupConfigAP() {
  // 1. Set WiFi mode ke AP (Access Point)
  // 2. Buat AP dengan SSID "ESP32-Setup" dan password "12345678"
  // 3. Tampilkan IP address AP (default: 192.168.4.1)
  // 4. Setup routes untuk konfigurasi
  // 5. Start web server
}
```
**Kapan dipanggil:**
- Saat startup jika SSID kosong
- Saat koneksi WiFi gagal
- Saat reset WiFi credentials

#### Function: connectToWiFi()
```cpp
void connectToWiFi() {
  // 1. Cek apakah SSID ada
  // 2. Set WiFi mode ke STA (Station)
  // 3. Koneksi ke WiFi dengan credentials
  // 4. Wait hingga 10 detik (20 * 500ms) untuk koneksi
  // 5. Jika success:
  //    - Setup control routes
  //    - Start server
  //    - Print IP address
  // 6. Jika gagal: setup AP mode
}
```

#### Function: setup()
```cpp
void setup() {
  // 1. Initialize Serial (115200 baud)
  // 2. Setup GPIO pins:
  //    - RELAY_PINS[] (default: 32, 33, 25, 26, 27, 14, 16, 17)
  //    - Semua relay di-set OUTPUT, initial LOW (ON)
  //    - RESET_BUTTON_PIN (0) sebagai INPUT_PULLUP
  // 3. Load WiFi credentials dari EEPROM
  // 4. Attempt WiFi connection
}
```
**Catatan:** 
- `delay(1000)` di awal untuk stabilisasi serial
- Relay initial state LOW berarti ON

#### Function: loop()
```cpp
void loop() {
  // 1. Handle incoming HTTP requests
  server.handleClient();
  
  // 2. Detect reset button press (5 seconds)
  // 3. Parse serial commands:
  //    - "reset": clear WiFi credentials & restart
  //    - "status": show current WiFi status
}
```

---

### **WebHandlers.cpp - HTTP Request Handlers**

#### Static Variables (Private to module)
```cpp
static WebServer* s_server = nullptr;    // Pointer ke WebServer instance
static const int* s_relayPins = nullptr; // Array pin relay
static size_t s_relayCount = 0;          // Jumlah relay terdaftar
```

#### Function: setupConfigRoutes()
```cpp
void setupConfigRoutes(WebServer& server) {
  // Register HTTP routes untuk setup mode:
  // - GET  /config  → handleConfig()
  // - POST /save    → handleSave()
}
```

#### Function: setupControlRoutes()
```cpp
void setupControlRoutes(WebServer& server, const int* relayPins, size_t relayCount) {
  // Register HTTP routes untuk control mode:
  // - GET /           → handleRoot()
  // - GET /N/on       → handleRelayOn(index)
  // - GET /N/off      → handleRelayOff(index)
  // - GET /N/status   → handleRelayStatus(index)
}
```

#### Function: handleConfig()
```cpp
void handleConfig() {
  // Return HTML form dengan:
  // - Input field: SSID
  // - Input field: Password
  // - Submit button (POST ke /save)
}
```
**Response Type:** `text/html`  
**Response Code:** `200 OK`

#### Function: handleSave()
```cpp
void handleSave() {
  // 1. Cek apakah "ssid" dan "password" parameter ada
  // 2. Ambil nilai dari parameter
  // 3. Save ke EEPROM via saveWiFiCredentials()
  // 4. Return success HTML
  // 5. Delay 2 detik
  // 6. Restart ESP32 (ESP.restart())
}
```
**Parameter:** 
- `ssid` (string) - SSID WiFi
- `password` (string) - Password WiFi

**Response Type:** `text/html`  
**Response Code:** `200 OK`

#### Function: handleRoot()
```cpp
void handleRoot() {
  // Return HTML dengan:
  // - Title: "Kontrol Relay"
  // - Tombol dinamis sesuai jumlah relay (default 8)
  // - Status display untuk semua relay
  // - JavaScript:
  //   * Fetch /N/status untuk tiap relay setiap 1 detik
  //   * Update UI status dinamis
}
```
**Response Type:** `text/html`  
**Response Code:** `200 OK`

#### Function: handleRelayOn() & handleRelayOff()
```cpp
void handleRelayOn(size_t index) {
  // Ambil pin dari array s_relayPins[index]
  // Set LOW (relay active-low)
  // Print ke serial: "Relay N ON"
  // Return: "Relay N ON"
}

void handleRelayOff(size_t index) {
  // Ambil pin dari array s_relayPins[index]
  // Set HIGH (relay inactive)
  // Print ke serial: "Relay N OFF"
  // Return: "Relay N OFF"
}
```

#### Function: handleRelayStatus()
```cpp
void handleRelayStatus(size_t index) {
  // Read pin s_relayPins[index]
  // Return "ON" jika LOW, "OFF" jika HIGH
}
```
**Response Type:** `text/plain`  
**Response Code:** `200 OK`

---

### **WiFiStorage.cpp - EEPROM Management**

#### EEPROM Memory Map
```
EEPROM Size: 512 bytes

Bytes 0-31:    SSID (max 31 chars + null terminator)
Bytes 32-95:   Password (max 63 chars + null terminator)
Bytes 96-511:  Reserved/Unused
```

#### Function: loadWiFiCredentials()
```cpp
void loadWiFiCredentials(String& ssid, String& password) {
  // 1. Begin EEPROM with size 512
  // 2. Loop 32 times untuk read SSID:
  //    - Read byte dari EEPROM[0+i]
  //    - Jika '\0' (null) → break
  //    - Else → append ke string ssid
  // 3. Loop 64 times untuk read Password:
  //    - Read byte dari EEPROM[32+i]
  //    - Jika '\0' → break
  //    - Else → append ke string password
  // 4. Print ke serial: "WiFi credentials loaded from EEPROM"
}
```
**Return:** ssid dan password di-populate via reference

#### Function: saveWiFiCredentials()
```cpp
void saveWiFiCredentials(const String& ssid, const String& password) {
  // 1. Begin EEPROM
  // 2. Clear bytes 0-95 dengan write 0
  // 3. Loop untuk write SSID:
  //    - EEPROM[0+i] = ssid[i]
  // 4. EEPROM[32] = '\0' (null terminator untuk SSID)
  // 5. Loop untuk write Password:
  //    - EEPROM[32+i] = password[i]
  // 6. EEPROM[96] = '\0' (null terminator untuk Password)
  // 7. EEPROM.commit() - save ke flash
  // 8. Print ke serial: "WiFi credentials saved to EEPROM"
}
```
**Parameters:**
- `ssid` (const String&) - SSID WiFi (max 31 chars)
- `password` (const String&) - Password WiFi (max 63 chars)

#### Function: clearWiFiCredentials()
```cpp
void clearWiFiCredentials() {
  // 1. Begin EEPROM
  // 2. Loop 96 times (0-95):
  //    - Write 0 ke semua EEPROM cells
  // 3. EEPROM.commit()
}
```
**Effect:** Menghapus SSID dan Password dari EEPROM

---

## Function Reference

| File | Function | Input | Output | Purpose |
|------|----------|-------|--------|---------|
| main.cpp | setupConfigAP() | - | void | Start AP mode untuk WiFi setup |
| main.cpp | connectToWiFi() | - | void | Connect ke WiFi yang tersimpan |
| main.cpp | setup() | - | void | Initialize hardware & load configs |
| main.cpp | loop() | - | void | Main loop: handle requests & button |
| WebHandlers.cpp | setupConfigRoutes(server) | WebServer& | void | Setup routes: /config, /save |
| WebHandlers.cpp | setupControlRoutes(server, relayPins, relayCount) | WebServer&, const int*, size_t | void | Setup routes: /, /N/on, /N/off, /N/status |
| WebHandlers.cpp | handleConfig() | - | void | Show WiFi config form |
| WebHandlers.cpp | handleSave() | - | void | Save WiFi credentials |
| WebHandlers.cpp | handleRoot() | - | void | Show relay control page |
| WebHandlers.cpp | handleRelayOn/Off(index) | size_t | void | Control relay |
| WebHandlers.cpp | handleRelayStatus(index) | size_t | void | Return relay status |
| WiFiStorage.cpp | loadWiFiCredentials(ssid, pw) | String&, String& | void | Load from EEPROM |
| WiFiStorage.cpp | saveWiFiCredentials(ssid, pw) | const String&, const String& | void | Save to EEPROM |
| WiFiStorage.cpp | clearWiFiCredentials() | - | void | Erase EEPROM |

---

## Data Structure

### WiFi State Machine
```
States:
1. STARTUP
   ├─ Load EEPROM
   └─ Branch based on SSID content

2. AP_MODE (Config Mode)
   ├─ SSID: "ESP32-Setup"
   ├─ Password: "12345678"
   ├─ IP: 192.168.4.1/24
   └─ Routes: /config, /save

3. STA_MODE (Connected)
   ├─ Connected to configured WiFi
   ├─ IP: Assigned by DHCP
   └─ Routes: /, /1..8/on, /1..8/off, /1..8/status

4. RETRY_AP_MODE (Connection Failed)
   └─ Same as AP_MODE
```

### HTTP Request/Response Examples

Catatan: endpoint relay bersifat dinamis; ganti N sesuai `RELAY_COUNT` (default 1..8).

#### Request: Configure WiFi
```http
POST /save HTTP/1.1
Host: 192.168.4.1
Content-Type: application/x-www-form-urlencoded

ssid=MyWiFi&password=MyPassword123
```

#### Response: Configuration Success
```http
HTTP/1.1 200 OK
Content-Type: text/html

<html>
<head><title>Disimpan</title></head>
<body>
<h1>Konfigurasi Disimpan!</h1>
<p>Menghubung ke WiFi...</p>
<p>Refresh halaman dalam 10 detik</p>
<script>setTimeout(function(){ location.reload(); }, 10000);</script>
</body>
</html>
```

#### Request: Turn on Relay 1
```http
GET /1/on HTTP/1.1
Host: 192.168.1.100
```

#### Response: Relay Control
```http
HTTP/1.1 200 OK
Content-Type: text/plain

Relay 1 ON
```

#### Request: Get Status
```http
GET /1/status HTTP/1.1
Host: 192.168.1.100
```

#### Response: Status
```http
HTTP/1.1 200 OK
Content-Type: text/plain

ON
```

---

## Protokol HTTP

### HTTP Methods Used
- **GET** - Untuk retrieve data (status, halaman) dan simple actions (relay control)
- **POST** - Untuk submit form (WiFi credentials)

### Content Types
- `text/html` - HTML pages dan forms
- `text/plain` - Simple text responses (status, control feedback)
- `application/x-www-form-urlencoded` - Form data dari konfigurasi

### Response Codes
- `200 OK` - Request berhasil
- `404 Not Found` - Relay index tidak valid atau route tidak terdaftar

### CORS
- No CORS headers implemented (same-origin access only)

---

## Error Handling

### WiFi Connection Error
```
Scenario: ESP32 mencoba connect ke WiFi tapi gagal
Flow:
1. connectToWiFi() attempt koneksi
2. Loop max 20x dengan delay 500ms = max 10 detik wait
3. Jika masih tidak connected → fallback ke setupConfigAP()
4. User reconfigure WiFi
```

### EEPROM Read Error
```
Scenario: EEPROM rusak atau tidak terisi
Flow:
1. loadWiFiCredentials() returns empty string
2. ssid.length() == 0
3. setupConfigAP() triggered
4. User dapat setup WiFi baru
```

### Serial Input Handling
```cpp
if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "reset") { /* ... */ }
    else if (command == "status") { /* ... */ }
    // Undefined commands simply ignored
}
```

### Button Press Detection
```cpp
if (digitalRead(RESET_BUTTON_PIN) == LOW) {  // Button pressed
    if (!buttonPressed) {
        buttonPressStart = millis();  // Catat waktu press
        buttonPressed = true;
    }
    
    unsigned long pressDuration = millis() - buttonPressStart;
    if (pressDuration >= 5000) {  // 5 detik telah berlalu
        ESP.restart();
    }
} else {  // Button released
    buttonPressed = false;  // Reset flag
}
```

---

## Performance Notes

### Memory Usage
- **SSID max:** 31 bytes
- **Password max:** 63 bytes
- **String overhead:** ~24 bytes per String object
- **Estimated RAM:** ~5-10 KB untuk semua variables

### Timing
- **WiFi connection attempt:** 10 detik max
- **Serial baud rate:** 115200 (optimal untuk ESP32)
- **Status update interval (JS):** 1 detik (dari browser)
- **EEPROM write:** ~1-2 ms per byte
- **Reset button response:** 5 detik

### Network Performance
- **HTTP server:** Blocking (handleClient waits untuk complete transaction)
- **No timeout implemented:** Long connections possible
- **Auth terbatas:** Basic Auth hanya untuk konfigurasi; kontrol relay tanpa auth

### Scalability Limitations
- **Max 1 relay control request:** Tidak support concurrent requests
- **EEPROM wear:** Limited write cycles (~100K per cell)
- **WiFi range:** Standard ESP32 (~100m indoor)

---

## Security Notes

### Current Implementation
1. **Default AP Password:** "12345678" (weak, hardcoded)
2. **No encryption:** HTTP only, no HTTPS
3. **Auth terbatas:** Basic Auth hanya untuk konfigurasi; kontrol relay tanpa auth
4. **No rate limiting:** Susceptible to brute force atau DoS
5. **Credentials visible in code:** Password visible di source

### Recommendations for Production
- [ ] Use WPA2 encryption untuk AP mode
- [ ] Implement JWT atau session tokens
- [ ] Add rate limiting per IP
- [ ] Use HTTPS/TLS jika possible
- [ ] Implement proper password hashing
- [ ] Add audit logging

---

**Dokumentasi Teknis v1.0**  
**Last Updated: 4 Februari 2026**
