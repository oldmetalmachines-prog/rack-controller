#!/bin/bash
#
# build_all.sh - Build all firmware targets locally
#
# Usage: ./build_all.sh [--skip-p4]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

SKIP_P4=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-p4)
            SKIP_P4=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [--skip-p4]"
            echo ""
            echo "Options:"
            echo "  --skip-p4   Skip ESP32-P4 build (requires ESP-IDF)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "============================================"
echo "Building All Firmware Targets"
echo "============================================"
echo ""

# Track results
RESULTS=()

# Build ESP32-S3 (PlatformIO)
echo "[1/3] Building ESP32-S3 (PlatformIO)..."
echo "----------------------------------------------"
if pio run -d "$REPO_ROOT/firmware/esp32_s3_pio" -e s3dev; then
    echo "✓ ESP32-S3 build successful"
    RESULTS+=("ESP32-S3: PASS")
else
    echo "✗ ESP32-S3 build FAILED"
    RESULTS+=("ESP32-S3: FAIL")
fi
echo ""

# Build CYD (PlatformIO)
echo "[2/3] Building CYD (PlatformIO)..."
echo "----------------------------------------------"
if pio run -d "$REPO_ROOT/firmware/cyd_pio" -e cyd; then
    echo "✓ CYD build successful"
    RESULTS+=("CYD: PASS")
else
    echo "✗ CYD build FAILED"
    RESULTS+=("CYD: FAIL")
fi
echo ""

# Build ESP32-P4 (ESP-IDF) - optional
echo "[3/3] Building ESP32-P4 (ESP-IDF)..."
echo "----------------------------------------------"
if [[ "$SKIP_P4" == "true" ]]; then
    echo "⊘ ESP32-P4 build skipped (--skip-p4 flag)"
    RESULTS+=("ESP32-P4: SKIPPED")
elif [[ -z "$IDF_PATH" ]]; then
    echo "⊘ ESP32-P4 build skipped (IDF_PATH not set)"
    RESULTS+=("ESP32-P4: SKIPPED (no IDF)")
else
    if idf.py -C "$REPO_ROOT/firmware/esp32_p4_idf" build; then
        echo "✓ ESP32-P4 build successful"
        RESULTS+=("ESP32-P4: PASS")
    else
        echo "✗ ESP32-P4 build FAILED"
        RESULTS+=("ESP32-P4: FAIL")
    fi
fi
echo ""

# Summary
echo "============================================"
echo "Build Summary"
echo "============================================"
for result in "${RESULTS[@]}"; do
    echo "  $result"
done
echo "============================================"

# Exit with error if any builds failed
for result in "${RESULTS[@]}"; do
    if [[ "$result" == *"FAIL"* ]]; then
        exit 1
    fi
done

exit 0
