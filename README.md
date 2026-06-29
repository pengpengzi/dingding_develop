# dingding_develop

ESP32 Wi-Fi identity simulator for controlled testing. The firmware creates a configurable 2.4 GHz SoftAP and can set the AP interface MAC/BSSID to a user-provided value. It is based on the DingDing WiFi workflow and adapted for classic ESP32 development boards.

## Features

- Configure target SSID, password, and AP MAC/BSSID from a captive-style web page.
- Persist configuration in ESP32 NVS so the device can boot directly into the simulated AP.
- Validate MAC address format and reject multicast MAC addresses.
- Use LittleFS for the configuration UI.
- Avoid advertising the simulated AP as an internet gateway, so phones can prefer cellular data where the OS supports it.
- Long-press the configured button to clear saved Wi-Fi settings and return to setup mode.

## Hardware

- ESP32 classic development board, for example ESP32 DevKit using `esp32dev`.
- Default LED pin: `GPIO2`.
- Default clear-settings button pin: `GPIO0`, often the BOOT button.

The firmware only supports ESP32 2.4 GHz Wi-Fi. It cannot emulate 5 GHz, Wi-Fi 6/802.11ax, channel width, or real router forwarding behavior.

## Build And Upload

Install PlatformIO, then run:

```bash
pio run -e esp32dev
pio run -e esp32dev -t upload
pio run -e esp32dev -t uploadfs
```

If your serial port is not auto-detected, pass it on the command line:

```bash
pio run -e esp32dev -t upload --upload-port <serial-port>
pio run -e esp32dev -t uploadfs --upload-port <serial-port>
```

## Usage

1. Power on the ESP32.
2. Connect to the default hotspot `WIFI MANAGER`.
3. Open `http://192.168.4.1`.
4. Enter the target SSID, optional password, and target AP MAC/BSSID.
5. Submit the form. The ESP32 restarts the SoftAP using the saved settings.

The MAC/BSSID field is the target AP's MAC address, not the client device's Wi-Fi MAC address.

## Privacy

The repository does not include saved SSID, password, or BSSID values. Runtime configuration is stored only in ESP32 NVS after the web form is submitted on the device.
