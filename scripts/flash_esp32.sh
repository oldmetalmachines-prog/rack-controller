#!/usr/bin/env bash
set -euo pipefail

PORT="${1:-/dev/ttyUSB0}"
BAUD="${2:-921600}"
BIN="${3:-firmware/firmware.bin}"

if [ ! -e "${PORT}" ]; then
  echo "ERROR: Port not found: ${PORT}"
  echo "Available ports:"
  ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || true
  exit 1
fi

if [ ! -f "${BIN}" ]; then
  echo "ERROR: Firmware binary not found: ${BIN}"
  ls -la firmware || true
  exit 1
fi

echo "Flashing ESP32: port=${PORT} baud=${BAUD} bin=${BIN}"
esptool.py --port "${PORT}" --baud "${BAUD}" write_flash 0x0 "${BIN}"
echo "Flash done."
