# Dokumentasi Program Kontrol Relay Wireless (WiFi) - ESP32

Versi: 2.2 (WiFi Scan, Static IP + Subnet/DNS, Basic Auth, 8 Relay)  
Status: Production Ready  
Last Updated: 4 Februari 2026

## Ringkasan

- Auto-scan SSID berjalan saat AP aktif dan saat halaman konfigurasi dibuka; koneksi HTTP tetap stabil (AP tidak dimatikan) dan LED indikator berkedip selama scan.
- Saat boot, firmware menjalankan scan awal mode STA-only (AP bisa restart sebentar) agar hasil lebih lengkap.

## Fitur Utama

- Kontrol hingga 8 relay via HTTP/HTML UI (default pin: GPIO 32, 33, 25, 26, 27, 14, 16, 17; ubah di `src/main.cpp`).
- Default relay saat boot: ON (active-low). Ubah di `src/main.cpp` jika ingin default OFF.
- Mode AP untuk onboarding, tetap bisa akses `/config` saat sudah STA.
- Scan WiFi dari web dan auto-fill SSID.
- Static IP dengan IP/Gateway/Subnet/DNS, fallback ke DHCP jika invalid.
- API untuk set static IP (GET/POST JSON).
- Basic Auth untuk endpoint konfigurasi (default `admin` / `admin123` - ganti sebelum produksi).
- Reset kredensial lewat tombol BOOT 5 detik atau perintah serial `reset`.

## Arsitektur Singkat

- WebServer port 80.
- Route konfigurasi: `/config`, `/save`, `/api/scan-wifi`, `/api/set-static-ip`.
- Route kontrol: `/`, `/1..8/on`, `/1..8/off`, `/1..8/status`.
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
6) Akses `http://<IP_ESP32>/` untuk kontrol relay (tanpa auth).

## Endpoint Konfigurasi (wajib Basic Auth)

| Endpoint | Method | Fungsi |
|---|---|---|
| `/config` | GET | Halaman konfigurasi WiFi |
| `/save` | POST | Simpan SSID/Password/Static IP |
| `/api/scan-wifi` | GET | Scan SSID terdekat |
| `/api/set-static-ip` | GET/POST | Set IP/Gateway/Subnet/DNS |

## Endpoint Kontrol Relay (tanpa auth)

| Endpoint | Method | Fungsi |
|---|---|---|
| `/` | GET | Halaman kontrol relay |
| `/1..8/on` / `/1..8/off` | GET | Relay N ON/OFF |
| `/1..8/status` | GET | Status relay (ON/OFF) |

Catatan: N mengikuti `RELAY_COUNT` di `src/main.cpp` (default 8).

Contoh POST static IP:
```bash
curl -u admin:admin123 -X POST http://<IP>/api/set-static-ip \
  -H "Content-Type: application/json" \
  -d '{"ip":"192.168.1.105","gateway":"192.168.1.1","subnet":"255.255.255.0","dns1":"8.8.8.8"}'
```

## Keamanan

- Basic Auth aktif di endpoint konfigurasi; ubah credential di kode sebelum produksi.
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
