# Rack Controller

Automated firmware CI/CD pipeline for ESP32-based rack monitoring and control devices.

## Overview

This repository manages firmware for multiple ESP32 targets with a CI/CD pipeline featuring:
- **Automated builds** via GitHub Actions on self-hosted runners
- **Hardware-in-the-Loop (HIL) testing** with real device flashing and validation
- **Structured failure reporting** via auto-generated GitHub Issues
- **Manual deployment gates** for staging/production flashing

## Supported Targets

| Target | Platform | Framework | Status |
|--------|----------|-----------|--------|
| ESP32-S3 | PlatformIO | Arduino | ✅ Active |
| CYD (ESP32-2432S028R) | PlatformIO | Arduino | ✅ Active |
| ESP32-P4 | ESP-IDF | Native | 🚧 Placeholder |

## Boot Contract

All firmware targets implement a standard boot contract for automated validation:

### Serial Output (within 10 seconds of boot)
```json
{"device":"<mac-derived-id>","fw":"<git-sha>","target":"<s3|cyd|p4>","selftest":"pass"}
```

On failure:
```json
{"device":"<mac-derived-id>","fw":"<git-sha>","target":"<s3|cyd|p4>","selftest":"fail","err":"<error-code>"}
```

### MQTT Status (retained)
Topic: `lab/<device-id>/status`
```json
{"device":"<id>","fw":"<sha>","target":"<target>","selftest":"pass|fail","ts":<epoch>}
```

## Quick Start

### Local Development

1. Copy secrets template:
   ```bash
   cp firmware/esp32_s3_pio/include/secrets.h.template firmware/esp32_s3_pio/include/secrets.h
   cp firmware/cyd_pio/include/secrets.h.template firmware/cyd_pio/include/secrets.h
   ```

2. Edit `secrets.h` with your Wi-Fi and MQTT credentials.

3. Build all targets:
   ```bash
   ./tools/build_all.sh
   ```

4. Flash a specific target:
   ```bash
   ./tools/flash_esp32_pio.sh --project firmware/esp32_s3_pio --env s3dev --port /dev/ttyUSB0
   ```

### CI/CD Pipeline

See [docs/WORKFLOW.md](docs/WORKFLOW.md) for the complete pipeline documentation.

## Directory Structure

```
rack-controller/
├── firmware/
│   ├── esp32_s3_pio/      # ESP32-S3 PlatformIO project
│   ├── cyd_pio/           # CYD (ESP32-2432S028R) PlatformIO project
│   └── esp32_p4_idf/      # ESP32-P4 ESP-IDF project (placeholder)
├── tools/
│   ├── flash_esp32_pio.sh # PlatformIO flash script
│   ├── flash_esp32_idf.sh # ESP-IDF flash script
│   ├── serial_watch.py    # Serial output validator
│   ├── mqtt_check.py      # MQTT status validator
│   └── build_all.sh       # Local build helper
├── .github/workflows/
│   ├── build.yml          # CI build workflow
│   ├── hil.yml            # Hardware-in-the-loop tests
│   ├── issue_on_fail.yml  # Auto-create issues on failure
│   └── release_flash.yml  # Manual deployment workflow
└── docs/
    ├── DEVICE_POOL.md     # Test device registry
    ├── RUNNERS.md         # Self-hosted runner setup
    ├── WORKFLOW.md        # Pipeline documentation
    └── PINMAPS.md         # Hardware pin assignments
```

## License

MIT License - See LICENSE file for details.
