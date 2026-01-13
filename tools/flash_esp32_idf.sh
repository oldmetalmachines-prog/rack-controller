#!/bin/bash
#
# flash_esp32_idf.sh - Flash ESP32 devices using ESP-IDF
#
# Usage: flash_esp32_idf.sh --project <path> --port <port>
#
# Example:
#   ./flash_esp32_idf.sh --project firmware/esp32_p4_idf --port /dev/ttyUSB_P4_TEST
#

set -e

# Default values
PROJECT=""
PORT=""
LOG_DIR=""
IDF_TARGET="esp32p4"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --project)
            PROJECT="$2"
            shift 2
            ;;
        --port)
            PORT="$2"
            shift 2
            ;;
        --log-dir)
            LOG_DIR="$2"
            shift 2
            ;;
        --target)
            IDF_TARGET="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 --project <path> --port <port> [--log-dir <path>] [--target <chip>]"
            echo ""
            echo "Options:"
            echo "  --project   Path to ESP-IDF project directory"
            echo "  --port      Serial port for flashing (e.g., /dev/ttyUSB0)"
            echo "  --log-dir   Directory for logs (default: logs/<timestamp>)"
            echo "  --target    IDF target chip (default: esp32p4)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Validate required arguments
if [[ -z "$PROJECT" ]]; then
    echo "Error: --project is required"
    exit 1
fi

if [[ -z "$PORT" ]]; then
    echo "Error: --port is required"
    exit 1
fi

# Setup logging
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
if [[ -z "$LOG_DIR" ]]; then
    LOG_DIR="logs/${TIMESTAMP}"
fi
mkdir -p "$LOG_DIR"

BUILD_LOG="$LOG_DIR/build.log"
FLASH_LOG="$LOG_DIR/flash.log"

echo "============================================"
echo "ESP-IDF Flash Script"
echo "============================================"
echo "Project: $PROJECT"
echo "Port: $PORT"
echo "Target: $IDF_TARGET"
echo "Log directory: $LOG_DIR"
echo "============================================"

# Check if project exists
if [[ ! -d "$PROJECT" ]]; then
    echo "Error: Project directory not found: $PROJECT"
    exit 1
fi

# Check if IDF_PATH is set
if [[ -z "$IDF_PATH" ]]; then
    echo "Warning: IDF_PATH not set, assuming ESP-IDF is in PATH"
fi

# Check if port exists
if [[ ! -e "$PORT" ]]; then
    echo "Warning: Port $PORT does not exist (yet)"
fi

# Set target if needed
echo ""
echo "[0/2] Setting target to $IDF_TARGET..."
idf.py -C "$PROJECT" set-target "$IDF_TARGET" 2>&1 | tee -a "$BUILD_LOG" || true

# Build
echo ""
echo "[1/2] Building firmware..."
echo "Build started at $(date)" >> "$BUILD_LOG"

if idf.py -C "$PROJECT" build 2>&1 | tee -a "$BUILD_LOG"; then
    echo "Build successful"
else
    echo "Build FAILED - see $BUILD_LOG"
    exit 1
fi

# Flash
echo ""
echo "[2/2] Flashing firmware..."
echo "Flash started at $(date)" > "$FLASH_LOG"

if idf.py -C "$PROJECT" -p "$PORT" flash 2>&1 | tee -a "$FLASH_LOG"; then
    echo "Flash successful"
else
    echo "Flash FAILED - see $FLASH_LOG"
    exit 1
fi

echo ""
echo "============================================"
echo "Flash completed successfully"
echo "Logs saved to: $LOG_DIR"
echo "============================================"
