# ⚡ SPEARHEAD // Tactical Digi Keychain & Smart Cyber Companion

<div align="center">

![SPEARHEAD Banner](icons/icon-512.png)

### **Interactive Cyber Companion, Digital Badge & Smart Desk Dock**
*Powered by ESP32-C3 SuperMini (160MHz RISC-V) • 1.3" 240×240 IPS ST7789 • Web-BLE • PWA*

[![GitHub repo size](https://img.shields.io/github/repo-size/Joyboycoffee/Keychain?style=for-the-badge&color=d8c7ad)](https://github.com/Joyboycoffee/Keychain)
[![GitHub stars](https://img.shields.io/github/stars/Joyboycoffee/Keychain?style=for-the-badge&color=e2b46a)](https://github.com/Joyboycoffee/Keychain/stargazers)
[![License: MIT](https://img.shields.io/badge/License-MIT-a2c892.svg?style=for-the-badge)](LICENSE)
[![Web BLE](https://img.shields.io/badge/Web--BLE-Ready-82b1ff?style=for-the-badge)](https://joyboycoffee.github.io/Keychain/)
[![Web Flasher](https://img.shields.io/badge/Web--Flasher-1--Click%20Install-df735d?style=for-the-badge)](https://joyboycoffee.github.io/Keychain/installer.html)

**[🌐 Live Web Control Deck](https://joyboycoffee.github.io/Keychain/)** • **[⚡ 1-Click Web USB Flasher](https://joyboycoffee.github.io/Keychain/installer.html)** • **[📦 Release Binaries](assets/bins/)**

---

</div>

## 🌌 Overview

**SPEARHEAD** is an open-source, cyberpunk-inspired tactile smart keychain, wearable digital badge, and intelligent desk dock. Built on the ultra-compact **ESP32-C3 SuperMini** (160MHz 32-bit RISC-V), it features a crisp 1.3" 240×240 IPS display, debounced capacitive touch interaction, wireless Bluetooth Low Energy (Web-BLE) telemetry, and an automated dual-port smart charging dock system.

Whether clipped to your backpack as an expressive cyber mascot or placed on your desk as an ambient companion, SPEARHEAD delivers ultra-smooth 40+ FPS animations, procedural digital rain, max-font marquee broadcasting, and authentic Dasai Mochi expressions with customizable playback playlists.

---

## ⚡ 1-Click Direct Installation (No Coding / Compiling Required!)

You can flash SPEARHEAD directly onto your ESP32-C3 from your **Android mobile phone** or **computer browser** in seconds:

### Method 1: In-Browser Web Flasher (Easiest - Mobile or Desktop)
1. Open **[https://joyboycoffee.github.io/Keychain/installer.html](https://joyboycoffee.github.io/Keychain/installer.html)** in **Google Chrome**, **Microsoft Edge**, or **Opera** (on Android or PC).
2. Connect your ESP32-C3 to your phone or computer via a USB-C cable (or USB-C to USB-C / OTG cable).
3. Click **"⚡ CONNECT & INSTALL FIRMWARE"** and select the ESP32-C3 JTAG/Serial port.
4. The browser automatically erases and writes all 4MB of firmware, partition tables, bootloader, and LittleFS GIF files!

### Method 2: Direct Single-File Mobile Flashing (Android Apps)
If you prefer using an Android flashing app (such as *Serial USB Terminal*, *ESP32 Loader*, or *DapLink*):
1. Download the single factory merged binary: **[`assets/bins/spearhead_merged_factory_4mb.bin`](assets/bins/spearhead_merged_factory_4mb.bin)**.
2. Connect your ESP32-C3 via OTG cable.
3. In the flasher app, select baud rate `460800` (or `115200`).
4. Select `spearhead_merged_factory_4mb.bin` at target offset **`0x00000000`** (`0x0`).
5. Tap **Flash**!

### Method 3: Desktop Flasher Studio & Python Auto-Command (PC / Mac / Linux)
A dedicated auto-detecting desktop GUI and CLI tool is included directly in the repository:
- **Desktop Application (GUI)**:
  - Double-click **`flash_keychain.bat`** (or the **`Spearhead Flasher`** desktop shortcut) to open the tactical flasher studio.
  - Automatically scans all connected COM ports and highlights your connected ESP32-C3.
  - Features real-time progress bar, live terminal console, baud rate selector, and an *Auto-Flash on Connect* toggle.
- **Instant Automatic CLI Flash**:
  - Run **`flash_auto.bat`** or in terminal:
    ```bash
    python flash_tool.py --auto
    ```
    Scans all connected ports, automatically picks the ESP32-C3, flashes the full factory firmware + Mochi GIF filesystem, and resets the board automatically with zero manual prompts!
- **Advanced CLI Commands**:
  ```bash
  python flash_tool.py --list          # Scan & list candidate ESP serial ports
  python flash_tool.py --port COM7     # Flash specific COM port
  python flash_tool.py --type app      # Fast 4-second app firmware update only (0x10000)
  python flash_tool.py --type fs       # LittleFS GIF assets only (0x170000)
  ```

---

## 📦 Pre-Compiled Binary Assets

All verified release binaries are maintained directly in the repository under [`assets/bins/`](assets/bins/):

| File | Target Flash Offset | Size | Description |
|---|---|---|---|
| **[`spearhead_merged_factory_4mb.bin`](assets/bins/spearhead_merged_factory_4mb.bin)** | **`0x00000000`** | **4.0 MB** | **All-in-one factory image.** Contains bootloader, partition table, boot selector, app firmware, and LittleFS filesystem. *Ideal for 1-click mobile flashing.* |
| **[`firmware.bin`](assets/bins/firmware.bin)** | **`0x00010000`** | ~908 KB | Main application firmware partition. Flash here to update code without re-uploading LittleFS GIFs. |
| **[`littlefs.bin`](assets/bins/littlefs.bin)** | **`0x00170000`** | 2.56 MB | LittleFS filesystem image containing all 42 authentic Dasai Mochi GIF animations. |
| **[`bootloader.bin`](assets/bins/bootloader.bin)** | **`0x00000000`** | 13 KB | ESP32-C3 2nd stage bootloader. |
| **[`partitions.bin`](assets/bins/partitions.bin)** | **`0x00008000`** | 3 KB | Custom 4MB partition table configuration. |
| **[`boot_app0.bin`](assets/bins/boot_app0.bin)** | **`0x0000e000`** | 8 KB | OTA boot selector data partition. |
| **[`manifest.json`](assets/bins/manifest.json)** | — | — | ESP Web Tools manifest for in-browser automated installation. |

---

## 🎮 The 7 Operating Modes

Switch modes instantly by **tapping the button 5 times** or selecting directly from the **Web BLE Dashboard**:

```
[0. CYBERPET] ➔ [1. DASAI MOCHI] ➔ [2. CYBER HUD] ➔ [3. MATRIX RAIN] ➔ [4. MARQUEE] ➔ [5. STREAM MEDIA] ➔ [6. DESK DOCK]
```

### 0. 🐱 CyberPet Mascot Engine
- **7 Interactive Meme Avatars**: Luffy Star-Eyes, Shy Cute `👉👈`, Giggling Kitten with Pink Bows, Sad Banana Cat, Whining Umaru-chan, Grumpy Cat, and Bunny Hum `🐰`.
- Animated particle effects (floating love hearts, twinkling stars, anime tears, and vector anger sparks `💢`).
- Single tap pokes the pet with sound/visual feedback and smoothly retains the active mode.

### 1. 🤖 Dasai Mochi Procedural Faces
- **42 Authentic GIF Animations** loaded directly from flash LittleFS.
- **5 Dynamic Color Palettes**: Warm White ⚪, Pastel Cyan 🩵, Sakura Pink 🌸, Mochi Gold 🟡, and Neon Violet 🟣.
- **Triple-Tap Turbo Boost**: Unleashes high-speed race and drift animations.
- Soundboard triggers for real-time emotional reactions.

### 2. ⏱️ CyberHUD Tactical Dashboard
- **4 Distinct Layouts**:
  - `Layout 0`: Compact HUD (Robot eyes on top, clock below).
  - `Layout 1`: Big Clock & Gauges (Large tactical readout).
  - `Layout 2`: Minimalist Dash.
  - `Layout 3`: **Minimalistic One** (Large clean Fredoka-inspired clock with weekday & date text, tailored for desk companion mode).
- **Battery Readout**: Real-time calibrated voltage (`3.70V - 4.20V`) and color-coded percentage gauge (Green / Yellow / Red).
- **Animated Sine Waveform**: Real-time audio-visualizer frequency wave.
- 12-Hour (AM/PM) and 24-Hour Military time format switching.

### 3. 🌧️ Matrix Digital Rain
- Cascading green phosphor code rain rendered with custom micro-glyphs.
- **4 Color Themes**: Cyber Green, Neon Blue, Amber Gold, and **Heart Rain** (cascading hearts for secret hold mode).

### 4. 📢 Marquee Broadcast Ticker
- Ultra-smooth, non-blocking horizontal ticker tape broadcast.
- Configurable font sizes (Size 1 to 12 Mega-font), scroll speed, and bidirectional scrolling (Left-to-Right or Right-to-Left).
- Enclosed in an Obsidian and Tactical Amber cyber frame.

### 5. 🎬 30 FPS Stream Media Engine
- Hardware DMA-accelerated frame pusher capable of sustained 30 FPS video streaming over Web-BLE.
- Still image viewer loaded from internal flash (`/live_image.jpg`).

### 6. 🛋️ Smart Desk Dock (Desk Companion)
- **Automatic USB Power Sensing**: Engages automatically after 10 seconds of verified USB charging power.
- **Cross-Fade Transitions**: Smoothly fades between Dasai Mochi animations and the Minimalistic Clock at zero screen tear.
- **Customizable 42-Animation Playlist**:
  - Web UI selector allows picking which animations will play while docked.
  - **Quick Presets**: `⭐ TOP 12`, `✨ TOP 20`, `🌟 ALL 42`, `🥰 KAWAII`, `⚡ ACTION`, `☕ CHILL`, and `🚫 CLEAR`.
  - **Sequential vs. Shuffle**: Choose between ordered cycling or randomized transitions.
  - Seamless loading: the new GIF is decoded into RAM while the display is dark during cross-fade!
- **Auto Bluetooth Lock**: Bluetooth remains permanently ON and discoverable while docked; automatically powers down after 30 seconds of inactivity when unplugged to conserve battery.

---

## 🖐️ Tactile Button & Touch Gestures

| Gesture | Action | Description |
|---|---|---|
| **1 Tap** | **Poke / Interact** | Gentle interaction; plays custom reaction and remembers/returns to previous active mode. |
| **2 Taps** | **Sub-Option / Advance** | Cycles avatar emotions, changes matrix theme, or **advances to the next animation** in Desk Dock playlist. |
| **3 Taps** | **Turbo Boost** | Triggers high-speed racing animation and speedlines. |
| **4 Taps** | **BLE Radio Toggle** | Toggles Bluetooth Low Energy ON or OFF. (Blue LED blinks once on enable, twice on disable). |
| **5 Taps** | **Cycle System Mode** | Cycles through all 7 operating modes. |
| **7 Taps** | **Easter Egg** | Increments and unlocks secret developer easter egg counter! |
| **Hold (2s)** | **Universal Shy Love** | Triggers secret custom reaction message and blush animation across all modes. |
| **Deep Sleep** | **Wakeup** | Press button again to wake up instantly! |

---

## 🔌 Hardware Wiring & Schematic

```
               ┌─────────────────────────────────────┐
               │         ESP32-C3 SuperMini          │
               │                                     │
               │  [GPIO 6]  ───────> ST7789 SCL      │
               │  [GPIO 10] ───────> ST7789 SDA      │
               │  [GPIO 5]  ───────> ST7789 RES      │
               │  [GPIO 3]  ───────> ST7789 DC       │
               │  [GPIO 2]  ───────> ST7789 BLK (PWM)│
               │  [GPIO 4]  <─────── TTP223 / Button │
               │  [GPIO 0]  <─────── Battery Divider │
               │  [GPIO 20] <─────── TP4056 STDBY LED│
               │  [GPIO 21] <─────── TP4056 CHRG LED │
               │  [3.3V]    ───────> Display & Sensor│
               │  [GND]     ───────> Common Ground   │
               └─────────────────────────────────────┘
```

| ESP32-C3 Pin | Target Pin / Component | Function / Notes |
|---|---|---|
| **GPIO 6** | ST7789 **SCL / SCK** | SPI Clock (Configured via LovyanGFX @ 40MHz) |
| **GPIO 10** | ST7789 **SDA / MOSI** | SPI Data |
| **GPIO 5** | ST7789 **RES / RST** | Display Hardware Reset |
| **GPIO 3** | ST7789 **DC** | Display Data / Command Selection |
| **GPIO 2** | ST7789 **BLK / BL** | PWM Display Backlight Control (Actively clamped LOW in deep sleep) |
| **GPIO 4** | TTP223 **SIG** / Button | Capacitive Touch / Tactile Button (RTC Wakeup capable) |
| **GPIO 0** | Battery ADC Divider | Analog battery voltage measurement (16-sample smoothed ADC) |
| **GPIO 20** | TP4056 **Blue / STDBY** | Pure High-Z dynamic sensing for external charger module |
| **GPIO 21** | TP4056 **Red / CHRG** | Pure High-Z dynamic sensing for external charger module |
| **3.3V** | Display & Sensor **VCC** | Regulated 3.3V Power Rail |
| **GND** | Display & Sensor **GND** | Common System Ground |

> **ST7789 Display Note**: 7-pin ST7789 modules have CS tied internally to GND. LovyanGFX is configured for **SPI Mode 3 (Clock idle HIGH)** and **270° landscape-left rotation**.

---

## 🔋 Power Management & Battery Calibration

- **0% Backlight Bleed Deep Sleep**: When powered down or idle timeout expires, `gpio_hold_en((gpio_num_t)2)` clamps the backlight pin LOW. The screen is 100% pitch-black with zero power leakage.
- **Multimeter Calibration Tool**: In the Web Control Deck under **Power & Logs**, enter your battery's actual voltage observed via a multimeter to calibrate the ADC multiplier. Saved permanently in ESP32 Flash Preferences!
- **Battery Run-Time Logger**: Automatically records the discharge run voltage to LittleFS flash at every 1% battery step. Retrieve the full log via Web-BLE to plot actual battery curves.
- **Brown-Out Suppression**: ESP32-C3 software brownout reset suppression prevents random reboots when connecting or disconnecting cables.

---

## 🛠️ Building From Source (PlatformIO)

If you wish to modify the code or compile locally:

### 1. Prerequisites
- Install [Visual Studio Code](https://code.visualstudio.com/) and the [PlatformIO IDE Extension](https://platformio.org/).
- Or install the [PlatformIO Core CLI](https://docs.platformio.org/en/latest/core/installation/index.html).

### 2. Clone Repository
```bash
git clone https://github.com/Joyboycoffee/Keychain.git
cd Keychain
```

### 3. Build & Upload Firmware
```bash
# Compile firmware
pio run

# Flash to connected ESP32-C3
pio run --target upload

# Upload LittleFS GIF Filesystem (Required for Mochi animations)
pio run --target uploadfs
```

---

## 📱 Progressive Web App (PWA) & Web-BLE

SPEARHEAD comes with a zero-install, offline-capable **Progressive Web App**:

- **Web Bluetooth (Web-BLE)**: Connects directly from Chrome or Edge on Android, Windows, Mac, Linux, or iOS (via Bluefy browser).
- **Time & Date Auto-Sync**: Automatically synchronizes your device's clock and date to phone local time on connection.
- **Install as Native App**: Tap the in-app *"INSTALL APP"* prompt to add SPEARHEAD to your home screen or desktop as a standalone application.

---

## 📁 Repository Structure

```
├── assets/
│   ├── bins/
│   │   ├── spearhead_merged_factory_4mb.bin   # 4MB single factory binary (Offset 0x0)
│   │   ├── firmware.bin                       # App firmware partition (Offset 0x10000)
│   │   ├── littlefs.bin                       # LittleFS filesystem partition (Offset 0x170000)
│   │   ├── bootloader.bin                     # Bootloader (Offset 0x0)
│   │   ├── partitions.bin                     # Partition table (Offset 0x8000)
│   │   ├── boot_app0.bin                      # OTA boot data (Offset 0xe000)
│   │   └── manifest.json                      # ESP Web Tools installer manifest
│   ├── sandwich_layout.svg                    # Hardware CAD / mechanical layout
│   ├── tp4056_mod.svg                         # TP4056 sense modification guide
│   └── wiring_schematic.svg                   # Full electrical wiring schematic
├── data/
│   └── mochi/                                 # 42 Authentic Dasai Mochi GIF files
├── include/
│   ├── dasai_mochi.h                          # 42 Mochi GIF player & animation engine
│   ├── cyber_hud.h                            # CyberHUD layouts & clock renderer
│   ├── meme_pet.h                             # CyberPet 7 meme avatars
│   └── matrix_rain.h                          # Digital rain canvas engine
├── src/
│   └── main.cpp                               # Master firmware state machine & BLE GATT
├── icons/                                     # High-res PWA and platform icons
├── index.html                                 # Web Bluetooth Control Deck (PWA root)
├── installer.html                             # 1-Click Web USB / Serial Firmware Installer
├── partitions.csv                             # Custom 4MB partition table
├── platformio.ini                             # PlatformIO build configuration
└── README.md                                  # You are here!
```

---

## 👤 Author & Architecture

**Gourav Kumar** ([@Joyboycoffee](https://github.com/Joyboycoffee))  
*Embedded Firmware, BLE Communication & Hardware UI Development*

---

## 📄 License

This project is licensed under the **MIT License** — free for maker, hobbyist, and open-source use. See [`LICENSE`](LICENSE) for details.
