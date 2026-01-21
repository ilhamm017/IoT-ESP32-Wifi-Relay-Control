# Panduan Pengguna - ESP32 WiFi Relay Control

Dokumen ini fokus ke cara pakai (user guide). Untuk detail kode program, lihat `docs/code/DOKUMENTASI_TEKNIS.md`.

---

## Ringkas

- Device menyediakan mode Access Point (AP) untuk setup awal.
- Setelah terkoneksi, halaman konfigurasi tetap bisa diakses lewat `/config`.
- Static IP bisa diaktifkan saat setup atau diubah via endpoint.

---

## Setup Awal (AP Mode)

1. Nyalakan ESP32, tunggu AP aktif.
2. Sambungkan ke WiFi `ESP32-Setup` (password `12345678`).
3. Buka `http://192.168.4.1/config`.
4. Pilih SSID dari list atau isi manual, lalu masukkan password.
5. (Opsional) centang Static IP, isi IP dan gateway.
6. Klik **Simpan & Koneksi**. Device akan restart.

---

## Akses Saat Sudah Terkoneksi

- Buka `http://<IP_ESP32>/config` untuk ganti SSID atau password kapan saja.
- Hal ini tidak perlu reset hardware.

---

## Kontrol Relay

- Buka `http://<IP_ESP32>/` untuk halaman kontrol.
- Tersedia tombol ON/OFF untuk relay 1 dan 2.

---

## Endpoint yang Bisa Diakses

**Base URL:**
- AP mode: `http://192.168.4.1`
- STA mode: `http://<IP_ESP32>`

### Konfigurasi

- `/config` (GET)  
  Fungsi: halaman konfigurasi WiFi (SSID, password, static IP).  
  Contoh: `http://192.168.4.1/config`

- `/save` (POST)  
  Fungsi: simpan SSID/password/static IP dari form.  
  Contoh: gunakan tombol **Simpan & Koneksi** di halaman `/config`.

- `/api/scan-wifi` (GET)  
  Fungsi: scan SSID yang tersedia.  
  Contoh:
  ```
  http://<IP_ESP32>/api/scan-wifi
  ```

- `/api/set-static-ip` (GET/POST)  
  Fungsi: simpan static IP via URL atau JSON.  
  Contoh GET:
  ```
  http://<IP_ESP32>/api/set-static-ip?ip=192.168.1.105&gateway=192.168.1.1
  ```
  Contoh POST:
  ```bash
  curl -X POST http://<IP_ESP32>/api/set-static-ip \
    -H "Content-Type: application/json" \
    -d "{\"ip\":\"192.168.1.105\",\"gateway\":\"192.168.1.1\"}"
  ```
  Catatan: device akan restart dan IP bisa berubah.

### Kontrol Relay

- `/` (GET)  
  Fungsi: halaman kontrol relay.  
  Contoh: `http://<IP_ESP32>/`

- `/1/on` (GET)  
  Fungsi: nyalakan relay 1.  
  Contoh: `http://<IP_ESP32>/1/on`

- `/1/off` (GET)  
  Fungsi: matikan relay 1.  
  Contoh: `http://<IP_ESP32>/1/off`

- `/2/on` (GET)  
  Fungsi: nyalakan relay 2.  
  Contoh: `http://<IP_ESP32>/2/on`

- `/2/off` (GET)  
  Fungsi: matikan relay 2.  
  Contoh: `http://<IP_ESP32>/2/off`

- `/1/status` (GET)  
  Fungsi: status relay 1 (`ON` / `OFF`).  
  Contoh: `http://<IP_ESP32>/1/status`

- `/2/status` (GET)  
  Fungsi: status relay 2 (`ON` / `OFF`).  
  Contoh: `http://<IP_ESP32>/2/status`

---

## Static IP

Ada 2 cara:

1. **Via halaman konfigurasi**: centang "Gunakan Static IP".
2. **Via endpoint**: lihat `docs/code/DOKUMENTASI_ENDPOINT.md`.

Catatan: Setelah static IP disimpan, device akan restart dan IP bisa berubah.

---

## Reset WiFi

- Tekan tombol BOOT selama 5 detik, atau
- Ketik `reset` di Serial Monitor (115200).

Setelah reset, device kembali ke AP mode.


