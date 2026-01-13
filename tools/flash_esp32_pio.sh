#!/bin/bash
#
# flash_esp32_pio.sh - Flash ESP32 devices using PlatformIO
#
# Usage: flash_esp32_pio.sh --project <path> --env <env> --port <port>
#
# Example:
#   ./flash_esp32_pio.sh --project firmware/esp32_s3_pio --env s3dev --port /dev/ttyUSB_ESP32_TEST
#

set -e

# Default values
PROJECT=""
ENV=""
PORT=""
LOG_DIR=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --project)
            PROJECT="$2"
            shift 2
            ;;
        --env)
            ENV="$2"
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
        -h|--help)
            echo "Usage: $0 --project <path> --env <env> --port <port> [--log-dir <path>]"
            echo ""
            echo "Options:"
            echo "  --project   Path to PlatformIO project directory"
            echo "  --env       PlatformIO environment to build/flash"
            echo "  --port      Serial port for flashing (e.g., /dev/ttyUSB0)"
            echo "  --log-dir   Directory for logs (default: logs/<timestamp>)"
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

if [[ -z "$ENV" ]]; then
    echo "Error: --env is required"
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
echo "PlatformIO Flash Script"
echo "============================================"
echo "Project: $PROJECT"
echo "Environment: $ENV"
echo "Port: $PORT"
echo "Log directory: $LOG_DIR"
echo "============================================"

# Check if project exists
if [[ ! -d "$PROJECT" ]]; then
    echo "Error: Project directory not found: $PROJECT"
    exit 1
fi

# Check if port exists
if [[ ! -e "$PORT" ]]; then
    echo "Warning: Port $PORT does not exist (yet)"
fi

# Build
echo ""
echo "[1/2] Building firmware..."
echo "Build started at $(date)" > "$BUILD_LOG"

if pio run -d "$PROJECT" -e "$ENV" 2>&1 | tee -a "$BUILD_LOG"; then
    echo "Build successful"
else
    echo "Build FAILED - see $BUILD_LOG"
    exit 1
fi

# Flash
echo ""
echo "[2/2] Flashing firmware..."
echo "Flash started at $(date)" > "$FLASH_LOG"

if pio run -d "$PROJECT" -e "$ENV" -t upload --upload-port "$PORT" 2>&1 | tee -a "$FLASH_LOG"; then
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
