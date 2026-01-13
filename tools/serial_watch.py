#!/usr/bin/env python3
"""
serial_watch.py - Monitor serial port for expected output

Watches a serial port for a specific substring within a timeout period.
Used for HIL testing to validate firmware boot contract.

Usage:
    serial_watch.py --port <port> --timeout <seconds> --expect '<substring>'

Example:
    serial_watch.py --port /dev/ttyUSB0 --timeout 30 --expect '"selftest":"pass"'
"""

import argparse
import serial
import sys
import time
import os
from datetime import datetime


def create_log_dir():
    """Create timestamped log directory."""
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_dir = f"logs/{timestamp}"
    os.makedirs(log_dir, exist_ok=True)
    return log_dir


def watch_serial(port: str, baudrate: int, timeout: float, expect: str, log_dir: str) -> bool:
    """
    Watch serial port for expected substring.
    
    Args:
        port: Serial port path
        baudrate: Baud rate
        timeout: Maximum time to wait in seconds
        expect: Substring to search for
        log_dir: Directory to write logs
    
    Returns:
        True if expected substring found, False otherwise
    """
    captured_output = []
    log_file = os.path.join(log_dir, "serial_output.log")
    
    print(f"Opening serial port {port} at {baudrate} baud...")
    print(f"Watching for: {expect}")
    print(f"Timeout: {timeout} seconds")
    print("-" * 50)
    
    try:
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=0.1,  # Short timeout for non-blocking reads
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        return False
    
    start_time = time.time()
    found = False
    
    try:
        while (time.time() - start_time) < timeout:
            if ser.in_waiting > 0:
                try:
                    line = ser.readline().decode('utf-8', errors='replace').strip()
                    if line:
                        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                        captured_output.append(f"[{timestamp}] {line}")
                        print(f"[{timestamp}] {line}")
                        
                        if expect in line:
                            print("-" * 50)
                            print(f"SUCCESS: Found expected substring!")
                            found = True
                            break
                except Exception as e:
                    print(f"Read error: {e}")
            
            time.sleep(0.01)  # Small delay to prevent CPU spinning
        
        if not found:
            print("-" * 50)
            print(f"TIMEOUT: Expected substring not found within {timeout} seconds")
    
    finally:
        ser.close()
        
        # Write captured output to log file
        with open(log_file, 'w') as f:
            f.write(f"Serial Watch Log\n")
            f.write(f"Port: {port}\n")
            f.write(f"Expected: {expect}\n")
            f.write(f"Result: {'PASS' if found else 'FAIL'}\n")
            f.write("-" * 50 + "\n")
            f.write("\n".join(captured_output))
        
        print(f"Log saved to: {log_file}")
    
    return found


def main():
    parser = argparse.ArgumentParser(
        description="Watch serial port for expected output"
    )
    parser.add_argument(
        "--port", "-p",
        required=True,
        help="Serial port (e.g., /dev/ttyUSB0)"
    )
    parser.add_argument(
        "--baudrate", "-b",
        type=int,
        default=115200,
        help="Baud rate (default: 115200)"
    )
    parser.add_argument(
        "--timeout", "-t",
        type=float,
        default=30,
        help="Timeout in seconds (default: 30)"
    )
    parser.add_argument(
        "--expect", "-e",
        required=True,
        help="Substring to search for"
    )
    parser.add_argument(
        "--log-dir",
        help="Log directory (default: logs/<timestamp>)"
    )
    
    args = parser.parse_args()
    
    log_dir = args.log_dir or create_log_dir()
    os.makedirs(log_dir, exist_ok=True)
    
    success = watch_serial(
        port=args.port,
        baudrate=args.baudrate,
        timeout=args.timeout,
        expect=args.expect,
        log_dir=log_dir
    )
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
