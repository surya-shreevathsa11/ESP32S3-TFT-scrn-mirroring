#!/usr/bin/env python3
"""
TFT Screen Mirror - PC Transmitter
Captures the screen, resizes to 128x128, converts to RGB565, and streams over TCP to ESP32.
"""

import argparse
import socket
import sys
import time

import mss
import numpy as np
from PIL import Image

DISPLAY_WIDTH = 128
DISPLAY_HEIGHT = 128
FRAME_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2  # RGB565 = 2 bytes per pixel


def capture_and_convert(monitor_index=1) -> bytes:
    """Capture full screen, fit entire image into 128x128 (letterbox), convert to RGB565."""
    with mss.mss() as sct:
        monitor = sct.monitors[monitor_index]
        screenshot = sct.grab(monitor)
        img = Image.frombytes(
            "RGB",
            (screenshot.width, screenshot.height),
            screenshot.rgb,
        )
    w, h = img.size
    scale = min(DISPLAY_WIDTH / w, DISPLAY_HEIGHT / h)
    new_w = max(1, round(w * scale))
    new_h = max(1, round(h * scale))
    img = img.resize((new_w, new_h), Image.Resampling.LANCZOS)
    canvas = Image.new("RGB", (DISPLAY_WIDTH, DISPLAY_HEIGHT), (0, 0, 0))
    paste_x = (DISPLAY_WIDTH - new_w) // 2
    paste_y = (DISPLAY_HEIGHT - new_h) // 2
    canvas.paste(img, (paste_x, paste_y))

    arr = np.array(canvas)
    r, g, b = arr[:, :, 0], arr[:, :, 1], arr[:, :, 2]
    rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
    return rgb565.astype(np.uint16).tobytes()


def main():
    parser = argparse.ArgumentParser(description="Stream laptop screen to ESP32 TFT")
    parser.add_argument("--ip", required=True, help="ESP32 IP address")
    parser.add_argument("--port", type=int, default=8090, help="TCP port (default: 8090)")
    parser.add_argument(
        "--target-fps",
        type=float,
        default=10.0,
        help="Target frames per second (default: 10)",
    )
    parser.add_argument(
        "--monitor-index",
        type=int,
        default=1,
        help="Monitor index (1=primary, 2=secondary, etc.)",
    )
    args = parser.parse_args()

    interval = 1.0 / args.target_fps
    last_frame_time = 0
    frame_count = 0

    print(f"Connecting to {args.ip}:{args.port}...")
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        sock.connect((args.ip, args.port))
        print("Connected. Streaming... (Ctrl+C to stop)")
    except (socket.error, OSError) as e:
        print(f"Connection failed: {e}")
        sys.exit(1)

    try:
        while True:
            now = time.perf_counter()
            if now - last_frame_time >= interval:
                frame = capture_and_convert(args.monitor_index)
                try:
                    sock.sendall(frame)
                except (BrokenPipeError, ConnectionResetError, OSError) as e:
                    print(f"\nConnection lost: {e}")
                    break
                frame_count += 1
                if frame_count % 30 == 0:
                    print(f"\rFrames sent: {frame_count}", end="", flush=True)
                last_frame_time = now
            else:
                time.sleep(0.001)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
