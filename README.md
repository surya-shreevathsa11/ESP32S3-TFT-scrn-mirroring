# ESP32-S3 TFT Screen Mirroring

Stream your Windows desktop to a 1.44" 128×128 TFT display over WiFi. The ESP32-S3 runs a TCP server; a Python script on your PC captures the screen, scales it to fit the display, and sends RGB565 frames. The full desktop is shown with correct aspect ratio (letterboxed when needed).

**Hardware:** Seeed Studio XIAO ESP32-S3 + ST7735 128×128 SPI TFT.

---

## Table of Contents

- [Hardware](#hardware)
- [Wiring](#wiring)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Ideas to Improve](#ideas-to-improve)
- [Project Structure](#project-structure)
- [License](#license)

---

## Hardware

| Item | Notes |
|------|--------|
| **Seeed Studio XIAO ESP32-S3** | WiFi + USB-C |
| **1.44" 128×128 RGB TFT** | ST7735 driver, 8-pin SPI (GND, Vcc, SCL, SDA, RES, DC, CS, BLK) |

---

## Wiring

| TFT Pin | → | XIAO ESP32-S3 | GPIO |
|---------|---|----------------|------|
| GND     |   | GND            | —    |
| Vcc     |   | 3V3            | —    |
| SCL     |   | D8             | 7    |
| SDA     |   | D10            | 9    |
| RES     |   | D4             | 5    |
| DC      |   | D3             | 4    |
| CS      |   | D2             | 3    |
| BLK     |   | D5             | 6    |

- Do not use D6 or D7 (USB serial).
- For always-on backlight, connect BLK to 3V3 instead of D5.

---

## Prerequisites

- **VS Code** with **PlatformIO** (or PlatformIO IDE)
- **Python 3.7+** (for the PC transmitter)
- Laptop and ESP32 on the **same WiFi network**

---

## Quick Start

### 1. Clone the repository

```bash
git clone https://github.com/surya-shreevathsa11/ESP32S3-TFT-scrn-mirroring.git
cd ESP32S3-TFT-scrn-mirroring
```

### 2. Configure WiFi

Edit `include/config.h` and set your network credentials:

```c
#define WIFI_SSID     "YourWiFiName"
#define WIFI_PASSWORD "YourPassword"
```

### 3. Build and upload to ESP32

1. Connect the XIAO ESP32-S3 via USB.
2. In PlatformIO: **Build** (✓) then **Upload** (→).
3. Open **Serial Monitor** at **115200** baud.
4. Wait for `WiFi connected` and note the **IP address** (e.g. `192.168.1.100`).

### 4. Run the PC transmitter

```bash
cd pc_transmitter
pip install -r requirements.txt
python transmitter.py --ip 192.168.1.100
```

Replace `192.168.1.100` with the IP shown in the Serial Monitor. The TFT will show your desktop, scaled to 128×128.

---

## Usage

| Option | Description | Default |
|--------|-------------|---------|
| `--ip` | ESP32 IP address | *(required)* |
| `--port` | TCP port | 8090 |
| `--target-fps` | Target frame rate | 10 |
| `--monitor-index` | Monitor to capture (1=primary, 2=secondary, …) | 1 |

**Examples:**

```bash
# Default (primary monitor)
python transmitter.py --ip 192.168.1.100

# Smoother motion
python transmitter.py --ip 192.168.1.100 --target-fps 15

# Second monitor
python transmitter.py --ip 192.168.1.100 --monitor-index 2
```

Stop streaming with **Ctrl+C**.

---

## Troubleshooting

### Yellow shows as blue (or red/blue swapped)

Some ST7735 panels expect BGR order. You can either:

- In `platformio.ini` under `build_flags`, add: `-D TFT_RGB_ORDER=0` then rebuild and upload, **or**
- In `pc_transmitter/transmitter.py`, change the pixel line to BGR565:  
  `((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3)` and use that instead of the current RGB565 line.

### Display split / bottom at top

The ST7735 “tab” type may not match your display. In `platformio.ini` try:

- `ST7735_BLACKTAB` (default) — no offset
- `ST7735_GREENTAB3` — if you see random pixels on the edges

### Connection refused / cannot connect

- Confirm the ESP32 IP from the Serial Monitor.
- Ensure laptop and ESP32 are on the same WiFi.
- Allow Python through Windows Firewall (private networks) if prompted.
- Check that port 8090 is not in use by another application.

### Display stays black or garbage

- Recheck wiring (SCL→D8, SDA→D10, CS→D2, DC→D3, RES→D4).
- Re-upload firmware; hold **Boot** on the XIAO while starting upload if needed.

### Low FPS or lag

- Lower `--target-fps` (e.g. 5–8).
- Use 5 GHz WiFi if available and keep the ESP32 in good signal range.
- In `platformio.ini`, reduce `SPI_FREQUENCY` (e.g. to `20000000`).

---

## Ideas to Improve (Same Hardware)

- **Higher FPS** — Tune `--target-fps` and WiFi; optionally capture a smaller screen region.
- **Capture a region** — Mirror only a window or a fixed rectangle (e.g. taskbar, corner) to reduce load.
- **Auto-reconnect** — Retry connecting to the ESP32 if the link drops.
- **Save IP** — Store the last-used ESP32 IP in a config file so you do not have to type it each time.
- **Hotkey to start/stop** — Use a launcher or AutoHotkey to start/stop the transmitter.
- **Battery + case** — Power the XIAO from a USB power bank and use a small case for a portable mirror.
- **Different modes** — e.g. full mirror vs static image or clock (could use a button on the ESP32 later).

---

## Project Structure

```
ESP32S3-TFT-scrn-mirroring/
├── src/
│   └── main.cpp           # ESP32: WiFi server, receives frames, draws to TFT
├── include/
│   └── config.h           # WiFi SSID, password, TCP port
├── pc_transmitter/
│   ├── transmitter.py     # PC: capture screen, resize, send RGB565 over TCP
│   └── requirements.txt  # Python deps (mss, Pillow, numpy)
├── platformio.ini        # Board, libs, TFT_eSPI build flags
└── README.md
```

---

## License

This project is for educational and personal use. Use and modify as you like; attribution is appreciated.
