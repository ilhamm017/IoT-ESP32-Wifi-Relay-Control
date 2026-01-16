import machine
import network
import socket
import time

RELAY_PIN = 12
RESET_BUTTON_PIN = 0  # GPIO 0 = tombol BOOT

AP_SSID = "ESP32-Setup"
AP_PASSWORD = "12345678"

WIFI_FILE = "wifi.json"

relay = machine.Pin(RELAY_PIN, machine.Pin.OUT)
relay.value(1)  # Relay OFF (active-low)

reset_btn = machine.Pin(RESET_BUTTON_PIN, machine.Pin.IN, machine.Pin.PULL_UP)

wifi_connected = False


def load_wifi_credentials():
    try:
        with open(WIFI_FILE, "r") as f:
            content = f.read().strip()
    except OSError:
        return "", ""

    if not content:
        return "", ""

    # Format sederhana: ssid\npassword
    parts = content.split("\n", 1)
    if len(parts) != 2:
        return "", ""

    return parts[0].strip(), parts[1].strip()


def save_wifi_credentials(ssid, password):
    with open(WIFI_FILE, "w") as f:
        f.write(ssid + "\n" + password)


def clear_wifi_credentials():
    try:
        with open(WIFI_FILE, "w") as f:
            f.write("")
    except OSError:
        pass


def start_ap():
    ap_if = network.WLAN(network.AP_IF)
    ap_if.active(True)
    ap_if.config(essid=AP_SSID, password=AP_PASSWORD)
    return ap_if


def stop_ap():
    ap_if = network.WLAN(network.AP_IF)
    ap_if.active(False)


def connect_sta(ssid, password, timeout_s=10):
    global wifi_connected

    if not ssid:
        wifi_connected = False
        return False

    sta_if = network.WLAN(network.STA_IF)
    sta_if.active(True)
    if not sta_if.isconnected():
        sta_if.connect(ssid, password)

        start = time.ticks_ms()
        while not sta_if.isconnected():
            if time.ticks_diff(time.ticks_ms(), start) > timeout_s * 1000:
                break
            time.sleep(0.3)

    wifi_connected = sta_if.isconnected()
    return wifi_connected


def http_response(client, status, content_type, body):
    client.send("HTTP/1.1 {}\r\n".format(status))
    client.send("Content-Type: {}\r\n".format(content_type))
    client.send("Content-Length: {}\r\n".format(len(body)))
    client.send("Connection: close\r\n\r\n")
    client.send(body)


def parse_request(data):
    try:
        text = data.decode()
    except Exception:
        return None, None, {}

    lines = text.split("\r\n")
    if not lines:
        return None, None, {}

    parts = lines[0].split()
    if len(parts) < 2:
        return None, None, {}

    method, path = parts[0], parts[1]
    headers = {}
    for line in lines[1:]:
        if not line:
            break
        if ":" in line:
            key, value = line.split(":", 1)
            headers[key.strip().lower()] = value.strip()

    return method, path, headers


def parse_form(body):
    data = {}
    for pair in body.split("&"):
        if "=" in pair:
            k, v = pair.split("=", 1)
            data[k] = v.replace("+", " ")
    return data


def page_config():
    return (
        "<html>"
        "<head>"
        "<title>WiFi Setup</title>"
        "<style>"
        "body { font-family: Arial; text-align: center; margin-top: 50px; }"
        "input { padding: 10px; font-size: 16px; width: 300px; margin: 10px; }"
        "button { padding: 10px 30px; font-size: 16px; cursor: pointer; }"
        "</style>"
        "</head>"
        "<body>"
        "<h1>Konfigurasi WiFi</h1>"
        "<form action='/save' method='POST'>"
        "<div>"
        "<label>SSID:</label><br>"
        "<input type='text' name='ssid' placeholder='Nama WiFi' required>"
        "</div>"
        "<div>"
        "<label>Password:</label><br>"
        "<input type='password' name='password' placeholder='Password WiFi' required>"
        "</div>"
        "<button type='submit'>Simpan &amp; Koneksi</button>"
        "</form>"
        "</body>"
        "</html>"
    )


def page_control():
    return (
        "<html>"
        "<head>"
        "<title>Relay Control</title>"
        "<style>"
        "body { font-family: Arial; text-align: center; margin-top: 50px; }"
        "button { padding: 20px 40px; font-size: 24px; margin: 10px; cursor: pointer; }"
        ".on { background-color: #4CAF50; color: white; }"
        ".off { background-color: #f44336; color: white; }"
        ".status { margin-top: 30px; font-size: 20px; }"
        "</style>"
        "</head>"
        "<body>"
        "<h1>Kontrol Relay</h1>"
        "<button class='on' onclick='fetch(\"/on\")'>NYALAKAN (ON)</button>"
        "<button class='off' onclick='fetch(\"/off\")'>MATIKAN (OFF)</button>"
        "<div class='status'>"
        "<p>Status: <span id='status'>Loading...</span></p>"
        "</div>"
        "<script>"
        "function updateStatus() {"
        "  fetch('/status').then(r => r.text()).then(data => {"
        "    document.getElementById('status').textContent = data;"
        "  });"
        "}"
        "updateStatus();"
        "setInterval(updateStatus, 1000);"
        "</script>"
        "</body>"
        "</html>"
    )


def handle_client(client):
    data = client.recv(1024)
    if not data:
        return

    method, path, headers = parse_request(data)
    if method is None:
        return

    if path == "/config" and method == "GET":
        http_response(client, "200 OK", "text/html", page_config())
        return

    if path == "/" and method == "GET":
        http_response(client, "200 OK", "text/html", page_control())
        return

    if path == "/on" and method == "GET":
        relay.value(0)
        http_response(client, "200 OK", "text/plain", "Relay ON")
        return

    if path == "/off" and method == "GET":
        relay.value(1)
        http_response(client, "200 OK", "text/plain", "Relay OFF")
        return

    if path == "/status" and method == "GET":
        status = "ON" if relay.value() == 0 else "OFF"
        http_response(client, "200 OK", "text/plain", status)
        return

    if path == "/save" and method == "POST":
        content_length = int(headers.get("content-length", "0"))
        body = ""
        if content_length:
            body_data = data.split(b"\r\n\r\n", 1)
            if len(body_data) == 2:
                body = body_data[1].decode()
            while len(body) < content_length:
                body += client.recv(1024).decode()

        form = parse_form(body)
        ssid = form.get("ssid", "")
        password = form.get("password", "")
        save_wifi_credentials(ssid, password)

        response = (
            "<html>"
            "<head><title>Disimpan</title></head>"
            "<body>"
            "<h1>Konfigurasi Disimpan!</h1>"
            "<p>Menghubung ke WiFi...</p>"
            "<p>Refresh halaman dalam 10 detik</p>"
            "<script>setTimeout(function(){ location.reload(); }, 10000);</script>"
            "</body>"
            "</html>"
        )
        http_response(client, "200 OK", "text/html", response)
        time.sleep(2)
        machine.reset()
        return

    http_response(client, "404 Not Found", "text/plain", "Not Found")


def main():
    ssid, password = load_wifi_credentials()

    if connect_sta(ssid, password):
        stop_ap()
    else:
        start_ap()

    addr = socket.getaddrinfo("0.0.0.0", 80)[0][-1]
    s = socket.socket()
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(addr)
    s.listen(2)
    s.settimeout(0.2)

    button_press_start = None

    while True:
        if reset_btn.value() == 0:
            if button_press_start is None:
                button_press_start = time.ticks_ms()
            else:
                if time.ticks_diff(time.ticks_ms(), button_press_start) >= 5000:
                    clear_wifi_credentials()
                    time.sleep(1)
                    machine.reset()
        else:
            button_press_start = None

        try:
            client, _ = s.accept()
        except OSError:
            continue

        try:
            handle_client(client)
        finally:
            client.close()


main()
