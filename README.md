# Rack Controller

[![Build](https://github.com/oldmetalmachines-prog/rack-controller/actions/workflows/build.yml/badge.svg)](https://github.com/oldmetalmachines-prog/rack-controller/actions/workflows/build.yml)
[![HIL Tests](https://github.com/oldmetalmachines-prog/rack-controller/actions/workflows/hil.yml/badge.svg)](https://github.com/oldmetalmachines-prog/rack-controller/actions/workflows/hil.yml)
[![License](https://img.shields.io/github/license/oldmetalmachines-prog/rack-controller)](LICENSE)

Automated firmware CI/CD pipeline for ESP32-based rack monitoring and control devices.

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

The repository includes a complete CI/CD pipeline:

- **[build.yml](.github/workflows/build.yml)** - Automated builds on push/PR with matrix strategy and caching
- **[hil.yml](.github/workflows/hil.yml)** - Hardware-in-the-loop testing on real devices
- **[issue_on_fail.yml](.github/workflows/issue_on_fail.yml)** - Auto-creates GitHub issues on workflow failures
- **[release_flash.yml](.github/workflows/release_flash.yml)** - Manual deployment to staging/production with approval gates

For complete pipeline documentation, see:
- [docs/WORKFLOW.md](docs/WORKFLOW.md) - Workflow details
- [docs/WORKFLOW_INTEGRATION.md](docs/WORKFLOW_INTEGRATION.md) - Complete integration guide

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

## Self-Hosted Runners

This project uses two self-hosted GitHub Actions runners:

1. **Build Runner** - Compiles firmware (ZimaBlade #1 or similar)
2. **HIL Runner** - Flashes and tests hardware (ZimaBlade #2 with USB access)

Quick setup:
```bash
cp .env.example .env
# Edit .env with your GitHub runner tokens
docker-compose up -d
```

See [docs/RUNNERS.md](docs/RUNNERS.md) for detailed setup instructions.
```

## License

MIT License - See LICENSE file for details.
