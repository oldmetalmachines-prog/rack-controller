#!/usr/bin/env python3
"""
mqtt_check.py - Verify MQTT status message from device

Subscribes to an MQTT topic and checks for expected content.
Used for HIL testing to validate firmware MQTT publishing.

Usage:
    mqtt_check.py --broker <host> --topic <topic> --timeout <seconds>

Example:
    mqtt_check.py --broker homelab.local --topic lab/esp32-aabbccddeeff/status --timeout 15
"""

import argparse
import json
import sys
import time
import os
from datetime import datetime

try:
    import paho.mqtt.client as mqtt
except ImportError:
    print("Error: paho-mqtt not installed. Run: pip install paho-mqtt")
    sys.exit(1)


class MQTTChecker:
    """MQTT message checker for HIL testing."""
    
    def __init__(self, broker: str, port: int, topic: str, expect: str, timeout: float, log_dir: str):
        self.broker = broker
        self.port = port
        self.topic = topic
        self.expect = expect
        self.timeout = timeout
        self.log_dir = log_dir
        
        self.found = False
        self.messages = []
        self.client = None
    
    def on_connect(self, client, userdata, flags, rc, properties=None):
        """Callback when connected to broker."""
        if rc == 0:
            print(f"Connected to MQTT broker: {self.broker}:{self.port}")
            client.subscribe(self.topic)
            print(f"Subscribed to: {self.topic}")
        else:
            print(f"Connection failed with code: {rc}")
    
    def on_message(self, client, userdata, msg):
        """Callback when message received."""
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        payload = msg.payload.decode('utf-8', errors='replace')
        
        self.messages.append({
            "timestamp": timestamp,
            "topic": msg.topic,
            "payload": payload,
            "retained": msg.retain
        })
        
        print(f"[{timestamp}] Topic: {msg.topic}")
        print(f"           Payload: {payload}")
        print(f"           Retained: {msg.retain}")
        
        if self.expect in payload:
            print("-" * 50)
            print("SUCCESS: Found expected content in message!")
            self.found = True
    
    def check(self) -> bool:
        """
        Connect and check for expected message.
        
        Returns:
            True if expected message found, False otherwise
        """
        print(f"Connecting to MQTT broker {self.broker}:{self.port}...")
        print(f"Topic: {self.topic}")
        print(f"Expected content: {self.expect}")
        print(f"Timeout: {self.timeout} seconds")
        print("-" * 50)
        
        # Create client with callback API version
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        
        try:
            self.client.connect(self.broker, self.port, 60)
            self.client.loop_start()
            
            start_time = time.time()
            while (time.time() - start_time) < self.timeout and not self.found:
                time.sleep(0.1)
            
            if not self.found:
                print("-" * 50)
                print(f"TIMEOUT: Expected content not found within {self.timeout} seconds")
        
        except Exception as e:
            print(f"MQTT Error: {e}")
            return False
        
        finally:
            self.client.loop_stop()
            self.client.disconnect()
            self._write_log()
        
        return self.found
    
    def _write_log(self):
        """Write captured messages to log file."""
        log_file = os.path.join(self.log_dir, "mqtt_output.log")
        
        with open(log_file, 'w') as f:
            f.write("MQTT Check Log\n")
            f.write(f"Broker: {self.broker}:{self.port}\n")
            f.write(f"Topic: {self.topic}\n")
            f.write(f"Expected: {self.expect}\n")
            f.write(f"Result: {'PASS' if self.found else 'FAIL'}\n")
            f.write("-" * 50 + "\n")
            
            for msg in self.messages:
                f.write(f"\n[{msg['timestamp']}]\n")
                f.write(f"  Topic: {msg['topic']}\n")
                f.write(f"  Payload: {msg['payload']}\n")
                f.write(f"  Retained: {msg['retained']}\n")
        
        print(f"Log saved to: {log_file}")


def create_log_dir():
    """Create timestamped log directory."""
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_dir = f"logs/{timestamp}"
    os.makedirs(log_dir, exist_ok=True)
    return log_dir


def main():
    parser = argparse.ArgumentParser(
        description="Check MQTT topic for expected message"
    )
    parser.add_argument(
        "--broker", "-b",
        required=True,
        help="MQTT broker hostname or IP"
    )
    parser.add_argument(
        "--port", "-p",
        type=int,
        default=1883,
        help="MQTT broker port (default: 1883)"
    )
    parser.add_argument(
        "--topic", "-t",
        required=True,
        help="MQTT topic to subscribe to"
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=15,
        help="Timeout in seconds (default: 15)"
    )
    parser.add_argument(
        "--expect", "-e",
        default='"selftest":"pass"',
        help='Expected substring in message (default: \'"selftest":"pass"\')'
    )
    parser.add_argument(
        "--log-dir",
        help="Log directory (default: logs/<timestamp>)"
    )
    
    args = parser.parse_args()
    
    log_dir = args.log_dir or create_log_dir()
    os.makedirs(log_dir, exist_ok=True)
    
    checker = MQTTChecker(
        broker=args.broker,
        port=args.port,
        topic=args.topic,
        expect=args.expect,
        timeout=args.timeout,
        log_dir=log_dir
    )
    
    success = checker.check()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
