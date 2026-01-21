# Dokumentasi Program Kontrol Relay Wireless (WiFi) - ESP32

**Versi:** 2.0 (dengan fitur WiFi Scan, Static IP, Persistent Config)  
**Status:** ✅ Production Ready  
**Last Updated:** 22 Januari 2026

## 📋 Ringkasan Proyek

Program ini adalah sistem kontrol relay wireless berbasis **ESP32** yang memungkinkan pengguna untuk mengontrol 2 relay melalui koneksi WiFi menggunakan web server HTTP. Sistem ini dapat diakses melalui browser dengan interface web yang user-friendly, dilengkapi dengan fitur WiFi scanning, konfigurasi IP static, dan persistent configuration.

---

## 🎯 Fitur Utama

### 1. **Kontrol Relay via WiFi**
   - Mengendalikan 2 relay secara independen (GPIO 32 dan GPIO 33)
   - Interface web dengan tombol ON/OFF untuk setiap relay
   - Status real-time dari setiap relay yang ditampilkan di halaman web

### 2. **Konfigurasi WiFi Otomatis & Manual**
   - Mode Access Point (AP) otomatis ketika tidak terhubung ke WiFi
   - Interface konfigurasi web untuk input SSID dan password WiFi
   - Penyimpanan kredensial WiFi ke EEPROM (non-volatile storage)
   - **[NEW] Persistent Config** - Akses /config bahkan saat sudah connected

### 3. **WiFi Network Scanning** ✨ NEW
   - **Endpoint:** `/api/scan-wifi` - Scan jaringan WiFi yang tersedia
   - Menampilkan SSID dengan signal strength (dBm)
   - Dropdown select di form untuk mencegah salah ketik SSID
   - Auto-fill input field saat select SSID dari list

### 4. **Static IP Configuration** ✨ NEW
   - Set IP static saat setup WiFi
   - Input fields: SSID + Password + IP Address + Gateway
   - EEPROM expansion untuk simpan IP & gateway
   - Fallback ke DHCP jika konfigurasi invalid
   - Support untuk custom IP routing

### 5. **IP Setting via API** ✨ NEW
   - **Endpoint:** `POST /api/set-static-ip` atau `GET /api/set-static-ip?ip=...&gateway=...`
   - Ubah static IP tanpa web form
   - Accept JSON (POST): `{"ip":"192.168.1.105","gateway":"192.168.1.1"}`
   - Query (GET): `?ip=192.168.1.105&gateway=192.168.1.1`
   - Device restart otomatis; koneksi bisa pindah setelah apply

### 6. **Manajemen WiFi**
   - Koneksi otomatis ke WiFi yang tersimpan saat startup
   - Fallback ke AP mode jika koneksi gagal
   - Reset WiFi credentials via tombol BOOT atau serial command
   - Reconfigure WiFi kapan saja tanpa hardware reset

### 7. **Monitoring Sistem**
   - Serial monitor untuk debugging dan monitoring
   - Command line interface via serial: `status`, `reset`
   - Informasi real-time tentang status koneksi WiFi & IP address

### 8. **Reset Hardware**
   - Tombol BOOT (GPIO 0) dapat digunakan untuk reset WiFi credentials
   - Durasi: tekan selama 5 detik untuk trigger reset
   - Serial output indicator untuk confirmation

---

## 🔧 Spesifikasi Hardware

| Komponen | Detail |
|----------|--------|
| **Board** | ESP32 DoIT DevKit V1 |
| **Relay 1** | GPIO 32 |
| **Relay 2** | GPIO 33 |
| **Reset Button** | GPIO 0 (Tombol BOOT bawaan) |
| **Serial** | 115200 baud rate |
| **Web Server** | Port 80 (HTTP) |

### Logika Relay:
- **LOW (0)** = Relay ON (kontak tertutup)
- **HIGH (1)** = Relay OFF (kontak terbuka)

---
- **HIGH (1)** = Relay OFF (kontak terbuka)

---

## 📡 Arsitektur Sistem (v2.0)

```
┌─────────────────────────────────────────────────────┐
│              ESP32 WiFi Relay v2.0                  │
├─────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────┐  │
│  │     Web Server (Port 80, HTTP)               │  │
│  │  ┌───────────────────────────────────────┐   │  │
│  │  │   Configuration Routes                │   │  │
│  │  │  /config, /save                      │   │  │
│  │  │  /api/scan-wifi (NEW)                │   │  │
│  │  │  /api/set-static-ip (NEW)           │   │  │
│  │  └───────────────────────────────────────┘   │  │
│  │  ┌───────────────────────────────────────┐   │  │
│  │  │   Control Routes                     │   │  │
│  │  │  /, /1/on, /1/off, /2/on, /2/off   │   │  │
│  │  │  /1/status, /2/status              │   │  │
│  │  └───────────────────────────────────────┘   │  │
│  └──────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────┐  │
│  │      EEPROM Storage (512 bytes) - NEW       │  │
│  │  [0-31]:    SSID (32 bytes)                 │  │
│  │  [32-95]:   Password (64 bytes)             │  │
│  │  [96-111]:  Static IP (16 bytes) NEW       │  │
│  │  [112-127]: Gateway (16 bytes) NEW         │  │
│  │  [128]:     Flags (1 byte) NEW             │  │
│  │  [129-511]: Reserved                       │  │
│  └──────────────────────────────────────────────┘  │
├─────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────┐  │
│  │        GPIO Output                           │  │
│  │  ├─ Relay 1 (GPIO 32)                       │  │
│  │  └─ Relay 2 (GPIO 33)                       │  │
│  └──────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

---

## 📱 API Endpoints (v2.0 Updated)

### **Mode Konfigurasi** (Accessible di AP & STA mode)

| Endpoint | Method | Deskripsi | Response |
|----------|--------|-----------|----------|
| `/config` | GET | Halaman form konfigurasi WiFi (dengan scan & static IP) | HTML |
| `/api/scan-wifi` | GET | Scan jaringan WiFi yang tersedia | JSON |
| `/save` | POST | Simpan SSID, password, static IP ke EEPROM | HTML |
| `/api/set-static-ip` | GET/POST | Set static IP via API endpoint (query or JSON) | JSON |

**[NEW]** Perbedaan penting: `/config` dan `/save` sekarang accessible **bahkan saat WiFi sudah connected**, memungkinkan user untuk reconfigure tanpa reset!

### **Mode Kontrol** (Setelah WiFi terhubung)

| Endpoint | Method | Deskripsi | Response |
|----------|--------|-----------|----------|
| `/` | GET | Halaman kontrol utama dengan 4 tombol | HTML Interface |
| `/1/on` | GET | Nyalakan relay 1 | "Relay 1 ON" |
| `/1/off` | GET | Matikan relay 1 | "Relay 1 OFF" |
| `/2/on` | GET | Nyalakan relay 2 | "Relay 2 ON" |
| `/2/off` | GET | Matikan relay 2 | "Relay 2 OFF" |
| `/1/status` | GET | Ambil status relay 1 | "ON" atau "OFF" |
| `/2/status` | GET | Ambil status relay 2 | "ON" atau "OFF" |

---

## 🚀 Panduan Penggunaan

### **Startup Pertama Kali**

1. Upload program ke ESP32 menggunakan PlatformIO
2. Buka Serial Monitor (115200 baud)
3. Karena EEPROM kosong, ESP32 akan masuk mode AP otomatis
4. Serial Monitor akan menampilkan:
   ```
   === ESP32 Relay Control Server ===
   Memulai Access Point untuk konfigurasi...
   Access Point aktif:
   SSID: ESP32-Setup
   Password: 12345678
   IP: 192.168.4.1
   ```

### **Konfigurasi WiFi (Melalui Access Point)**

1. Cari WiFi "ESP32-Setup" di perangkat Anda
2. Hubungkan dengan password: `12345678`
3. Buka browser, akses: `http://192.168.4.1`
4. Klik menu `/config` atau langsung masuk halaman config
5. Masukkan:
   - SSID WiFi target
   - Password WiFi target
6. Klik "Simpan & Koneksi"
7. ESP32 akan restart dan otomatis terhubung ke WiFi Anda

### **Kontrol Relay (Setelah Terhubung)**

1. Cari IP address ESP32 di router Anda (atau lihat serial monitor)
2. Buka browser: `http://<IP_ADDRESS>`
3. Halaman web menampilkan:
   - 4 tombol: "Saklar 1 ON", "Saklar 1 OFF", "Saklar 2 ON", "Saklar 2 OFF"
   - Status real-time kedua relay (update setiap 1 detik)
4. Klik tombol untuk mengendalikan relay

### **Set Static IP via API**

- **GET (via URL):**  
  `http://<IP_ADDRESS>/api/set-static-ip?ip=192.168.1.105&gateway=192.168.1.1`
- **POST (JSON):**
  ```
  curl -X POST http://<IP_ADDRESS>/api/set-static-ip \
    -H "Content-Type: application/json" \
    -d "{\"ip\":\"192.168.1.105\",\"gateway\":\"192.168.1.1\"}"
  ```
- **Catatan:** device akan restart untuk apply; akses kembali dengan IP baru.

### **Kontrol via Serial Monitor**

- **Status System:**
  ```
  status
  ```
  Output:
  ```
  === Status ===
  SSID: [nama wifi terhubung]
  WiFi Connected: Yes
  IP Address: [IP address ESP32]
  ```

- **Reset WiFi Credentials:**
  ```
  reset
  ```
  - Menghapus SSID dan password dari EEPROM
  - Restart otomatis
  - Kembali ke mode AP setup

### **Reset Menggunakan Tombol BOOT**

1. Tekan tombol BOOT (berlabel "BOOT" di board ESP32)
2. Tahan selama 5 detik (jangan lepas)
3. Serial monitor akan menampilkan: `"Tombol BOOT ditekan selama 5 detik! Mereset WiFi..."`
4. ESP32 akan restart dengan EEPROM yang sudah dikosongkan
5. Kembali ke mode AP setup

---

## 📂 Struktur Kode

### **File: `src/main.cpp`**
**Fungsi Utama:**
- `setup()` - Inisialisasi GPIO, load WiFi credentials dari EEPROM
- `loop()` - Handle client HTTP requests, detect tombol reset, parse serial command
- `setupConfigAP()` - Setup WiFi Access Point untuk konfigurasi awal
- `connectToWiFi()` - Koneksi ke WiFi yang tersimpan

### **File: `src/WebHandlers.cpp`**
**Fungsi:**
- `setupConfigRoutes()` - Setup routes untuk konfigurasi WiFi
- `setupControlRoutes()` - Setup routes untuk kontrol relay
- `handleConfig()` - Tampilkan form konfigurasi WiFi (HTML)
- `handleSave()` - Proses penyimpanan kredensial WiFi
- `handleRoot()` - Tampilkan halaman kontrol relay utama
- `handleSwitch1On/Off()` - Kontrol relay 1
- `handleSwitch2On/Off()` - Kontrol relay 2
- `handleStatus1/2()` - Kirim status relay

### **File: `src/WiFiStorage.cpp`**
**Fungsi:**
- `loadWiFiCredentials()` - Baca SSID & password dari EEPROM
- `saveWiFiCredentials()` - Tulis SSID & password ke EEPROM
- `clearWiFiCredentials()` - Hapus isi EEPROM

---

## 💾 Penyimpanan EEPROM

Total: **512 bytes**

| Lokasi | Size | Isi |
|--------|------|-----|
| 0 - 31 | 32 bytes | SSID (null-terminated) |
| 32 - 95 | 64 bytes | Password (null-terminated) |
| 96 - 511 | 416 bytes | Reserved |

**Format:** String dalam C dengan null terminator (`\0`)

---

## 🔌 Diagram Koneksi

```
ESP32 Pin Layout:
    ┌──────────────────────┐
    │      ESP32 Board     │
    │                      │
    │ GPIO32 ──────→ Relay 1
    │ GPIO33 ──────→ Relay 2
    │ GPIO0  ←────── BOOT Button
    │ GND    ──────→ GND
    │ 3V3    ──────→ VCC
    │                      │
    └──────────────────────┘
         │
         │ WiFi (builtin)
         │
    ┌──────────────────┐
    │   WiFi Router    │
    │ 192.168.x.x      │
    └──────────────────┘
```

---

## ⚙️ Konfigurasi & Build

### **PlatformIO Configuration** (`platformio.ini`)
```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
```

### **Build Command:**
```bash
pio run -e esp32doit-devkit-v1
```

### **Flash Command:**
```bash
pio run -e esp32doit-devkit-v1 -t upload
```

### **Monitor Serial:**
```bash
pio device monitor -b 115200
```

---

## 🔍 Flow Diagram - Startup

```
START
  ↓
Initialize Serial (115200 baud)
  ↓
Setup GPIO:
  - Relay 1 (GPIO32) = HIGH (OFF)
  - Relay 2 (GPIO33) = HIGH (OFF)
  - Reset Button (GPIO0) = INPUT_PULLUP
  ↓
Load WiFi Credentials from EEPROM
  ↓
Check SSID empty?
  ├─ YES → Start Access Point "ESP32-Setup"
  │        Setup Config Routes (/config, /save)
  │        Wait for user WiFi input
  │
  └─ NO  → Attempt Connect to WiFi
           Connection success?
           ├─ YES → Setup Control Routes
           │        (/1/on, /1/off, /2/on, /2/off, /1/status, /2/status)
           │        Print IP Address
           │
           └─ NO  → Start Access Point "ESP32-Setup"
                    Setup Config Routes
  ↓
LOOP:
  - Handle HTTP Client Requests
  - Check Reset Button (5 sec press)
  - Check Serial Commands (reset, status)
```

---

## 🌐 Flow Diagram - Web Interface

### **Configuration Flow:**
```
Browser Access: 192.168.4.1
    ↓
Request: GET /config
    ↓
Receive: HTML Form (SSID, Password input)
    ↓
User input & Submit (POST /save)
    ↓
Server saves to EEPROM
    ↓
ESP32 Restart
    ↓
Load credentials from EEPROM
    ↓
Connect to WiFi
    ↓
Success → Show Control Page
```

### **Control Flow:**
```
Browser Access: <ESP32_IP>
    ↓
Request: GET /
    ↓
Receive: HTML with 4 buttons + Status display
    ↓
JavaScript fetch() status every 1 second
    ↓
User click button
    ↓
Fetch GET /1/on (or /1/off, /2/on, /2/off)
    ↓
Server digitalWrite(GPIO, LOW/HIGH)
    ↓
Return Response: "Relay X ON/OFF"
    ↓
Status update via /X/status endpoints
    ↓
UI display status update
```

---

## 📊 Status Flow

```
┌──────────────────────────────────────────┐
│       WiFi Connection States             │
├──────────────────────────────────────────┤
│                                          │
│  Power On / Restart                      │
│      ↓                                   │
│  Load EEPROM Credentials                 │
│      ↓                                   │
│  SSID Empty?                             │
│   /    \                                 │
│  YES   NO                                │
│  |      └─ Attempt WiFi Connect          │
│  |          ├─ Success → [CONNECTED]     │
│  |          └─ Fail → [AP MODE]          │
│  |                                       │
│  └─ [AP MODE] (Config Mode)              │
│      └─ Waiting for WiFi input           │
│          └─ User submit → Save & Restart │
│              └─ [CONNECTED]              │
│                                          │
└──────────────────────────────────────────┘
```

---

## 🛠️ Troubleshooting

| Masalah | Penyebab | Solusi |
|---------|---------|--------|
| ESP32 tidak bisa connect ke WiFi | Password salah atau SSID tidak ditemukan | Reset credentials dan reconfigure |
| ESP32 stuck di Access Point | SSID sudah dihapus atau WiFi tidak tersedia | Tunggu koneksi atau use manual reset |
| Web interface tidak bisa diakses | Device tidak di WiFi yang sama | Cek IP address ESP32 di router |
| Relay tidak bergerak | GPIO tidak terkoneksi dengan relay module | Cek koneksi pin 32 & 33 |
| Serial monitor tidak menampilkan output | Baud rate salah | Gunakan 115200 baud |

---

## 📝 Catatan Penting

1. **Keamanan WiFi Access Point:** Password default "12345678" digunakan untuk setup mode. Ini hanya untuk konfigurasi awal dan dapat diubah di kode jika diperlukan.

2. **EEPROM Lifespan:** ESP32 EEPROM memiliki batasan write cycle (~100,000 kali). Hindari save credentials terlalu sering.

3. **Power Supply:** Pastikan power supply cukup untuk ESP32 + relay module (minimal 500mA).

4. **Relay Type:** Kode ini mengasumsikan relay active-LOW (ON saat GPIO LOW). Jika menggunakan relay active-HIGH, sesuaikan logika di WebHandlers.cpp.

5. **Serial Monitor:** Sangat berguna untuk debugging. Selalu monitor serial output saat development.

---

## 🎓 Pengembangan Lebih Lanjut

### Fitur yang bisa ditambahkan:
- [ ] Authentication/Login untuk web interface
- [ ] MQTT support untuk integrasi dengan smart home systems
- [ ] Scheduling/Timer untuk relay otomatis
- [ ] Database untuk logging aktivitas relay
- [ ] Sensor input (temperature, humidity) untuk monitoring
- [ ] Mobile app / Dashboard yang lebih advanced
- [ ] OTA (Over-The-Air) firmware update
- [ ] TLS/HTTPS untuk keamanan komunikasi
- [ ] Telegram bot integration untuk notifikasi

---

## 📞 Informasi Kontak & License

**Proyek:** IoT ESP32 WiFi Relay Control  
**Hardware:** ESP32 DoIT DevKit V1  
**Framework:** Arduino (PlatformIO)  
**Status:** Production Ready ✓

---

**Dokumentasi dibuat:** 22 Januari 2026
