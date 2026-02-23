#include <Arduino.h>
#include <string.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <TFT_eSPI.h>
#include "config.h"

TFT_eSPI tft = TFT_eSPI();
WiFiServer server(TCP_PORT);
WiFiClient client;

// Frame buffer for receiving RGB565 data (128*128*2 = 32768 bytes)
uint8_t* frameBuffer = nullptr;
size_t bytesBuffered = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== TFT Screen Mirror ===");

  tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
#if defined(TFT_BL) && (TFT_BL >= 0)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif
  tft.drawString("Connecting...", 64, 64, 2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    tft.fillScreen(TFT_RED);
    tft.drawString("WiFi failed!", 64, 64, 2);
    Serial.println("\nWiFi connection failed!");
    return;
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address (use this in PC sender): ");
  Serial.println(WiFi.localIP());
  Serial.print("Port: ");
  Serial.println(TCP_PORT);

  tft.fillScreen(TFT_GREEN);
  tft.drawString("WiFi OK", 64, 50, 2);
  tft.drawString(WiFi.localIP().toString().c_str(), 64, 80, 2);
  tft.drawString("Use this IP on PC", 64, 95, 2);
  tft.drawString("Waiting for PC...", 64, 110, 2);

  server.begin();

  // Allocate frame buffer (32 KB for 128x128 RGB565)
  frameBuffer = (uint8_t*)malloc(FRAME_SIZE);
  if (!frameBuffer) {
    Serial.println("Failed to allocate frame buffer!");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  // Accept new client if none connected
  if (!client || !client.connected()) {
    client = server.available();
    if (client) {
      client.setNoDelay(true);
      Serial.println("PC connected");
      tft.fillScreen(TFT_BLACK);
    }
  }

  if (client && client.connected() && frameBuffer) {
    // Read incoming bytes (TCP is stream-based; accumulate until full frame)
    size_t n = client.read(frameBuffer + bytesBuffered, FRAME_SIZE - bytesBuffered);
    if (n > 0) {
      bytesBuffered += n;
    }

    if (bytesBuffered >= FRAME_SIZE) {
      tft.pushImage(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (uint16_t*)frameBuffer);
      // Discard any extra bytes (next frame starts)
      if (bytesBuffered > FRAME_SIZE) {
        memmove(frameBuffer, frameBuffer + FRAME_SIZE, bytesBuffered - FRAME_SIZE);
        bytesBuffered -= FRAME_SIZE;
      } else {
        bytesBuffered = 0;
      }
    }
  } else {
    bytesBuffered = 0;
  }
}
