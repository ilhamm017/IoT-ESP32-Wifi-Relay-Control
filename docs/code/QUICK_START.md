# Quick Start Guide - ESP32 WiFi Relay Control

## 🎯 5 Menit Setup

### Langkah 1: Upload ke ESP32 (2 menit)
```bash
# Buka folder project di terminal
cd d:\Documents\Project\IOT\IoT-ESP32-Wifi-Relay-Control

# Build & Upload
pio run -e esp32doit-devkit-v1 -t upload

# Monitor Serial
pio device monitor -b 115200
```

### Langkah 2: Setup WiFi Access Point (1 menit)
Setelah upload, di Serial Monitor akan muncul:
```
Access Point aktif:
SSID: ESP32-Setup
Password: 12345678
IP: 192.168.4.1
```

Di smartphone/laptop:
1. Cari WiFi "ESP32-Setup"
2. Connect dengan password `12345678`
3. Buka browser: `http://192.168.4.1`

### Langkah 3: Masukkan WiFi Credentials (1 menit)
1. Klik atau akses `/config` endpoint
2. Masukkan:
   - **SSID:** Nama WiFi rumah/kantor Anda
   - **Password:** Password WiFi Anda
3. Klik "Simpan & Koneksi"
4. ESP32 akan restart

### Langkah 4: Kontrol Relay (1 menit)
1. Cek Serial Monitor untuk IP address baru (misal: `192.168.1.105`)
2. Buka browser: `http://192.168.1.105`
3. Klik tombol untuk kontrol relay (jumlah mengikuti `RELAY_COUNT` di `src/main.cpp`, default 8).
   Contoh pin default: GPIO 32, 33, 25, 26, 27, 14, 16, 17.
4. Status akan update otomatis setiap 1 detik
Catatan: Default relay saat boot adalah ON (active-low). Ubah di `src/main.cpp` jika ingin default OFF.

---

## 📞 Serial Commands (Optional)

Ketik di Serial Monitor:

### Status System
```
status
```
Output:
```
=== Status ===
SSID: MyWiFi
WiFi Connected: Yes
IP Address: 192.168.1.105
```

### Reset WiFi
```
reset
```
- Hapus WiFi credentials
- Restart ke mode setup
- Kembali ke access point "ESP32-Setup"

---

## 🔧 Troubleshooting Cepat

| Masalah | Solusi |
|---------|--------|
| Serial Monitor kosong | Cek baud rate = 115200 |
| WiFi tidak connect | Cek SSID & password, coba reset |
| Web tidak bisa diakses | Pastikan device di WiFi yang sama |
| Relay tidak bergerak | Cek koneksi kabel sesuai `RELAY_PINS` di `src/main.cpp` |
| Access Point tidak muncul | Tunggu 1-2 menit, rescan WiFi |

---

## 🌐 API Cheatsheet

### Buka di Browser atau curl:

```bash
# Halaman kontrol (HTML interface)
http://192.168.1.105/

# Nyalakan Relay 1
http://192.168.1.105/1/on

# Matikan Relay 1
http://192.168.1.105/1/off

# Cek status Relay 1
http://192.168.1.105/1/status

# Relay N (N = 1..8 default)
http://192.168.1.105/1/on
http://192.168.1.105/1/off
http://192.168.1.105/1/status

# Contoh relay 8
http://192.168.1.105/8/on
```

### Via curl (untuk automation):
```bash
# Turn on relay 1
curl "http://192.168.1.105/1/on"

# Get status
curl "http://192.168.1.105/1/status"
```

---

## 🔌 Koneksi Hardware

**Minimal wiring untuk testing:**
```
ESP32 (3.3V) ──────→ Relay Module VCC
ESP32 GND    ──────→ Relay Module GND
ESP32 GPIO32 ──────→ Relay Module CH1 (Signal)
ESP32 GPIO33 ──────→ Relay Module CH2 (Signal)
ESP32 GPIO25 ──────→ Relay Module CH3 (Signal)
ESP32 GPIO26 ──────→ Relay Module CH4 (Signal)
ESP32 GPIO27 ──────→ Relay Module CH5 (Signal)
ESP32 GPIO14 ──────→ Relay Module CH6 (Signal)
ESP32 GPIO16 ──────→ Relay Module CH7 (Signal)
ESP32 GPIO17 ──────→ Relay Module CH8 (Signal)

Relay Module COM1 ──→ Perangkat yang ingin dikontrol (fase)
Relay Module COM2 ──→ Perangkat yang ingin dikontrol (fase)
```
Catatan: Sesuaikan pin di `src/main.cpp` jika wiring berbeda atau jumlah relay tidak 8.

---

## 📱 Mengakses dari Smartphone

1. Connect smartphone ke WiFi yang sama dengan ESP32
2. Buka browser smartphone
3. Akses: `http://<IP_ADDRESS_ESP32>`
4. Bookmark untuk akses cepat

---

## 💡 Tips & Tricks

### Mendapatkan IP Address ESP32
**Via Serial Monitor:**
```
Saat connected, lihat baris:
"IP Address: 192.168.1.XXX"
```

**Via Router Admin Panel:**
1. Login ke router (misal: `192.168.1.1`)
2. Cari "Connected Devices" atau "DHCP Clients"
3. Cari perangkat dengan nama "ESP32"

### Setting Static IP
Tambahkan di `main.cpp` sebelum `WiFi.begin()`:
```cpp
IPAddress ip(192, 168, 1, 105);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
WiFi.config(ip, gateway, subnet);
```

### Custom Relay Names
Edit di `src/WebHandlers.cpp` → `handleRoot()` untuk mengubah label tombol. Karena tombol dibuat dinamis, gunakan array nama dan ambil berdasarkan index relay.
```cpp
static const char* RELAY_LABELS[] = {
  "Lampu Ruang Tamu", "Lampu Kamar", "Pompa", "Kipas"
};
static const size_t RELAY_LABEL_COUNT = sizeof(RELAY_LABELS) / sizeof(RELAY_LABELS[0]);

// Di dalam loop pembuatan tombol:
String label = (i < RELAY_LABEL_COUNT) ? String(RELAY_LABELS[i]) : ("Saklar " + idx);
html += "<button class='on' onclick='fetch(\"/" + idx + "/on\")'>" + label + " ON</button>";
```

### Enable Debug Logging
Tambahkan di `setup()`:
```cpp
Serial.setDebugOutput(true);
```

---

## ⚡ Automation Examples

### Via Python (untuk home automation):
```python
import requests
import time

ESP32_IP = "192.168.1.105"

# Turn on relay 1
requests.get(f"http://{ESP32_IP}/1/on")
time.sleep(2)

# Check status
status = requests.get(f"http://{ESP32_IP}/1/status").text
print(f"Relay 1 status: {status}")

# Turn off after 5 seconds
time.sleep(5)
requests.get(f"http://{ESP32_IP}/1/off")
```

### Via Node.js:
```javascript
const fetch = require('node-fetch');

const ESP32_IP = "192.168.1.105";

async function controlRelay() {
    // Turn on
    await fetch(`http://${ESP32_IP}/1/on`);
    
    // Wait 5 seconds
    await new Promise(r => setTimeout(r, 5000));
    
    // Check status
    const status = await fetch(`http://${ESP32_IP}/1/status`).then(r => r.text());
    console.log(`Relay 1: ${status}`);
    
    // Turn off
    await fetch(`http://${ESP32_IP}/1/off`);
}

controlRelay();
```

---

## 🚀 Deployment Checklist

- [ ] Upload code ke ESP32
- [ ] Verify serial monitor output
- [ ] Setup WiFi via access point
- [ ] Verify WiFi connection
- [ ] Test semua relay yang digunakan (1..`RELAY_COUNT`)
- [ ] Verify web interface
- [ ] Check status endpoint responses
- [ ] Test dari smartphone
- [ ] Document IP address untuk reference

---

## ❓ FAQ

**Q: Berapa lama battery bisa tahan?**  
A: Tergantung relay usage. Standby current ~80mA. Dengan battery 5000mAh, ~60 jam.

**Q: Bisa kontrol dari internet (cloud)?**  
A: Saat ini hanya lokal WiFi. Perlu MQTT broker atau cloud integration untuk akses jarak jauh.

**Q: Apa relay maximum rating?**  
A: Tergantung relay module. Biasanya 10A @ 250VAC atau 5A @ 30VDC.

**Q: Bisa trigger via webhook?**  
A: Ya, dengan menambahkan endpoint yang trigger dari IFTTT atau similar.

---

**Quick Start v1.0**  
**Last Updated: 4 Februari 2026**
