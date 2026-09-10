# ⚡ ESP32-C3 Cyber Meme Keychain Badge

An open-source interactive cyberpunk smart keychain and digital badge powered by the **ESP32-C3 SuperMini**, a **1.3" 240×240 IPS ST7789 display**, capacitive touch interaction, Web Bluetooth (Web-BLE) controls, and an intelligent low-power management system.

![GitHub repo size](https://img.shields.io/github/repo-size/Joyboycoffee/Keychain)
![GitHub last commit](https://img.shields.io/github/last-commit/Joyboycoffee/Keychain)
![License](https://img.shields.io/badge/license-MIT-cyan.svg)

---

## 🚀 Features

- **🎭 7 Interactive Meme Avatars**:
  1. **Luffy Star-Eyes** (Default excited state)
  2. **Shy Cute 👉👈** (Animated floating love hearts)
  3. **Giggling Kitten with Pink Bows** (Twinkling star effects)
  4. **Sad Banana Cat 🍌** (Animated crying tears)
  5. **Whining Umaru-chan 😭** (Dramatic anime whine effects)
  6. **Grumpy Cat 😾** (Vector anime anger sparks `💢`)
  7. **Bunny "Hum!" 🐰** (Pouting with vector anger sparks `💢`)
- **🖐️ Intuitive Touch Gestures (TTP223 on GPIO 1)**:
  - **Single Tap**: Gentle poke / interaction (keeps current avatar steady, no accidental shuffling).
  - **Double Tap**: Triggers interactive meme reactions (cycles through emotions).
  - **Hold (>0.5s)**: Triggers **Shy Love 👉👈** with floating hearts.
  - **Auto-Revert**: Returns smoothly to default **Luffy** after 4 seconds of idle.
- **📢 Screen-Filling Max-Font Text Marquee**:
  - Huge bold font (Size 4, neon green) centered horizontally and scrolling smoothly from right to left.
  - Single clean cyan border with zero clutter or sub-labels.
- **🎬 30 FPS Video / GIF Player Engine**:
  - Web App slices video/GIF clips into frames and uploads them into internal RAM.
  - Plays continuously at a hardware-accelerated **30 FPS (33ms per frame)** via DMA transfer.
- **🖼️ Custom Image Uploader**:
  - Instant picture upload directly from any browser via Web-BLE.
- **🌑 100% Pitch-Black Auto Deep Sleep**:
  - Automatically enters deep sleep after **30 seconds** of inactivity without BLE connection.
  - Hardware `gpio_hold_en` actively clamps backlight pin (GPIO 2) LOW for **0% backlight leakage (pitch black)**.
  - Instant wakeup on touching the touch sensor (GPIO 1) or pressing RST.
- **🌐 Responsive Web-BLE Controller**:
  - Connects wirelessly from Chrome / Edge (Android, iOS Web-BLE, Mac, Windows, Linux).
  - Syncs phone time and weather.
  - Lets you select avatars, send banner messages, upload images, and play videos.

---

## 🔌 Hardware Wiring Diagram

| ESP32-C3 SuperMini Pin | Component Pin | Function |
|---|---|---|
| **GPIO 6** | ST7789 **SCL / SCK** | SPI Clock |
| **GPIO 10** | ST7789 **SDA / DIN** | SPI Data (MOSI) |
| **GPIO 5** | ST7789 **RES / RST** | Display Hardware Reset |
| **GPIO 3** | ST7789 **DC** | Data / Command Selection |
| **GPIO 2** | ST7789 **BLK / BL** | PWM Backlight Control |
| **3.3V** | ST7789 **VCC** | Power (3.3V) |
| **GND** | ST7789 **GND** | Ground |
| **GPIO 1** (RTC) | TTP223 **SIG / OUT** | Capacitive Touch Sensor |
| **3.3V** | TTP223 **VCC** | Touch Sensor Power |
| **GND** | TTP223 **GND** | Touch Sensor Ground |

> **Note on ST7789 7-pin displays**: CS is tied internally to GND. LovyanGFX is configured for **SPI Mode 3 (Clock idle HIGH)** and **270° landscape-left rotation**.

---

## 🛠️ Building & Flashing

This project uses [PlatformIO](https://platformio.org/).

### 1. Build Firmware:
```bash
pio run
```

### 2. Upload to ESP32-C3:
```bash
pio run --target upload
```

---

## 📱 Web Bluetooth Controller

Open [`index.html`](index.html) directly in any Web-BLE compatible browser (Chrome / Edge) or host it via GitHub Pages:

1. Click **"📡 Connect Keychain"**.
2. Pair with **`CYBER_KEYCHAIN`**.
3. Enjoy full wireless control over meme emotions, marquee text, brightness, time sync, custom images, and 30 FPS video playback!

---

## 📄 License
MIT License. Free for maker, hobbyist, and open-source projects!
