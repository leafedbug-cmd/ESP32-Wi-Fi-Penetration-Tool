# ESP32 Wi-Fi Penetration Tool

Modular ESP-IDF application that turns the ESP32 platform into a portable Wi-Fi
assessment and demonstration rig. The firmware exposes a management access point
with a web UI where you can launch typical 802.11 attacks (PMKID, handshake
collection, deauthentication-based DoS, rogue AP, etc.), export captures, and
extend the toolset with new payloads.

<p align="center">
  <img src="doc/images/logo.png" alt="Logo" width="240px">
</p>

## Highlights
- PMKID, WPA/WPA2 handshake capture, parsing and export to PCAP/HCCAPX
- Deauthentication & DoS attacks via stack bypass or rogue AP replication
- Passive sniffing mode for opportunistic handshake collection
- Built-in management AP (`ManagementAP` / `mgmtadmin`) and responsive web UI
- Pluggable attack/component framework for experimenting with new ideas
- Optional WS2812 status LED pulse on ESP32-S3 boards/Flipper Wi-Fi Dev Board

## Quick Start
1. Clone the repository and install either [PlatformIO](https://platformio.org/)
   or the matching [ESP-IDF toolchain](https://docs.espressif.com/projects/esp-idf).
2. Choose a build method:

### PlatformIO (recommended for newcomers)
```bash
pio run              # build
pio run -t upload    # flash (set upload_port/monitor_port in platformio.ini)
pio device monitor   # view serial logs
```
The default environment targets `esp32-s3-devkitc-1`. Adjust `platformio.ini`
to fit your board, baud rate, or preferred COM port.

### ESP-IDF (upstream workflow)
- Project developed and verified against **ESP-IDF v4.1** (commit
  `5ef1b390026270503634ac3ec9f1ec2e364e23b2`) and **ESP-IDF v4.4**.
- Use `idf.py set-target esp32s2` if you are building for the Flipper Zero Wi-Fi
  Dev Board, otherwise the default ESP32 target is sufficient.
```bash
idf.py build               # compile
idf.py -p <PORT> flash     # replace <PORT> with your serial adapter
```

## Usage
1. Power the ESP32 module and press reset (Flipper Zero Wi-Fi Dev Board handles
   this through GPIO).
2. Wait for the management AP to appear (`ManagementAP` / `mgmtadmin`).
3. Browse to `http://192.168.4.1` to access the controller UI.
4. Launch the desired attack, collect exports, or configure new targets.

![Web client UI](doc/images/ui-config.png)

## Flashing Options
- **Prebuilt binaries**: Download from the [releases](https://github.com/FroggMaster/ESP32-Wi-Fi-Penetration-Tool/releases/new)
  page, place your ESP32 into download mode (hold **BOOT** while powering on),
  and run `Flash.sh` (Linux/macOS) or `Flash.bat` (Windows).
- **ESP-IDF CLI**: `idf.py -p <PORT> flash`
- **esptool**:
  ```bash
  esptool.py -p /dev/ttyS5 -b 115200 --after hard_reset write_flash \
      --flash_mode dio --flash_freq 40m --flash_size detect \
      0x8000  build/partition_table/partition-table.bin \
      0x1000  build/bootloader/bootloader.bin \
      0x10000 build/esp32-wifi-penetration-tool.bin
  ```
  Replace the serial port with `COMx` on Windows.

## WS2812 Status LED
The optional `led_strip_ws2812` component drives the built-in WS2812 pixel
present on many ESP32-S3 boards. During boot `app_main` will:
- Initialize the LED on GPIO 48 via the RMT peripheral
- Pulse the LED blue while the firmware is running

You can reuse `led_strip_set_color` or `led_strip_pulse` inside your own attack
implementations to indicate progress, success, errors, etc. See
`components/led_strip_ws2812/README.md` for details and tunables.

## Repository Layout
- `main/` – attack implementations plus orchestration logic
- `components/` – reusable modules (Wi-Fi controller, webserver, parsers, WS2812)
- `doc/` – architecture notes, attack theory, images, and Doxygen output target
- `platformio.ini` – PlatformIO environment for a GitHub-friendly workflow
- `sdkconfig.defaults` – baseline configuration shared by both build systems

The theory behind each attack is described in `doc/ATTACKS_THEORY.md`, and every
component contains its own README with extra implementation notes.

## Documentation & Contribution
- Generate API docs: `doxygen` ⇒ `doc/api/html/index.html`
- Contributions and refactors are welcome. Please keep Doxygen comments up to
  date and document new components or attacks thoroughly.

## Disclaimer
This project demonstrates Wi-Fi vulnerabilities for educational purposes only.
Operate it exclusively on networks you own or have explicit permission to test.
