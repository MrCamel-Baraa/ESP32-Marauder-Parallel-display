# ESP32 Marauder — DIY Build

A custom implementation of [justcallmekoko's ESP32 Marauder](https://github.com/justcallmekoko/ESP32Marauder) built entirely from off-the-shelf components. This build uses a standard ESP32 DevKit and a 2.4" TFT LCD shield with an analog resistive touchscreen — no official Marauder PCB required.

---

## Hardware

| Component | Details |
|-----------|---------|
| Microcontroller | ESP32 DevKit (30-pin), ESP32-D0WD-V3, 4MB flash, CH340C USB, USB-C |
| Display | 2.4" TFT LCD Shield, ILI9341 driver, 8-bit parallel interface |
| Touchscreen | Resistive analog touch (no XPT2046 chip) |
| SD Card | FAT32 formatted, separate SPI bus |

---

## Wiring

### Display (8-bit Parallel)

| TFT Pin | ESP32 Pin |
|---------|-----------|
| 3V3 | 3V3 |
| GND | GND |
| LCD_RST | D26 |
| LCD_CS | D27 |
| LCD_RS | D14 |
| LCD_WR | D32 |
| LCD_RD | D13 |
| LCD_D0 | D16 |
| LCD_D1 | D4 |
| LCD_D2 | D23 |
| LCD_D3 | D22 |
| LCD_D4 | D21 |
| LCD_D5 | D19 |
| LCD_D6 | D18 |
| LCD_D7 | D17 |

### Touch (Analog)

| TFT Pin | ESP32 Pin |
|---------|-----------|
| LCD_RS | D35 (ADC1, input-only) |
| LCD_D1 | D34 (ADC1, input-only) |

### SD Card

| SD Pin | ESP32 Pin |
|--------|-----------|
| SD_SS | D5 |
| SD_DI | D33 |
| SD_DO | D25 |
| SD_SCK | D2 |

---

## Software Setup

### Requirements

- Arduino IDE
- ESP32 board package **v2.0.14** (important — newer versions may break things)
- The following libraries:
  - TFT_eSPI (latest, with manual fix — see below)
  - Adafruit TouchScreen (s60sc modified version from [s60sc/Adafruit_TouchScreen](https://github.com/s60sc/Adafruit_TouchScreen))
  - NimBLE-Arduino

### Arduino IDE Settings

```
Board:            LOLIN D32
Flash Frequency:  80MHz
Partition Scheme: Minimal SPIFFS
```

### TFT_eSPI Configuration (`User_Setup.h`)

```cpp
#define ILI9341_DRIVER
#define ESP32_PARALLEL

#define TFT_CS   27
#define TFT_DC   14
#define TFT_RST  26
#define TFT_WR   32
#define TFT_RD   13
#define TFT_BL   -1

#define TFT_D0   16
#define TFT_D1    4
#define TFT_D2   23
#define TFT_D3   22
#define TFT_D4   21
#define TFT_D5   19
#define TFT_D6   18
#define TFT_D7   17

#define TFT_MISO 25
#define TFT_MOSI 33
#define TFT_SCLK 2

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT
```

### Required Code Fix for TFT_eSPI

In your Arduino libraries folder, open `TFT_eSPI/Processors/TFT_eSPI_ESP32.c` and replace all 3 occurrences of:
```c
reg = gpio_input_get();
```
with:
```c
reg = REG_READ(GPIO_IN_REG);
```

---

## How to Use the Device

### Touch Zones

The screen is divided into **3 horizontal tap zones**. There are no physical buttons — everything is controlled by tapping these zones:

```
┌─────────────────────┐
│                     │
│    TAP = UP / PREV  │  ← Top third
│                     │
├─────────────────────┤
│                     │
│   TAP = SELECT / OK │  ← Middle third
│                     │
├─────────────────────┤
│                     │
│   TAP = DOWN / NEXT │  ← Bottom third
│                     │
└─────────────────────┘
```

- **Top zone** — scroll up / previous item
- **Middle zone** — select / confirm / enter
- **Bottom zone** — scroll down / next item

### Navigation

1. Power on the device — you will see a splash screen then the main menu
2. Tap the **top or bottom zone** to scroll through menu options
3. Tap the **middle zone** to select the highlighted option
4. While a scan or attack is running, **tap anywhere on the screen** to stop it and return to the menu

### Brightness Control

To adjust brightness while on the main menu or connected screen:
1. **Hold** the top or bottom zone for **1.5 seconds**
2. The brightness adjustment screen will appear
3. **Tap top** = brighter, **tap bottom** = dimmer
4. **Tap middle** or wait 3 seconds to save and exit

---

## Features

### WiFi

| Feature | Status | Notes |
|---------|--------|-------|
| Scan APs (Access Points) | ✅ Working | Displays SSID, BSSID, channel, RSSI |
| Scan Stations | ✅ Working | Shows connected client devices |
| Scan All (AP + Station) | ✅ Working | Combined scan |
| Beacon Sniff | ✅ Working | Captures beacon frames |
| Beacon Spam | ✅ Working | Broadcasts fake SSIDs |
| Raw Packet Capture | ✅ Working | Captures all raw frames, saves to SD |
| Packet Monitor | ✅ Working | Live graph of beacon/probe/deauth counts |
| Deauth Flood | ⚠️ Partial | Sends packets but may not disconnect devices (see Known Issues) |
| Targeted Deauth | ⚠️ Partial | Touch works, same disconnection issue |
| Channel Analyzer | ✅ Working | Shows channel activity |
| Evil Portal | ✅ Working | Captive portal |
| War Drive | ✅ Working | GPS-assisted AP logging |

### Bluetooth

| Feature | Status | Notes |
|---------|--------|-------|
| BLE Scan (All) | ✅ Working | Scans for all BLE devices |
| AirTag Detection | ✅ Working | Detects nearby AirTags |
| Flipper Zero Detection | ✅ Working | |
| BLE Spam (Apple/Google/Samsung/Windows) | ⚠️ Partial | Works but touch is unresponsive during spam (see Known Issues) |
| BT Analyzer | ✅ Working | |

### Storage & Other

| Feature | Status | Notes |
|---------|--------|-------|
| SD Card (boot) | ✅ Working | Mounts and reads/writes at startup |
| SD Card (during WiFi scan) | ⚠️ Partial | May fail during heavy WiFi operations |
| PCAP saving | ⚠️ Partial | May fail during active scans |
| Settings persistence | ✅ Working | Saved to SPIFFS |
| Serial CLI | ✅ Working | Full command line over USB at 115200 baud |
| GPS | ⚠️ Defined, no hardware | Module not connected |
| NeoPixel LED | ⚠️ Defined, no hardware | Not connected |

---

## Serial CLI

Connect via USB at **115200 baud**. Type `#help` to see all available commands. Some useful ones:

```
#scanap              — Scan for access points
#scanstation         — Scan for stations
#scanall             — Scan APs and stations simultaneously
#sniffraw            — Start raw packet capture
#stopscan            — Stop any running scan
#attack -t deauth    — Start deauth flood
#reboot              — Reboot the device
#settings            — View current settings
#ls                  — List SD card files
```

---

## Known Issues

### 1. Deauth Not Disconnecting Devices
**Symptom:** The deauth attack shows packets being sent (3000+/sec) but target devices do not disconnect.

**Root cause:** The ESP32's WiFi AP interface has its own MAC address (`00:70:07:1c:87:25` in this build). When injecting raw 802.11 management frames with `esp_wifi_80211_tx`, the radio silently drops frames whose source MAC doesn't match the AP interface MAC. The frame is built correctly but never actually transmitted over the air.

**What aireplay-ng does differently:** It sets the wireless interface MAC to match the target AP before injecting, so the radio accepts the frames.

**Status:** Fix identified but not yet stable — calling `esp_wifi_set_mac()` per burst loop without causing device freeze is still being worked on.

**Workaround:** Use aireplay-ng on a Linux machine with a monitor-mode capable adapter for reliable deauth attacks.

### 2. BLE Spam Touch Unresponsive
**Symptom:** When BLE spam is running, the touchscreen stops responding. The device does not freeze — it's still advertising — but you cannot stop the attack from the screen.

**Root cause:** The NimBLE library calls `NimBLEDevice::init()` and `NimBLEDevice::deinit()` on every spam cycle. Each call blocks the CPU for 50-100ms. Since the main loop (which handles touch) is blocked during these calls, touch events are missed.

**Status:** Throttling to 200ms intervals reduces the problem but doesn't fully fix it. The correct fix is to move BLE spam to Core 1 as a FreeRTOS task so it runs independently of the main loop.

**Workaround:** Use the serial CLI — type `#stopscan` to stop BLE spam even when the touchscreen is unresponsive.

### 3. SD Card Errors During WiFi Scan
**Symptom:** SD card works at boot, but during active WiFi scanning you may see SD write errors in the serial output. PCAP files may be incomplete or fail to save.

**Root cause:** The SD card uses SPI, and the ESP32's WiFi stack can cause SPI bus timing issues during heavy radio operations.

**Status:** Not yet investigated or fixed. The scan itself still works — only SD saving is affected.

---

## Code Changes from Stock Marauder v1.11.0

This build required several modifications to get the standard Marauder source running on this custom hardware. Here is a summary of every change:

### configs.h
- Added `MARAUDER_V4` hardware profile with correct feature flags
- Disabled `HAS_BATTERY` (no MAX17048 chip)
- Disabled `HAS_BUTTONS` (touch-only)
- Kept `HAS_C5_SD` for SD initialization
- Added SD pin definitions: MISO=25, MOSI=33, SCK=2, CS=5
- Added `#else` fallback for `MEM_LOWER_LIM`

### Display.cpp
- Added complete custom `updateTouch()` function using analog resistive touch with bilinear interpolation calibration grid (60 calibration points across the screen)
- Commented out both `tft.setTouch(calData)` calls (incompatible with analog touch)
- Added `touchToExit()` display helper

### esp32_marauder.ino
- Added touch task globals (`touch_x`, `touch_y`, `touch_pending`) for Core 1 touch polling
- Added `touchTask()` FreeRTOS task pinned to Core 1
- Added `xTaskCreatePinnedToCore` in `setup()` to launch touch task
- Fixed `TFT_BL` pinMode to check for -1 before setting
- Added `BL_SETUP()`, `BL_SET()`, `BL_PREVIEW()` macros with API compatibility for board package 2.x vs 3.x

### MenuFunctions.cpp
- Added `extern volatile` declarations for touch globals at top of file
- Replaced `display_obj.updateTouch()` call in `main()` with `touch_pending` check (reads from Core 1 task instead of polling directly)
- Fixed `checkAnalyzerButtons()` — removed broken `while(display_obj.updateTouch(...))` loop that was throwing away captured touch coordinates due to the `wasPressed` latch behavior
- Added touch-to-exit check in `WIFI_ATTACK_DEAUTH_TARGETED` block

### WiFiScan.cpp
- Fixed `EvilPortal.h` include issue (`static char index_html`)
- Fixed `ieee80211_raw_frame_sanity_check` weak attribute
- Removed duplicate `HardwareSerial Serial2` declaration in `GpsInterface.cpp`
- Added `vTaskDelay(1)` in `packetRateLoop` — fixes touch unresponsiveness during raw capture
- Added `vTaskDelay(1)` inside `packetMonitorMain` for loop — fixes packet monitor button unresponsiveness
- Fixed BSSID field in `sendDeauthFrame` second packet (bytes 16-21 were incorrectly set to client MAC instead of AP BSSID)
- Added `delay(100)` after `esp_wifi_start()` in `startWiFiAttacks` to fix `ESP_ERR_WIFI_NOT_INIT` on first frame
- Throttled BLE spam loop to 200ms intervals to reduce freezing

---

## Build Photos

*(Add photos of your hardware here)*

---

## Credits

- Original Marauder firmware: [justcallmekoko/ESP32Marauder](https://github.com/justcallmekoko/ESP32Marauder)
- Modified Adafruit TouchScreen library: [s60sc/Adafruit_TouchScreen](https://github.com/s60sc/Adafruit_TouchScreen)
- TFT_eSPI library: [Bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
