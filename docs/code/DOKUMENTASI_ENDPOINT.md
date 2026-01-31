# Dokumentasi Endpoint - ESP32 WiFi Relay Control

Dokumen ini khusus endpoint HTTP. Untuk panduan pengguna, lihat `../../PANDUAN_PENGGUNA.md`.

---

## Base URL & Autentikasi

- AP mode: `http://192.168.4.1`
- STA mode: `http://<IP_ESP32>`
- **Endpoint konfigurasi memakai Basic Auth (default `admin` / `admin123`)**

---

## Endpoint Konfigurasi (wajib Basic Auth)

| Endpoint | Method | Deskripsi | Response |
|---|---|---|---|
| `/config` | GET | Halaman konfigurasi WiFi | HTML |
| `/save` | POST | Simpan SSID, password, static IP (IP/Gateway/Subnet/DNS) | HTML |
| `/api/scan-wifi` | GET | Scan WiFi, return list SSID | JSON |
| `/api/set-static-ip` | GET/POST | Simpan static IP (query atau JSON) | JSON |

### /api/scan-wifi (GET)

Response:
```json
{"networks":[{"ssid":"MyWiFi","rssi":-53}]}
```

Catatan:
- Endpoint ini melakukan scan dengan mode AP+STA sehingga koneksi HTTP tetap stabil (AP tetap aktif).
- Saat boot, firmware menjalankan scan awal mode STA-only (AP bisa restart sebentar) agar hasil lebih lengkap.
- LED indikator akan berkedip selama proses scan.

### /api/set-static-ip (GET)

Contoh:
```
http://<IP_ESP32>/api/set-static-ip?ip=192.168.1.105&gateway=192.168.1.1&subnet=255.255.255.0&dns1=8.8.8.8
```

Response:
```json
{"status":"ok","message":"Static IP saved. Reconnecting..."}
```

### /api/set-static-ip (POST JSON)

Contoh:
```bash
curl -u admin:admin123 -X POST http://<IP_ESP32>/api/set-static-ip \
  -H "Content-Type: application/json" \
  -d "{\"ip\":\"192.168.1.105\",\"gateway\":\"192.168.1.1\",\"subnet\":\"255.255.255.0\",\"dns1\":\"8.8.8.8\"}"
```

---

## Endpoint Kontrol Relay

| Endpoint | Method | Deskripsi | Response |
|---|---|---|---|
| `/` | GET | Halaman kontrol relay | HTML |
| `/1/on` | GET | Relay 1 ON | `Relay 1 ON` |
| `/1/off` | GET | Relay 1 OFF | `Relay 1 OFF` |
| `/2/on` | GET | Relay 2 ON | `Relay 2 ON` |
| `/2/off` | GET | Relay 2 OFF | `Relay 2 OFF` |
| `/1/status` | GET | Status relay 1 | `ON` / `OFF` |
| `/2/status` | GET | Status relay 2 | `ON` / `OFF` |

Catatan: Endpoint kontrol relay di atas **tidak** memakai autentikasi (sesuai requirement: auth hanya untuk konfigurasi).

---

## Catatan Penting

- Saat static IP disimpan, device akan restart dan IP bisa berubah.
- Jika IP/gateway invalid, device fallback ke DHCP.
- Batas panjang: SSID <= 31, password <= 63, IP/Gateway/Subnet/DNS <= 15 karakter.
