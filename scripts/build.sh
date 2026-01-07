#!/usr/bin/env bash
set -euo pipefail

echo "Build placeholder (replace with PlatformIO / ESP-IDF build)."
mkdir -p firmware
# For now just create a placeholder artifact so the pipeline proves out:
echo "dummy" > firmware/firmware.bin
echo "Build complete -> firmware/firmware.bin"
