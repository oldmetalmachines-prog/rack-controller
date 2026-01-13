# Device Pool

Registry of test devices used for Hardware-in-the-Loop (HIL) testing.

## Overview

The HIL testing system uses dedicated test devices connected to ZimaBlade #2 (the HIL runner). Each device has a stable port mapping via udev rules.

## Test Devices

### ESP32-S3 Test Device

| Property | Value |
|----------|-------|
| **Device** | ESP32-S3-DevKitC-1 |
| **USB Port** | ZimaBlade #2, Port 1 |
| **Symlink** | `/dev/ttyUSB_ESP32_S3_TEST` |
| **Serial Number** | `PLACEHOLDER_S3_SERIAL` |
| **USB ID** | `303a:1001` |
| **Purpose** | ESP32-S3 firmware validation |

### CYD Test Device

| Property | Value |
|----------|-------|
| **Device** | ESP32-2432S028R (Cheap Yellow Display) |
| **USB Port** | ZimaBlade #2, Port 2 |
| **Symlink** | `/dev/ttyUSB_CYD_TEST` |
| **Serial Number** | `PLACEHOLDER_CYD_SERIAL` |
| **USB ID** | `1a86:7523` (CH340) |
| **Purpose** | CYD firmware validation |

### ESP32-P4 Test Device (Placeholder)

| Property | Value |
|----------|-------|
| **Device** | ESP32-P4 DevKit (TBD) |
| **USB Port** | ZimaBlade #2, Port 3 |
| **Symlink** | `/dev/ttyUSB_P4_TEST` |
| **Serial Number** | `PLACEHOLDER_P4_SERIAL` |
| **USB ID** | `303a:XXXX` |
| **Purpose** | ESP32-P4 firmware validation |
| **Status** | 🚧 Not yet acquired |

## udev Rules

Create `/etc/udev/rules.d/99-esp32-devices.rules` on ZimaBlade #2:

```udev
# Rack Controller Test Devices
# 
# To find device serial numbers:
#   udevadm info -a /dev/ttyUSB0 | grep -E "serial|idVendor|idProduct"

# ESP32-S3 Test Device
SUBSYSTEM=="tty", ATTRS{idVendor}=="303a", ATTRS{idProduct}=="1001", \
  ATTRS{serial}=="PLACEHOLDER_S3_SERIAL", \
  SYMLINK+="ttyUSB_ESP32_S3_TEST", MODE="0666"

# CYD Test Device (CH340 USB-Serial)
SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", \
  ATTRS{serial}=="PLACEHOLDER_CYD_SERIAL", \
  SYMLINK+="ttyUSB_CYD_TEST", MODE="0666"

# ESP32-P4 Test Device (placeholder)
# SUBSYSTEM=="tty", ATTRS{idVendor}=="303a", ATTRS{idProduct}=="XXXX", \
#   ATTRS{serial}=="PLACEHOLDER_P4_SERIAL", \
#   SYMLINK+="ttyUSB_P4_TEST", MODE="0666"
```

### Finding Serial Numbers

```bash
# Plug in device, then:
dmesg | tail -5  # Find which ttyUSB was assigned

# Get device details
udevadm info -a /dev/ttyUSB0 | grep -E "ATTRS{serial}|ATTRS{idVendor}|ATTRS{idProduct}"

# Example output:
#   ATTRS{idVendor}=="303a"
#   ATTRS{idProduct}=="1001"
#   ATTRS{serial}=="ABC123456789"
```

### Applying Rules

```bash
# Reload udev rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# Verify symlinks
ls -la /dev/ttyUSB*
```

## Physical Setup

### Wiring Diagram

```
ZimaBlade #2 (HIL Runner)
├── USB Port 1 ─── ESP32-S3-DevKitC-1
├── USB Port 2 ─── CYD (ESP32-2432S028R)
├── USB Port 3 ─── [Reserved for ESP32-P4]
└── Ethernet ───── Lab Network (MQTT access)
```

### Power Considerations

- Each ESP32 device draws ~500mA during WiFi TX
- USB hub may be needed if running multiple devices
- Use powered USB hub for reliability

### Device Labeling

Recommend labeling physical devices:
- `S3-TEST-01` on ESP32-S3
- `CYD-TEST-01` on CYD
- Include QR code linking to this document

## Adding New Test Devices

1. **Physical connection**: Connect device to ZimaBlade #2
2. **Find serial**: Use `udevadm info` to get USB serial number
3. **Create udev rule**: Add to `/etc/udev/rules.d/99-esp32-devices.rules`
4. **Reload rules**: `sudo udevadm control --reload-rules && sudo udevadm trigger`
5. **Update this document**: Add device entry above
6. **Update workflows**: Add new port to `.github/workflows/hil.yml`

## Staging vs Production Devices

| Type | Purpose | Flash Method |
|------|---------|--------------|
| **Test** | HIL validation | Automatic via CI |
| **Staging** | Pre-production validation | Manual via `release_flash.yml` |
| **Production** | Deployed devices | Manual, out of band |

The CI pipeline only flashes **test** devices automatically. Staging and production flashing requires manual workflow dispatch.

## Troubleshooting

### Device Not Detected

```bash
# Check USB connection
lsusb

# Check kernel messages
dmesg | tail -20

# Try different USB port/cable
```

### Wrong Device Gets Flashed

- Verify serial numbers in udev rules
- Check symlinks point to correct devices
- Use `udevadm test` to debug rule matching

### Intermittent Connection

- Check USB cable quality (data cables, not charge-only)
- Try powered USB hub
- Check for EMI interference from nearby equipment
