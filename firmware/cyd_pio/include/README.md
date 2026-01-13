# CYD (ESP32-2432S028R) Include Directory

This directory contains header files for the CYD rack controller firmware.

## Files

- `secrets.h.template` - Template for Wi-Fi and MQTT credentials
- `secrets.h` - Your local credentials (git-ignored, create from template)

## Setup

```bash
cp secrets.h.template secrets.h
# Edit secrets.h with your credentials
```

## Hardware: ESP32-2432S028R (Cheap Yellow Display)

### Display Specifications
- **Screen**: 2.8" ILI9341 TFT LCD
- **Resolution**: 320x240 pixels
- **Touch**: XPT2046 resistive touch controller
- **Interface**: SPI

### Pin Mapping

The TFT_eSPI library is configured via build flags in `platformio.ini`.

#### Display Pins (ILI9341)
| Function | GPIO |
|----------|------|
| MISO     | 12   |
| MOSI     | 13   |
| SCLK     | 14   |
| CS       | 15   |
| DC       | 2    |
| RST      | -1 (not connected) |
| Backlight| 21   |

#### Touch Pins (XPT2046)
| Function | GPIO |
|----------|------|
| T_CS     | 33   |
| T_IRQ    | 36 (optional) |

Note: Touch shares MISO, MOSI, SCLK with display.

### Additional CYD Peripherals

| Peripheral | GPIO | Notes |
|------------|------|-------|
| RGB LED (R)| 4    | Active LOW |
| RGB LED (G)| 16   | Active LOW |
| RGB LED (B)| 17   | Active LOW |
| LDR (Light)| 34   | ADC input |
| SD Card CS | 5    | If using SD slot |

### TFT_eSPI Configuration

The display configuration is set via build flags in `platformio.ini` to avoid modifying library files. Key settings:

```
-DUSER_SETUP_LOADED=1      # Use our custom config
-DILI9341_2_DRIVER=1       # ILI9341 driver variant
-DTFT_RGB_ORDER=TFT_BGR    # Color order (may vary by batch)
```

If colors appear inverted, add `-DTFT_INVERSION_ON` to build flags.

### Calibration

Touch calibration values may need adjustment. Run calibration routine and update values in firmware if touch is inaccurate.

## Troubleshooting

### White/blank screen
- Check backlight pin (GPIO 21) is set HIGH
- Verify SPI pin connections
- Try reducing SPI frequency

### Inverted colors
- Add `-DTFT_INVERSION_ON` to build flags

### Touch not working
- Check T_CS (GPIO 33) connection
- Verify touch SPI frequency (2.5MHz recommended)
