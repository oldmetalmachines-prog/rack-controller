# Pin Maps

Hardware pin assignments for all supported targets.

## ESP32-S3 (esp32_s3_pio)

### DevKitC-1 Default Pinout

| Function | GPIO | Notes |
|----------|------|-------|
| **USB** | | |
| USB D+ | 20 | Native USB |
| USB D- | 19 | Native USB |
| **UART** | | |
| UART0 TX | 43 | Debug/programming |
| UART0 RX | 44 | Debug/programming |
| **SPI** | | |
| SPI2 MOSI | 11 | Available |
| SPI2 MISO | 13 | Available |
| SPI2 CLK | 12 | Available |
| SPI2 CS | 10 | Available |
| **I2C** | | |
| I2C SDA | 8 | Recommended |
| I2C SCL | 9 | Recommended |
| **ADC** | | |
| ADC1_CH0 | 1 | Available |
| ADC1_CH1 | 2 | Available |
| ADC1_CH2 | 3 | Available |
| ADC1_CH3 | 4 | Available |
| **Strapping** | | |
| BOOT | 0 | Boot mode select |
| **RGB LED** | | |
| WS2812 | 48 | Addressable RGB (DevKitC-1) |

### Reserved Pins (Do Not Use)

| GPIO | Reason |
|------|--------|
| 19, 20 | USB D-/D+ |
| 26-32 | SPI Flash (PSRAM on some variants) |
| 43, 44 | UART0 (programming) |

### Application Pin Assignments

Define your application-specific pins here:

```cpp
// Example rack controller pins
#define PIN_STATUS_LED    48  // WS2812 RGB
#define PIN_RELAY_1       5
#define PIN_RELAY_2       6
#define PIN_TEMP_SENSOR   4   // OneWire
#define PIN_I2C_SDA       8
#define PIN_I2C_SCL       9
```

---

## CYD - ESP32-2432S028R (cyd_pio)

### Display Pins (ILI9341)

| Function | GPIO | Notes |
|----------|------|-------|
| TFT_MISO | 12 | SPI shared with touch |
| TFT_MOSI | 13 | SPI shared with touch |
| TFT_SCLK | 14 | SPI shared with touch |
| TFT_CS | 15 | Display chip select |
| TFT_DC | 2 | Data/Command |
| TFT_RST | -1 | Not connected (tied high) |
| TFT_BL | 21 | Backlight (PWM capable) |

### Touch Pins (XPT2046)

| Function | GPIO | Notes |
|----------|------|-------|
| TOUCH_CS | 33 | Touch chip select |
| TOUCH_IRQ | 36 | Touch interrupt (optional) |
| TOUCH_MISO | 12 | Shared with display |
| TOUCH_MOSI | 13 | Shared with display |
| TOUCH_CLK | 14 | Shared with display |

### RGB LED (Active LOW)

| Function | GPIO | Notes |
|----------|------|-------|
| LED_RED | 4 | Active LOW |
| LED_GREEN | 16 | Active LOW |
| LED_BLUE | 17 | Active LOW |

### Other Peripherals

| Function | GPIO | Notes |
|----------|------|-------|
| LDR | 34 | Light sensor (ADC) |
| SD_CS | 5 | SD card chip select |
| SD_MISO | 19 | SD card SPI |
| SD_MOSI | 23 | SD card SPI |
| SD_CLK | 18 | SD card SPI |
| SPEAKER | 26 | Audio output (DAC) |

### CYD Pin Summary Diagram

```
                    ESP32-2432S028R
                 ┌─────────────────────┐
                 │  ┌───────────────┐  │
                 │  │               │  │
                 │  │   ILI9341     │  │
                 │  │   320x240     │  │
                 │  │               │  │
                 │  │   XPT2046     │  │
                 │  │   Touch       │  │
                 │  └───────────────┘  │
    USB-C ───────│                     │
                 │  [RGB LED]          │
                 │   R=4 G=16 B=17     │
                 │                     │
                 │  [LDR] GPIO34       │
                 │  [SD]  GPIO5        │
                 │  [SPK] GPIO26       │
                 └─────────────────────┘
```

### Available GPIOs for Expansion

After display, touch, and peripherals, these GPIOs may be available:

| GPIO | Notes |
|------|-------|
| 0 | Boot button (strapping pin) |
| 22 | Available |
| 25 | DAC (if not using speaker) |
| 27 | Available |
| 32 | Available |
| 35 | Input only |

**Warning:** Available pins vary by CYD revision. Always verify with your specific board.

---

## ESP32-P4 (esp32_p4_idf) - Placeholder

> ⚠️ **Note:** ESP32-P4 pinout is placeholder pending hardware availability.

### Expected Features

- Dual-core RISC-V
- USB OTG
- Ethernet MAC
- Enhanced peripherals

### Placeholder Pin Assignments

```c
// To be determined when hardware is available
#define PIN_UART_TX     1   // Placeholder
#define PIN_UART_RX     2   // Placeholder
```

---

## Common Interfaces

### MQTT Configuration

All targets use these MQTT settings (configured in `secrets.h`):

| Setting | Default | Notes |
|---------|---------|-------|
| Broker | `homelab.local` | Hostname or IP |
| Port | `1883` | Standard MQTT |
| Topic Prefix | `lab/` | Device status topic |

### Boot Contract Output

All targets output boot contract on UART0 at 115200 baud:

```
{"device":"<mac>","fw":"<sha>","target":"<s3|cyd|p4>","selftest":"pass"}
```

---

## Adding Custom Pins

When adding new hardware features:

1. Check pin availability in tables above
2. Avoid strapping pins (GPIO 0, etc.)
3. Consider ADC/DAC requirements
4. Document in this file
5. Update firmware `#define` statements
6. Test on physical hardware before CI

### Pin Configuration Template

```cpp
// config_pins.h - Application pin definitions
#ifndef CONFIG_PINS_H
#define CONFIG_PINS_H

// Target-specific pins
#if defined(TARGET_S3)
  #define PIN_CUSTOM_1    5
  #define PIN_CUSTOM_2    6
#elif defined(TARGET_CYD)
  #define PIN_CUSTOM_1    22
  #define PIN_CUSTOM_2    27
#elif defined(TARGET_P4)
  #define PIN_CUSTOM_1    TBD
  #define PIN_CUSTOM_2    TBD
#endif

#endif // CONFIG_PINS_H
```
