# Quick Start Guide - ESP32-S3 Departure Countdown

## 🚀 Fast Track to First Boot

### 1. Enter WiFi + API Secrets (first boot)

Open the Serial Monitor right after flashing (115200 baud) and follow the prompts:

```
WiFi SSID:
WiFi Password (blank for open):
NS API key:
```

✅ Make sure these match your 2.4GHz WiFi network exactly (case-sensitive!)

Credentials are stored in NVS. To change them later, do a factory reset (hold RESET for ~1s) and re-enter them.

### 2. Build & Upload (2 minutes)

```bash
cd /path/to/departure-countdown

# Clean previous build
pio run -e esp32-s3-devkitc-1 -t clean

# Build new firmware
pio run -e esp32-s3-devkitc-1

# Upload to ESP32-S3
pio run -e esp32-s3-devkitc-1 -t upload

# Watch the boot sequence
pio device monitor -b 115200
```

### 3. What You Should See

```
=== Departure Countdown System ===
Initializing...

Loading configuration... OK
Initializing display... OK
Connecting to WiFi... .......OK
IP Address: 192.168.1.100
Signal Strength: -52 dBm
Initializing time...
Forcing NTP sync...
NTP sync successful
Warning: RTC not found, continuing without RTC  ← This is OK!
OK
Initializing NS API... OK

=== System Ready ===
```

---

## ✅ Success Indicators

- ✅ WiFi connects within 20 seconds
- ✅ Gets IP address from router
- ✅ Signal strength shown (should be > -70 dBm)
- ✅ NTP sync successful
- ✅ "System Ready" message appears
- ✅ No crashes or reboots

---

## ⚠️ If Something Goes Wrong

### WiFi Won't Connect?

1. **Double-check credentials** (most common issue!)
2. **Try a phone hotspot** to test
3. **Check signal strength** - move ESP32-S3 closer to router
4. **Router compatibility** - ensure 2.4GHz is enabled, not 5GHz only

See [WIFI_TROUBLESHOOTING.md](WIFI_TROUBLESHOOTING.md) for detailed help.

### Still See RTC Errors?

Don't worry! "Warning: RTC not found" is normal and OK. The system uses NTP time instead.

### System Crashes?

If you still see crashes after rebuild, share the **new** serial monitor output. The old errors were from the old firmware.

---

## 📝 Next Steps After First Boot

1. **Get NS API Key**
   - Visit https://apiportal.ns.nl/
   - Sign up (free account)
   - Subscribe to "Reisinformatie API"
   - Copy your subscription key

2. **Configure via Serial**
   - Open serial monitor
   - Enter WiFi SSID/password and (optional) NS API key via Serial
   - Station code and travel times currently use defaults from code

3. **Connect Hardware** (optional, for full system)
   - MAX7219 display modules
   - Rotary encoder
   - Buttons
   - See [WIRING_ESP32-S3.md](WIRING_ESP32-S3.md)

---

## 🔧 Hardware Requirements

### Minimum (to test WiFi & API):
- ESP32-S3-DevKitC-1 board
- USB-C cable
- Computer with PlatformIO

### Full System:
- ESP32-S3-DevKitC-1
- 4x MAX7219 8x8 LED matrix modules
- Rotary encoder with push button
- 3x push buttons
- DS3231 RTC module (optional)
- Buzzer (optional)
- Wires and breadboard

---

## 💡 Quick Tips

- **Press Ctrl+C** to exit serial monitor
- **Press EN button** on ESP32-S3 to restart
- **Monitor baud rate** is 115200 (auto-detected by PlatformIO)
- **USB port** - ESP32-S3 uses native USB, no CH340 needed
- **Power** - USB provides enough power for ESP32-S3 + display

---

## 📖 Full Documentation

- [CURRENT_STATUS.md](CURRENT_STATUS.md) - Detailed status and next steps
- [WIFI_TROUBLESHOOTING.md](WIFI_TROUBLESHOOTING.md) - WiFi debugging guide
- [WIRING_ESP32-S3.md](WIRING_ESP32-S3.md) - Hardware wiring guide
- [README.md](README.md) - Complete project documentation

---

**Questions?** Check the documentation above or share your serial monitor output for help.

Last Updated: 2026-01-02
