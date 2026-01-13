# ESP32-S3 Include Directory

This directory contains header files for the ESP32-S3 rack controller firmware.

## Files

- `secrets.h.template` - Template for Wi-Fi and MQTT credentials
- `secrets.h` - Your local credentials (git-ignored, create from template)

## Setup

```bash
cp secrets.h.template secrets.h
# Edit secrets.h with your credentials
```

## Adding Custom Headers

Place any additional header files here. They will be automatically included in the build.
