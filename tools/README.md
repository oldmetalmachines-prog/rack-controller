# Tools

Helper scripts for building, flashing, and testing firmware.

## Scripts

### Build & Flash

| Script | Description |
|--------|-------------|
| `build_all.sh` | Build all firmware targets locally |
| `flash_esp32_pio.sh` | Flash ESP32 devices using PlatformIO |
| `flash_esp32_idf.sh` | Flash ESP32 devices using ESP-IDF |

### Validation (HIL Testing)

| Script | Description |
|--------|-------------|
| `serial_watch.py` | Monitor serial output for boot contract |
| `mqtt_check.py` | Verify MQTT status publication |

### Maintenance

| Script | Description |
|--------|-------------|
| `lint.sh` | Run code linters |

## Usage Examples

### Local Development

```bash
# Build everything
./tools/build_all.sh

# Build without P4 (if you don't have ESP-IDF)
./tools/build_all.sh --skip-p4

# Flash ESP32-S3
./tools/flash_esp32_pio.sh \
    --project firmware/esp32_s3_pio \
    --env s3dev \
    --port /dev/ttyUSB0

# Flash CYD
./tools/flash_esp32_pio.sh \
    --project firmware/cyd_pio \
    --env cyd \
    --port /dev/ttyUSB1
```

### HIL Testing

```bash
# Watch serial for boot contract
./tools/serial_watch.py \
    --port /dev/ttyUSB0 \
    --timeout 30 \
    --expect '"selftest":"pass"'

# Check MQTT status
./tools/mqtt_check.py \
    --broker homelab.local \
    --topic lab/esp32-aabbccddeeff/status \
    --timeout 15
```

## Dependencies

### Python Scripts

```bash
pip install pyserial paho-mqtt
```

### PlatformIO

```bash
pip install platformio
```

### ESP-IDF (for P4)

Follow [ESP-IDF installation guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).

## Logs

All scripts write logs to `logs/<timestamp>/` by default. Log directories contain:
- `build.log` - Build output
- `flash.log` - Flash output
- `serial_output.log` - Captured serial data
- `mqtt_output.log` - Captured MQTT messages
