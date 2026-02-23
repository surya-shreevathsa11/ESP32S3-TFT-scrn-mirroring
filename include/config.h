#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials - change these before uploading
#define WIFI_SSID     "YourWiFiName"
#define WIFI_PASSWORD "YourPassword"

// TCP server port for frame reception (PC sender must use this port)
#define TCP_PORT 8090

// IP address: You do NOT set a fixed IP here. The ESP32 gets an IP via DHCP
// from your router. After WiFi connects, the IP is printed on the Serial
// Monitor and shown on the TFT. Use that IP in your PC sender app (e.g. the
// "Connect to" or "ESP32 IP" field). Ensure PC and ESP32 are on the same WiFi.

// Display dimensions (must match transmitter)
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 128
#define FRAME_SIZE     (DISPLAY_WIDTH * DISPLAY_HEIGHT * 2)  // RGB565 = 2 bytes per pixel

#endif
