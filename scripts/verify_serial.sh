#!/usr/bin/env bash
set -euo pipefail

PORT="${1:-/dev/ttyUSB0}"
SECONDS_TO_READ="${2:-8}"

python3 - <<'PY'
import os, time, sys
import serial

port = os.environ.get("PORT", "/dev/ttyUSB0")
seconds = int(os.environ.get("SECONDS", "8"))

print(f"Opening {port} for {seconds}s...")
ser = serial.Serial(port, 115200, timeout=0.2)
t0 = time.time()
buf = []
while time.time() - t0 < seconds:
    line = ser.readline()
    if line:
        buf.append(line.decode(errors="ignore").rstrip())
ser.close()

print("---- SERIAL OUTPUT (last 80 lines) ----")
for l in buf[-80:]:
    print(l)
print("--------------------------------------")

# Basic heuristic: customize to your firmware output
ok = any(("ready" in l.lower()) or ("boot" in l.lower()) for l in buf)
sys.exit(0 if ok else 2)
PY
