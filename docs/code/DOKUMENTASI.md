# Dokumentasi Program Kontrol Relay Wireless (WiFi) - ESP32

Versi: 2.1 (WiFi Scan, Static IP + Subnet/DNS, Basic Auth)  
Status: Production Ready  
Last Updated: 30 Januari 2026

## Ringkasan

- Auto-scan SSID berjalan saat AP aktif dan saat halaman konfigurasi dibuka; koneksi HTTP tetap stabil (AP tidak dimatikan) dan LED indikator berkedip selama scan.
- Saat boot, firmware menjalankan scan awal mode STA-only (AP bisa restart sebentar) agar hasil lebih lengkap.

## Fitur Utama

- Kontrol dua relay (GPIO 32 & 33) via HTTP/HTML UI.
- Mode AP untuk onboarding, tetap bisa akses `/config` saat sudah STA.
- Scan WiFi dari web dan auto-fill SSID.
- Static IP dengan IP/Gateway/Subnet/DNS, fallback ke DHCP jika invalid.
- API untuk set static IP (GET/POST JSON).
- Basic Auth untuk semua endpoint (default `admin` / `admin123` - ganti sebelum produksi).
- Reset kredensial lewat tombol BOOT 5 detik atau perintah serial `reset`.

## Arsitektur Singkat

- WebServer port 80.
- Route konfigurasi: `/config`, `/save`, `/api/scan-wifi`, `/api/set-static-ip`.
- Route kontrol: `/`, `/1/on`, `/1/off`, `/2/on`, `/2/off`, `/1/status`, `/2/status`.
- Penyimpanan konfigurasi di EEPROM 512 byte.

## Quick Start

1) Flash firmware:
   ```bash
   pio run -e esp32doit-devkit-v1 -t upload
   pio device monitor -b 115200
   ```
2) Sambung ke AP `ESP32-Setup` (pass `12345678`).
3) Buka `http://192.168.4.1/config`, login Basic Auth (`admin`/`admin123`).
4) Pilih SSID atau ketik manual, isi password. Opsional: isi IP, Gateway, Subnet, DNS.
5) Klik **Simpan & Koneksi**; perangkat restart dan bergabung ke WiFi.
6) Akses `http://<IP_ESP32>/` (login Basic Auth) untuk kontrol relay.

## Endpoint (wajib Basic Auth)

| Endpoint | Method | Fungsi |
|---|---|---|
| `/` | GET | Halaman kontrol relay |
| `/config` | GET | Halaman konfigurasi WiFi |
| `/save` | POST | Simpan SSID/Password/Static IP |
| `/api/scan-wifi` | GET | Scan SSID terdekat |
| `/api/set-static-ip` | GET/POST | Set IP/Gateway/Subnet/DNS |
| `/1/on` / `/1/off` | GET | Relay 1 ON/OFF |
| `/2/on` / `/2/off` | GET | Relay 2 ON/OFF |
| `/1/status` / `/2/status` | GET | Status relay (ON/OFF) |

Contoh POST static IP:
```bash
curl -u admin:admin123 -X POST http://<IP>/api/set-static-ip \
  -H "Content-Type: application/json" \
  -d '{"ip":"192.168.1.105","gateway":"192.168.1.1","subnet":"255.255.255.0","dns1":"8.8.8.8"}'
```

## Keamanan

- Basic Auth aktif di semua endpoint; ubah credential di kode sebelum produksi.
- Input dibatasi: SSID <= 31 char, password <= 63 char, IP/Gateway/Subnet/DNS <= 15 char.

## EEPROM Layout v2.1

| Offset | Isi | Panjang |
|---|---|---|
| 0-31 | SSID | 32 byte |
| 32-95 | Password | 64 byte |
| 96-111 | Static IP | 16 byte |
| 112-127 | Gateway | 16 byte |
| 128-143 | Subnet Mask | 16 byte |
| 144-159 | DNS1 | 16 byte |
| 160 | Flag use_static_ip (1/0) | 1 byte |
| 161-511 | Reserved | - |

## Perintah Serial

- `status`: tampilkan SSID, status WiFi, dan IP saat ini.
- `reset`: hapus kredensial WiFi & static IP, lalu restart ke AP mode.

## Troubleshooting Singkat

- Tidak bisa akses setelah set static IP: pastikan subnet/gateway benar, atau reset via tombol BOOT 5 detik.
- DNS kosong: perangkat otomatis memakai gateway sebagai DNS.
- Lupa password Basic Auth: ubah konstanta `AUTH_USER`/`AUTH_PASS` di `src/WebHandlers.cpp` lalu re-flash.

## Lisensi

MIT License
