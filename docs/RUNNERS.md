# Self-Hosted Runner Setup

This document describes how to configure self-hosted GitHub Actions runners on ZimaBlades for the rack-controller CI/CD pipeline.

## Runner Roles

| Runner | Host | Labels | Purpose |
|--------|------|--------|---------|
| ZimaBlade #1 | Build Server | `self-hosted, linux, x64, build` | Compile firmware |
| ZimaBlade #2 | HIL Server | `self-hosted, linux, x64, hil, flash` | Flash & test devices |

## Prerequisites

### Both Runners

- Ubuntu 22.04+ or Debian 12+
- Docker (optional, for containerized builds)
- Git

### Build Runner (ZimaBlade #1)

- Python 3.11+
- PlatformIO Core CLI
- ESP-IDF (for P4, when available)

### HIL Runner (ZimaBlade #2)

- Python 3.11+
- PlatformIO Core CLI
- USB serial access (user in `dialout` group)
- Network access to MQTT broker
- Physical USB connections to test devices

## Installation

### 1. Install GitHub Actions Runner

```bash
# Create runner directory
mkdir -p ~/actions-runner && cd ~/actions-runner

# Download runner (check GitHub for latest version)
curl -o actions-runner-linux-x64-2.311.0.tar.gz -L \
  https://github.com/actions/runner/releases/download/v2.311.0/actions-runner-linux-x64-2.311.0.tar.gz

# Extract
tar xzf ./actions-runner-linux-x64-2.311.0.tar.gz

# Configure (get token from GitHub repo Settings > Actions > Runners)
./config.sh --url https://github.com/YOUR_USER/rack-controller \
            --token YOUR_TOKEN
```

### 2. Set Runner Labels

During configuration, set appropriate labels:

**Build Runner:**
```bash
./config.sh --url https://github.com/YOUR_USER/rack-controller \
            --token YOUR_TOKEN \
            --labels self-hosted,linux,x64,build
```

**HIL Runner:**
```bash
./config.sh --url https://github.com/YOUR_USER/rack-controller \
            --token YOUR_TOKEN \
            --labels self-hosted,linux,x64,hil,flash
```

### 3. Install as Service

```bash
sudo ./svc.sh install
sudo ./svc.sh start
sudo ./svc.sh status
```

### 4. Install Dependencies

**Build Runner:**
```bash
# Python and PlatformIO
sudo apt update
sudo apt install -y python3 python3-pip python3-venv
pip3 install platformio

# Verify
pio --version
```

**HIL Runner:**
```bash
# Python and dependencies
sudo apt install -y python3 python3-pip
pip3 install platformio pyserial paho-mqtt

# Add user to dialout for serial access
sudo usermod -aG dialout $USER
# Logout and login for group change to take effect

# Verify serial access
ls -la /dev/ttyUSB*
```

### 5. ESP-IDF Setup (Build Runner, for P4)

```bash
# Clone ESP-IDF
cd ~
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# Install for ESP32-P4 (when supported)
./install.sh esp32p4

# Add to shell profile
echo 'alias get_idf=". ~/esp-idf/export.sh"' >> ~/.bashrc

# Source for current session
. ./export.sh
```

## USB Device Configuration (HIL Runner)

### Create udev Rules for Stable Port Names

Create `/etc/udev/rules.d/99-esp32-devices.rules`:

```bash
sudo nano /etc/udev/rules.d/99-esp32-devices.rules
```

Add rules based on device serial numbers:

```udev
# ESP32-S3 Test Device
SUBSYSTEM=="tty", ATTRS{idVendor}=="303a", ATTRS{idProduct}=="1001", \
  ATTRS{serial}=="YOUR_S3_SERIAL", SYMLINK+="ttyUSB_ESP32_S3_TEST", MODE="0666"

# CYD Test Device (CH340 typically)
SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", \
  ATTRS{serial}=="YOUR_CYD_SERIAL", SYMLINK+="ttyUSB_CYD_TEST", MODE="0666"

# ESP32-P4 Test Device (placeholder)
SUBSYSTEM=="tty", ATTRS{idVendor}=="303a", ATTRS{idProduct}=="XXXX", \
  ATTRS{serial}=="YOUR_P4_SERIAL", SYMLINK+="ttyUSB_P4_TEST", MODE="0666"
```

Find device serial numbers:

```bash
# List USB devices with details
udevadm info -a /dev/ttyUSB0 | grep -E "ATTRS{serial}|ATTRS{idVendor}|ATTRS{idProduct}"
```

Reload rules:

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Verify symlinks:

```bash
ls -la /dev/ttyUSB*
```

## Network Configuration (HIL Runner)

Ensure the HIL runner can reach the MQTT broker:

```bash
# Test MQTT connectivity
mosquitto_pub -h homelab.local -p 1883 -t test -m "ping"
mosquitto_sub -h homelab.local -p 1883 -t test -C 1
```

If using firewall, allow outbound MQTT:

```bash
sudo ufw allow out 1883/tcp
```

## Troubleshooting

### Runner Not Picking Up Jobs

1. Check runner is online in GitHub Settings > Actions > Runners
2. Verify labels match workflow requirements
3. Check runner service status: `sudo ./svc.sh status`
4. Check logs: `journalctl -u actions.runner.*`

### Serial Port Access Denied

```bash
# Check user is in dialout group
groups $USER

# If not, add and re-login
sudo usermod -aG dialout $USER
```

### Device Not Found

```bash
# List connected USB devices
lsusb

# Check kernel messages
dmesg | tail -20

# Check udev rules are loaded
udevadm test /dev/ttyUSB0
```

### PlatformIO Build Failures

```bash
# Update PlatformIO
pip3 install -U platformio

# Clear cache
pio system prune

# Check platform is installed
pio platform list
```

## Security Considerations

- Runners have access to repository secrets during workflow execution
- USB access allows flashing arbitrary firmware
- Network access to MQTT broker
- Consider isolating runners on a dedicated VLAN
- Use GitHub's runner group permissions for access control
