# Departure Countdown System

ESP32-based smart departure countdown timer with real-time train departure tracking using NS (Nederlandse Spoorwegen) API.

## Features

- **Real-time Departure Data**: Fetches live train departures from NS API
- **Smart Countdown**: Calculates when you need to leave based on:
  - Real-time train departure times
  - Travel time to station (walk/bike/bus)
  - Configurable buffer time
- **MAX7219 LED Matrix Display**: Shows countdown and train information
- **Multi-modal Transport**: Switch between walk, bike, and bus modes
- **Alert System**: Audio (buzzer) and visual (RGB LED) alerts
- **Button Controls**: Easy mode switching and manual refresh
- **Comprehensive Tests**: Unit tests for all core modules
- **How It Works**: Overview of the code structure and data flow ([HOW_IT_WORKS.md](HOW_IT_WORKS.md))

## Hardware Requirements

### Core Components
- **ESP32-S3-DevKitC-1** Development Board
- MAX7219 LED Matrix Display (4x 8x8 modules recommended)
- Rotary Encoder
- 3x Push Buttons
- Buzzer (PWM capable)
- RGB LED (common cathode)
- DS3231 RTC Module (optional but recommended)

### Wiring (see [WIRING_ESP32-S3.md](WIRING_ESP32-S3.md) for complete guide)

```
ESP32-S3       Component
--------       ---------
GPIO 11        MAX7219 DIN (MOSI)
GPIO 12        MAX7219 CLK (SCK)
GPIO 10        MAX7219 CS
GPIO 4         Encoder CLK
GPIO 5         Encoder DT
GPIO 6         Encoder SW
GPIO 7         Start Button
GPIO 15        Reset Button
GPIO 16        Mode Button
GPIO 17        Buzzer
GPIO 18        LED Red
GPIO 8         LED Green
GPIO 3         LED Blue
GPIO 1         RTC SDA (I2C)
GPIO 2         RTC SCL (I2C)
```

**📋 See [WIRING_ESP32-S3.md](WIRING_ESP32-S3.md) for detailed wiring diagrams and troubleshooting!**

## Software Architecture

### Core Modules

1. **Time Manager** ([time_manager.h](include/time_manager.h))
   - NTP time synchronization
   - RTC integration for offline accuracy
   - ISO 8601 datetime parsing
   - Timezone handling (Europe/Amsterdam)

2. **Configuration Manager** ([config.h](include/config.h))
   - Save/load settings to NVS (Preferences)
   - Station code, API key, travel times
   - Transport mode and buffer time
   - Alert preferences

3. **NS API Client** ([ns_api.h](include/ns_api.h))
   - HTTP client for NS API integration
   - JSON response parsing
   - Departure data extraction
   - Next train selection logic

4. **Countdown Calculator** ([countdown_calc.h](include/countdown_calc.h))
   - Smart "leave home" time calculation
   - Real-time countdown updates
   - State management (safe, ready, urgent, etc.)
   - Edge case handling

5. **Display Manager** ([display.h](include/display.h))
   - MAX7219 LED matrix control
   - Countdown display (MM:SS format)
   - Train information scrolling
   - Transport mode indicators

6. **Alert Manager** ([alerts.h](include/alerts.h))
   - Audio alert patterns (buzzer)
   - Visual alerts (RGB LED colors)
   - State-based triggering
   - Configurable thresholds

7. **Input Handler** ([input.h](include/input.h))
   - Rotary encoder reading
   - Button debouncing
   - Short/long press detection
   - Event queue management

## Getting Started

### 1. Prerequisites

- [PlatformIO](https://platformio.org/) installed
- NS API subscription key (free) from [NS API Portal](https://apiportal.ns.nl/)
- WiFi credentials

### 2. Configuration

1. Clone this repository
2. Open in PlatformIO (VS Code or CLI)
3. Get your NS API key (if you don't already have one):
   - Visit [NS API Portal](https://apiportal.ns.nl/)
   - Create free account
   - Subscribe to "Reisinformatie API"
   - Copy your subscription key

4. Set configuration via Serial Monitor on first boot (stored in NVS):
   - WiFi SSID + password
   - NS API key (optional)
   - To re-enter later, do a factory reset (hold RESET for ~1s) and enter them again

5. Station code and travel times currently use defaults from code:
   - Station code (e.g., "HTNC" for Houten Castellum)
   - Travel times (walk: 20 min, bike: 8 min, bus: 12 min)
   - Buffer time (default: 2 min)
   - To change these at runtime, add your own UI or use the `ConfigManager` API and call `save()`.

### 3. Build and Upload

```bash
# Build project
pio run -e esp32-s3-devkitc-1

# Upload to ESP32-S3
pio run -e esp32-s3-devkitc-1 -t upload

# Open serial monitor
pio device monitor -b 115200
```

### 4. Running Tests

```bash
# Run all tests
pio test

# Run specific test
pio test -f test_config
pio test -f test_countdown_calc
pio test -f test_time_manager
pio test -f test_ns_api
```

## Usage

### Button Controls

- **MODE Button (short press)**: Cycle transport mode (Walk → Bike → Bus → Walk)
- **START Button (short press)**: Force refresh departure data
- **RESET Button (long press, 1s)**: Factory reset (clears all settings)

### Display States

The LED matrix cycles between:
1. **Countdown Display**: `L 08:15 W` (Leave in 8:15, Walk mode)
2. **Train Info**: Scrolling destination, train type, track number
3. **Status Messages**: WiFi status, errors, etc.

### Alert States

**LED Colors:**
- 🟢 Green: Safe time (>10 min to leave)
- 🟡 Yellow: Get ready (5-10 min)
- 🟠 Orange: Time to go (2-5 min)
- 🔴 Red (blinking): Leave now! (<2 min)
- 🔵 Blue: Train delayed
- 🟣 Purple: Error/no departures

**Audio Alerts:**
- **Time to leave**: Single long beep
- **Hurry up** (5 min before departure): Double beep
- **Train departing** (1 min): Rapid beeps

## Configuration

### Default Settings

```cpp
Station Code: HTNC (Houten Castellum)
Walk Time: 20 minutes
Bike Time: 8 minutes
Bus Time: 12 minutes
Active Mode: Walk
Buffer Time: 2 minutes
Audio Alerts: Enabled
LED Alerts: Enabled
```

### Modifying Settings

Settings are stored in NVS (non-volatile storage) and persist across reboots.

## Secrets and Provisioning

WiFi SSID/password and NS API key are captured at first boot via Serial and saved to NVS.
They are not hardcoded in the firmware or stored in the repository.

To re-provision, perform a factory reset (hold RESET for ~1s) and enter new values
when prompted on the Serial Monitor.


To change settings programmatically:

```cpp
configManager.setStationCode("UT");        // Utrecht Centraal
configManager.setTravelTime(WALK, 20);     // 20 min walk
configManager.setTravelTime(BIKE, 10);     // 10 min bike
configManager.setActiveMode(BIKE);          // Switch to bike
configManager.setBufferTime(5);             // 5 min buffer
configManager.save();                       // Save to NVS
```

## API Integration

### NS API Endpoints

The system uses the [NS Reisinformatie API v2](https://apiportal.ns.nl/docs/services/reisinformatie-api/operations/getDepartures):

```
GET https://gateway.apiportal.ns.nl/reisinformatie-api/api/v2/departures
Headers: Ocp-Apim-Subscription-Key: YOUR_KEY
Params: station=HTNC, maxJourneys=5
```

### Station Codes

Common station codes:
- `HTNC` - Houten Castellum
- `HT` - Houten
- `UT` - Utrecht Centraal
- `ASD` - Amsterdam Centraal
- `RTD` - Rotterdam Centraal
- `DH` - Den Haag Centraal

Find more at [NS Stations](https://www.ns.nl/stations)

## Testing

The project includes comprehensive unit tests for all core modules:

### Test Coverage

- **Configuration Manager**: 12 tests
  - Default config, setters, validation, factory reset

- **Time Manager**: 13 tests
  - ISO8601 parsing, time formatting, time differences

- **Countdown Calculator**: 16 tests
  - Leave time calculation, countdown formatting, state determination

- **NS API Client**: 11 tests
  - Initialization, URL generation, departure management

### Running Tests

```bash
# Run all tests
pio test -e native

# Run with verbose output
pio test -e native -v

# Run specific test file
pio test -e native -f test_countdown_calc
```

### Test Results

All tests should pass. Example output:
```
test/test_config.cpp:12:test_default_config                   [PASSED]
test/test_config.cpp:22:test_set_station_code                 [PASSED]
...
-----------------------
12 Tests 0 Failures 0 Ignored
OK
```

## Troubleshooting

### Display Issues

**Problem**: Display not working/blank
- Check wiring (DIN, CLK, CS, VCC, GND)
- Verify 5V power supply (2A recommended)
- Add 1000µF capacitor between VCC and GND
- Check SPI pins match code

**Problem**: Garbage on display
- Check module orientation
- Verify MAX_DEVICES count (4 for 4 modules)
- Try shorter wires (<15cm)
- Lower SPI clock speed

### WiFi Issues

**Problem**: Cannot connect to WiFi
- Verify SSID and password
- Check WiFi signal strength
- ESP32 only supports 2.4GHz (not 5GHz)
- Try power cycling ESP32

### NS API Issues

**Problem**: No departures fetched
- Verify NS API key is correct
- Check station code (4 letters)
- Test API manually with curl
- Check WiFi connection
- Monitor Serial output for HTTP errors

### Time Sync Issues

**Problem**: Time not syncing
- Check WiFi connection (NTP requires internet)
- Verify NTP server accessibility
- RTC can provide backup time
- Check timezone configuration

## Project Structure

```
departure-countdown/
├── include/              # Header files
│   ├── pins.h           # Pin definitions
│   ├── config.h         # Configuration manager
│   ├── time_manager.h   # Time synchronization
│   ├── ns_api.h         # NS API client
│   ├── countdown_calc.h # Countdown calculator
│   ├── display.h        # Display manager
│   ├── alerts.h         # Alert manager
│   ├── input.h          # Input handler
│   └── web_server.h     # Web server (future)
├── src/                 # Source files
│   ├── main.cpp         # Main application
│   ├── config_common.cpp
│   ├── config_esp32.cpp
│   ├── config_native.cpp
│   ├── time_manager_common.cpp
│   ├── time_manager_esp32.cpp
│   ├── time_manager_native.cpp
│   ├── ns_api.cpp
│   ├── ns_api_fetch_esp32.cpp
│   ├── ns_api_fetch_stub.cpp
│   ├── countdown_calc.cpp
│   ├── display_esp32.cpp
│   ├── display_stub.cpp
│   ├── alerts_esp32.cpp
│   ├── alerts_stub.cpp
│   ├── input_esp32.cpp
│   └── input_stub.cpp
├── test/                # Unit tests
│   ├── test_config.cpp
│   ├── test_time_manager.cpp
│   ├── test_countdown_calc.cpp
│   └── test_ns_api.cpp
├── platformio.ini       # PlatformIO configuration
├── README.md            # This file
└── IMPLEMENTATION_PLAN.md  # Detailed implementation plan
```

## Future Enhancements

See [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) Phase 6 for planned features:

- Web UI for configuration
- RESTful API with OpenAPI spec
- Multiple transport provider support (Google Maps)
- Multi-station support
- MQTT/Home Assistant integration
- Mobile app
- Low power modes

## License

This project is provided as-is for educational and personal use.

## Credits

- NS API: [Nederlandse Spoorwegen](https://www.ns.nl/)
- Libraries: See [platformio.ini](platformio.ini) for dependencies

## Support

For issues, questions, or contributions:
1. Check [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) for detailed documentation
2. Review troubleshooting section above
3. Check Serial Monitor output for debug information
4. Verify hardware connections match [pins.h](include/pins.h)
