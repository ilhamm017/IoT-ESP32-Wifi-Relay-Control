# Panduan Pengguna - ESP32 WiFi Relay Control

Dokumen ini fokus ke cara pakai (user guide). Untuk detail kode program, lihat `docs/code/DOKUMENTASI_TEKNIS.md`.

---

## Ringkas

- Device menyediakan mode Access Point (AP) untuk setup awal.
- Setelah terkoneksi, halaman konfigurasi tetap bisa diakses lewat `/config`.
- Konfigurasi WiFi dilindungi **Basic Auth** (default user `admin`, password `admin123`).
- Endpoint kontrol relay (ON/OFF/status) **tidak** memakai autentikasi.
- Static IP bisa diaktifkan saat setup atau diubah via endpoint.
- WiFi scan berjalan otomatis saat AP aktif dan saat halaman config dibuka; koneksi HTTP tetap jalan (AP tidak dimatikan) dan LED indikator onboard akan berkedip selama proses scan. Catatan: saat boot ada scan awal STA-only sehingga AP bisa restart sebentar.

---

## Setup Awal (AP Mode)

1. Nyalakan ESP32, tunggu AP aktif.
2. Sambungkan ke WiFi `ESP32-Setup` (password `12345678`).
3. Buka `http://192.168.4.1/config`, login Basic Auth (admin/admin123).
4. Daftar SSID otomatis muncul (scan otomatis). Bisa tekan **Scan Ulang** kapan saja.
5. Pilih SSID dari list atau isi manual, lalu masukkan password.
6. (Opsional) centang Static IP, isi IP, gateway, subnet, dan DNS (biarkan DNS kosong untuk gunakan gateway).
7. Klik **Simpan & Koneksi**. Device akan restart.

---

## Akses Saat Sudah Terkoneksi

- Buka `http://<IP_ESP32>/config` (login Basic Auth) untuk ganti SSID/password kapan saja.
- Hal ini tidak perlu reset hardware.

---

## Kontrol Relay

- Buka `http://<IP_ESP32>/` untuk halaman kontrol.
- Tersedia tombol ON/OFF untuk semua relay yang dikonfigurasi (default 8). Jumlah relay mengikuti `RELAY_COUNT` di `src/main.cpp`.
- Default relay saat boot: **ON** (active-low). Ubah di `src/main.cpp` jika ingin default OFF.

---

## Endpoint yang Bisa Diakses

**Base URL:**
- AP mode: `http://192.168.4.1`
- STA mode: `http://<IP_ESP32>`

### Konfigurasi (wajib Basic Auth)

- `/config` (GET)  
  Fungsi: halaman konfigurasi WiFi (SSID, password, static IP).  
  Contoh: `http://192.168.4.1/config`

- `/save` (POST)  
  Fungsi: simpan SSID/password/static IP dari form.  
  Contoh: gunakan tombol **Simpan & Koneksi** di halaman `/config`.

- `/api/scan-wifi` (GET)  
  Fungsi: scan SSID yang tersedia (AP tetap aktif; saat boot ada scan awal STA-only sehingga AP bisa restart sebentar).  
  Contoh:
  ```
  http://<IP_ESP32>/api/scan-wifi
  ```

- `/api/set-static-ip` (GET/POST)  
  Fungsi: simpan static IP via URL atau JSON (mendukung `ip`, `gateway`, `subnet`, `dns1`).  
  Contoh GET:
  ```
  http://<IP_ESP32>/api/set-static-ip?ip=192.168.1.105&gateway=192.168.1.1&subnet=255.255.255.0&dns1=8.8.8.8
  ```
  Contoh POST:
  ```bash
  curl -u admin:admin123 -X POST http://<IP_ESP32>/api/set-static-ip \
    -H "Content-Type: application/json" \
    -d "{\"ip\":\"192.168.1.105\",\"gateway\":\"192.168.1.1\",\"subnet\":\"255.255.255.0\",\"dns1\":\"8.8.8.8\"}"
  ```
  Catatan: device akan restart dan IP bisa berubah.

### Kontrol Relay

- `/` (GET)  
  Fungsi: halaman kontrol relay.  
  Contoh: `http://<IP_ESP32>/`

- `/N/on` (GET)  
  Fungsi: nyalakan relay N (N = 1..`RELAY_COUNT`, default 1..8).  
  Contoh: `http://<IP_ESP32>/1/on`

- `/N/off` (GET)  
  Fungsi: matikan relay N (N = 1..`RELAY_COUNT`, default 1..8).  
  Contoh: `http://<IP_ESP32>/1/off`

- `/N/status` (GET)  
  Fungsi: status relay N (`ON` / `OFF`).  
  Contoh: `http://<IP_ESP32>/1/status`

---

## Static IP

Ada 2 cara:

1. **Via halaman konfigurasi**: centang "Gunakan Static IP" dan isi IP/Gateway/Subnet/DNS.
2. **Via endpoint**: lihat `docs/code/DOKUMENTASI_ENDPOINT.md`.

Catatan: Setelah static IP disimpan, device akan restart dan IP bisa berubah.

---

## Keamanan & Auth

- Default Basic Auth: `admin` / `admin123`. **Ganti sebelum dipakai di jaringan produksi.**
- Hanya endpoint konfigurasi yang membutuhkan Basic Auth.
- Batas panjang input: SSID maks 31 karakter, password maks 63 karakter, field IP maks 15 karakter.

## Reset WiFi

- Tekan tombol BOOT selama 5 detik, atau
- Ketik `reset` di Serial Monitor (115200).

Setelah reset, device kembali ke AP mode.


