# ESP32 WiFi Relay Control System

Kontrol 2 relay via web browser menggunakan ESP32. Mendukung scan WiFi, konfigurasi IP static, dan halaman konfigurasi yang tetap bisa diakses saat sudah terhubung.

## Key Features

- Kontrol 2 relay (GPIO 32 & 33) via HTTP
- Setup WiFi lewat AP mode + form web
- Scan SSID dan pilih dari list
- Static IP (form atau endpoint)
- Konfigurasi tetap aksesible di `/config`

## Quick Start

1. Upload firmware via PlatformIO:
   ```bash
   pio run -e esp32doit-devkit-v1 -t upload
   pio device monitor -b 115200
   ```
2. Connect ke AP `ESP32-Setup` (password `12345678`).
3. Buka `http://192.168.4.1/config`, isi SSID/password, simpan.
4. Setelah restart, akses `http://<IP_ESP32>/` untuk kontrol relay.

## Endpoint Ringkas

| Endpoint | Method | Fungsi |
|---|---|---|
| `/` | GET | Halaman kontrol relay |
| `/config` | GET | Halaman konfigurasi WiFi |
| `/save` | POST | Simpan SSID/password/static IP |
| `/api/scan-wifi` | GET | Scan WiFi |
| `/api/set-static-ip` | GET/POST | Simpan static IP |

## Dokumentasi

- Panduan pengguna: `PANDUAN_PENGGUNA.md`
- Endpoint lengkap: `docs/code/DOKUMENTASI_ENDPOINT.md`
- Dokumentasi teknis (kode): `docs/code/DOKUMENTASI_TEKNIS.md`
- Dokumentasi lengkap: `docs/code/DOKUMENTASI.md`

## Hardware

- ESP32 DoIT DevKit V1
- Relay 2-channel
- Power 5V (USB/external)

## License

MIT
