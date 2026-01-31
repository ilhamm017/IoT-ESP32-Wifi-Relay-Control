# ESP32 WiFi Relay Control System

Kontrol 2 relay via web browser menggunakan ESP32. Mendukung scan WiFi, konfigurasi IP static (dengan subnet & DNS), Basic Auth, dan halaman konfigurasi yang tetap bisa diakses saat sudah terhubung.

## Key Features

- Kontrol 2 relay (GPIO 32 & 33) via HTTP (tanpa autentikasi)
- Setup WiFi lewat AP mode + form web (dengan proteksi Basic Auth, default `admin/admin123`)
- Auto-scan SSID saat AP dinyalakan dan saat halaman config dibuka; tombol "Scan Ulang" tersedia (scan ulang tidak mematikan AP; saat boot AP bisa restart sebentar untuk scan awal)
- Static IP lengkap (IP, Gateway, Subnet, DNS) via form atau endpoint
- Konfigurasi tetap aksesible di `/config`
- LED indikator onboard berkedip selama proses scan WiFi

## Quick Start

1. Upload firmware via PlatformIO:
   ```bash
   pio run -e esp32doit-devkit-v1 -t upload
   pio device monitor -b 115200
   ```
2. Connect ke AP `ESP32-Setup` (password `12345678`).
3. Buka `http://192.168.4.1/config`, login Basic Auth (default `admin` / `admin123`), isi SSID/password, subnet/DNS jika perlu, lalu simpan.
4. Setelah restart, akses `http://<IP_ESP32>/` untuk kontrol relay (tanpa auth).
5. Segera ganti password Basic Auth di kode sebelum produksi (untuk halaman konfigurasi).

## Endpoint Ringkas

**Konfigurasi (Basic Auth):**

| Endpoint | Method | Fungsi |
|---|---|---|
| `/config` | GET | Halaman konfigurasi WiFi |
| `/save` | POST | Simpan SSID/password/static IP |
| `/api/scan-wifi` | GET | Scan WiFi |
| `/api/set-static-ip` | GET/POST | Simpan static IP + subnet + DNS |

**Kontrol relay (tanpa auth):**

| Endpoint | Method | Fungsi |
|---|---|---|
| `/` | GET | Halaman kontrol relay |
| `/1/on` | GET | Relay 1 ON |
| `/1/off` | GET | Relay 1 OFF |
| `/2/on` | GET | Relay 2 ON |
| `/2/off` | GET | Relay 2 OFF |
| `/1/status` | GET | Status relay 1 |
| `/2/status` | GET | Status relay 2 |

## Dokumentasi

- Panduan pengguna: `PANDUAN_PENGGUNA.md`
- Endpoint lengkap: `docs/code/DOKUMENTASI_ENDPOINT.md`
- Dokumentasi teknis (kode): `docs/code/DOKUMENTASI_TEKNIS.md`
- Dokumentasi lengkap: `docs/code/DOKUMENTASI.md`
- Ringkasan fitur: `docs/code/FITUR_RINGKASAN.md`

## Hardware

- ESP32 DoIT DevKit V1
- Relay 2-channel
- Power 5V (USB/external)

## License

MIT
