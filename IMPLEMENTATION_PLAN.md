# Departure Countdown Project - Implementation Plan

## Project Overview
ESP32-based **smart** departure countdown timer system with real-time train departure tracking using NS (Nederlandse Spoorwegen) API or other transport providers.

**Core Concept:** Calculate when you need to leave home based on:
- Real-time train departure times from NS API
- Distance/travel time from home to station
- Transportation method (walk, bike, bus)
- Buffer time for safety margin

**Target Board:** ESP32 Dev Module
**Platform:** Espressif32
**Framework:** Arduino

---

## Key Features

### Smart Countdown System
1. **Real-time Departure Data**
   - Fetch live train departures from NS API
   - Support for multiple transport providers (NS, Google Maps, etc.)
   - Display next available departure
   - Handle delays and cancellations

2. **Intelligent Calculation**
   - Home → Station travel time based on method:
     - Walk: User-defined minutes
     - Bike: User-defined minutes
     - Bus: User-defined minutes + buffer
   - Configurable buffer time (safety margin)
   - Auto-update countdown based on real departures

3. **Web UI Configuration**
   - Set station code (e.g., HTNC for Houten Castellum)
   - Configure travel times for each method
   - Select active transportation method
   - Set NS API key
   - View upcoming departures

4. **OpenAPI Integration**
   - RESTful API for mobile apps
   - JSON endpoints for configuration
   - Status monitoring
   - Remote control

---

## Hardware Components Required

### Core Components
- **ESP32 Development Board** (ESP32 Dev Module)
- **Display Options:**
  - **Option A (Recommended): MAX7219 LED Matrix 4x (8x8) - SPI**
    - 4 modules daisy-chained
    - 32x8 pixel resolution
    - Bright, visible in daylight
    - Low power consumption per module
    - Easy scrolling text and animations
    - Ideal for countdown display
  - **Option B: MAX7219 with Custom 7-Segment Display - SPI**
    - Up to 8 digits per module
    - Can chain multiple modules for HH:MM:SS format
    - Clear numeric display
    - Retro clock aesthetic
    - Very readable from distance
  - Option C: 0.96" OLED Display (SSD1306) - I2C
  - Option D: 1.3" OLED Display (SH1106) - I2C
  - Option E: TFT LCD Display (ST7735/ILI9341) - SPI
  - Option F: E-Paper Display (for low power consumption)

### Input Components
- **Rotary Encoder** - For time adjustment
- **Push Buttons** - Start/Stop/Reset controls
- **Optional:** Keypad for direct time input

### Audio/Visual Alerts
- **Buzzer/Piezo Speaker** - Countdown alerts
- **RGB LED** - Visual status indicator
- **Optional:** WS2812B LED Strip for ambient effects

### Power & Misc
- **Power Supply:** USB-C or 5V adapter
- **RTC Module (DS3231):** Required for accurate timekeeping
- **WiFi connectivity:** Required for NS API integration

---

## Software Architecture

### 1. Core Modules

#### 1.1 Display Manager (`display.h/cpp`)
- Initialize display hardware (MAX7219/OLED)
- Render countdown time (MM:SS format)
- Show next train info (destination, track, delay)
- Display travel method icon
- Scrolling messages on LED matrix
- Handle screen updates efficiently

#### 1.2 NS API Client (`ns_api.h/cpp`)
- **HTTP client for NS API integration**
- Fetch departure data from `https://gateway.apiportal.ns.nl`
- Parse JSON responses
- Handle API authentication (Ocp-Apim-Subscription-Key)
- Extract departure info:
  - Planned/actual departure time
  - Destination
  - Track number
  - Delays/cancellations
  - Train type (Sprinter/Intercity)
- Auto-refresh every 30-60 seconds
- Error handling (network failures, invalid responses)

#### 1.3 Transport Provider Interface (`transport_provider.h/cpp`)
- **Abstract interface for multiple providers**
- NS API provider (primary)
- Google Maps API provider (future)
- Provider switching logic
- Fallback mechanisms

#### 1.4 Smart Countdown Calculator (`countdown_calc.h/cpp`)
- Calculate "leave home" time:
  - `leaveTime = departureTime - travelTime - bufferTime`
- Support multiple transport modes:
  - Walk (e.g., 15 min)
  - Bike (e.g., 8 min)
  - Bus (e.g., 12 min + bus schedule)
- Real-time countdown updates
- Next departure selection logic
- Handle edge cases (train in <5 min, already departed)

#### 1.5 Time Manager (`time_manager.h/cpp`)
- NTP time synchronization
- RTC integration for offline accuracy
- Timezone handling (Europe/Amsterdam)
- Time formatting utilities
- 1-second tick for display updates

#### 1.6 Input Handler (`input.h/cpp`)
- Rotary encoder reading with debounce
- Button state detection (short/long press)
- Input event queue management
- Quick transport mode switching
- Manual refresh trigger

#### 1.7 Alert Manager (`alerts.h/cpp`)
- Audio alerts (buzzer patterns)
- Visual alerts (LED blink patterns)
- Smart warnings:
  - **"Time to leave!"** (at calculated leave time)
  - **"Hurry up!"** (5 min before departure)
  - **"Train departing!"** (1 min before departure)
  - **"Train delayed"** (on delay notification)
- Configurable alert thresholds

#### 1.8 Configuration Manager (`config.h/cpp`)
- Save/load settings to SPIFFS/Preferences
- Store configuration:
  - NS API key
  - Station code (e.g., "HTNC")
  - Travel times (walk: 15, bike: 8, bus: 12)
  - Active transport method
  - Buffer time (default: 2 min)
  - Alert preferences
- Factory reset functionality

#### 1.9 WiFi & Web Server (`web_server.h/cpp`)
- WiFi connection management
- AsyncWebServer for web UI
- RESTful API endpoints (OpenAPI spec)
- WebSocket for live updates
- Captive portal for initial setup

#### 1.10 API Router (`api_routes.h/cpp`)
- **OpenAPI/REST endpoints:**
  - `GET /api/status` - Current countdown status
  - `GET /api/departures` - Next departures from station
  - `GET /api/config` - Get configuration
  - `POST /api/config` - Update configuration
  - `POST /api/transport-mode` - Switch transport method
  - `GET /api/health` - System health check

---

## MAX7219 Display Implementation Guide

### Hardware Setup

#### Option 1: MAX7219 LED Matrix (4x 8x8 modules)

**Specifications:**
- 4 modules daisy-chained (32x8 pixel resolution)
- SPI communication
- 5V operation (can work with 3.3V logic from ESP32)
- Each module: ~10-15mA per LED, max ~240mA per module at full brightness

**Wiring Diagram:**
```
ESP32          MAX7219 Module 1   MAX7219 Module 2-4
-----          ----------------   ------------------
GPIO 23 (MOSI) → DIN
GPIO 18 (SCK)  → CLK
GPIO 5  (CS)   → CS/LOAD
5V             → VCC
GND            → GND

Module 1 → Module 2 → Module 3 → Module 4
  DOUT → DIN    DOUT → DIN    DOUT → DIN
```

**Library Choice:**
1. **MD_MAX72XX + MD_Parola** (Recommended for animations)
   - Pros: Rich text effects, scrolling, sprites
   - Cons: Larger library size
   - Best for: Matrix displays with text/graphics

2. **LedControl** (Simple, lightweight)
   - Pros: Simple API, small footprint
   - Cons: Manual text rendering needed
   - Best for: 7-segment or custom patterns

**Code Example (MD_Parola):**
```cpp
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   18
#define DATA_PIN  23
#define CS_PIN    5

MD_Parola display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void setup() {
  display.begin();
  display.setIntensity(5);  // Brightness 0-15
  display.displayClear();
  display.print("00:00:00");
}

void displayCountdown(int hours, int mins, int secs) {
  char buffer[9];
  sprintf(buffer, "%02d:%02d:%02d", hours, mins, secs);
  display.print(buffer);
}
```

#### Option 2: MAX7219 with 7-Segment Display

**Specifications:**
- 8 digits per MAX7219 module
- Can chain 2 modules for 16 digits (HH:MM:SS + extras)
- Ultra-bright, large digits
- Common cathode 7-segment displays

**Segment Mapping:**
```
     A
    ___
F  |   | B
   |___|
   G
E  |   | C
   |___|
     D    DP
```

**Code Example (LedControl):**
```cpp
#include <LedControl.h>

#define DIN_PIN  23
#define CLK_PIN  18
#define CS_PIN   5
#define NUM_DEVICES 1  // One MAX7219 for 8 digits

LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, NUM_DEVICES);

void setup() {
  lc.shutdown(0, false);  // Wake up
  lc.setIntensity(0, 8);  // Brightness 0-15
  lc.clearDisplay(0);
}

void displayTime(int h, int m, int s) {
  lc.setDigit(0, 7, h / 10, false);    // Hours tens
  lc.setDigit(0, 6, h % 10, true);     // Hours ones (with DP as colon)
  lc.setDigit(0, 5, m / 10, false);    // Minutes tens
  lc.setDigit(0, 4, m % 10, true);     // Minutes ones (with DP)
  lc.setDigit(0, 3, s / 10, false);    // Seconds tens
  lc.setDigit(0, 2, s % 10, false);    // Seconds ones
  lc.setChar(0, 1, ' ', false);        // Blank
  lc.setChar(0, 0, ' ', false);        // Blank
}
```

### Display Features

#### For LED Matrix:
1. **Scrolling Messages**
   - "DEPARTURE COUNTDOWN"
   - "TIME'S UP!"
   - Menu options

2. **Animations**
   - Boot animation (logo/startup)
   - Blinking colon separator
   - Progress bar
   - Countdown pulse effect
   - Alert flash patterns

3. **Text Effects** (with MD_Parola)
   - Scroll left/right
   - Fade in/out
   - Dissolve
   - Blinds effect

#### For 7-Segment:
1. **Large, Clear Digits**
   - High visibility from distance
   - Classic clock appearance

2. **Decimal Point Control**
   - Use as colon separator
   - Blink for visual effect

3. **Brightness Control**
   - Auto-dim based on time of day
   - Manual adjustment via menu

### Power Considerations

**Current Draw:**
- Each LED: ~10-15mA when ON
- 4 modules at full brightness, all LEDs ON: ~1A
- Typical countdown display: ~200-400mA
- 7-segment: ~150-300mA depending on digits lit

**Power Supply:**
- USB 5V 2A recommended
- Add 1000µF capacitor near VCC/GND for stability
- Reduce brightness to lower current draw

**Level Shifting (if needed):**
- ESP32 outputs 3.3V logic
- MAX7219 accepts 3.3V input (typically works fine)
- If issues, use level shifter or pull-up resistors

---

## NS API Integration Details

### API Endpoint
```
GET https://gateway.apiportal.ns.nl/reisinformatie-api/api/v2/departures
```

### Authentication
```cpp
Headers:
  Ocp-Apim-Subscription-Key: YOUR_API_KEY
```

### Request Parameters
```cpp
station=HTNC  // Station code (e.g., HTNC = Houten Castellum)
maxJourneys=5 // Optional: limit number of results
```

### Response Structure
```json
{
  "payload": {
    "departures": [
      {
        "direction": "Den Haag Centraal",
        "name": "NS 6084",
        "plannedDateTime": "2025-12-24T23:28:00+0100",
        "actualDateTime": "2025-12-24T23:28:00+0100",
        "plannedTrack": "1",
        "actualTrack": "1",
        "product": {
          "categoryCode": "SPR",
          "shortCategoryName": "NS Sprinter"
        },
        "cancelled": false,
        "departureStatus": "INCOMING"
      }
    ]
  }
}
```

### Code Example
```cpp
#include <HTTPClient.h>
#include <ArduinoJson.h>

void fetchDepartures() {
  HTTPClient http;
  http.begin("https://gateway.apiportal.ns.nl/reisinformatie-api/api/v2/departures?station=HTNC");
  http.addHeader("Ocp-Apim-Subscription-Key", API_KEY);

  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();

    StaticJsonDocument<8192> doc;
    deserializeJson(doc, payload);

    JsonArray departures = doc["payload"]["departures"];

    for (JsonObject dep : departures) {
      String direction = dep["direction"];
      String plannedTime = dep["plannedDateTime"];
      String actualTime = dep["actualDateTime"];
      int track = dep["actualTrack"];
      bool cancelled = dep["cancelled"];

      // Calculate countdown
      // leaveTime = parseTime(actualTime) - travelTime - bufferTime
    }
  }

  http.end();
}
```

### Station Codes (Examples)
- `HTNC` - Houten Castellum
- `HT` - Houten
- `UT` - Utrecht Centraal
- `ASD` - Amsterdam Centraal
- `RTD` - Rotterdam Centraal

### Alternative Providers

#### Google Maps Distance Matrix API
```
GET https://maps.googleapis.com/maps/api/distancematrix/json
  ?origins=HOME_ADDRESS
  &destinations=STATION_ADDRESS
  &mode=walking|bicycling|transit
  &key=YOUR_API_KEY
```

**Benefits:**
- Real-time traffic data
- Multiple transport modes
- Accurate walking/cycling times
- Transit schedules for buses

**Integration:**
- Use for calculating travel time dynamically
- Fallback if NS API fails
- Combined use: NS for train times, Google for travel times

---

## Implementation Phases

### Phase 1: Basic Hardware Setup
**Duration:** Foundation setup

#### Tasks:
1. Configure PlatformIO environment
   - Add required libraries to `platformio.ini`
   - Set up build flags and optimization

2. Test basic ESP32 functionality
   - Serial communication
   - GPIO configuration
   - Power management

3. Display initialization
   - Choose display type
   - Install appropriate library
   - Test display output
   - Create basic UI layout

4. Input hardware setup
   - Wire rotary encoder
   - Wire buttons with pull-up resistors
   - Test input reading
   - Implement debouncing

**Deliverables:**
- Working display showing test pattern
- Responsive button/encoder input
- Serial debug output functional

---

### Phase 2: WiFi & Time Synchronization
**Duration:** Network connectivity

#### Tasks:
1. WiFi Connection
   - WiFi manager with captive portal
   - Store WiFi credentials in SPIFFS
   - Auto-reconnect on disconnection
   - WiFi status indicator on display

2. NTP Time Sync
   - Sync with NTP servers (pool.ntp.org)
   - Europe/Amsterdam timezone configuration
   - RTC module integration for offline accuracy
   - Periodic re-sync (every hour)

3. Basic Web Server
   - AsyncWebServer setup
   - Serve simple status page
   - REST API endpoint structure
   - CORS configuration

**Deliverables:**
- ESP32 connected to WiFi
- Accurate time from NTP + RTC
- Basic web server responding
- WiFi status on display

---

### Phase 3: NS API Integration
**Duration:** Smart departure tracking

#### Tasks:
1. NS API Client
   - HTTP client implementation
   - API authentication with subscription key
   - Fetch departure data for station
   - Parse JSON response (ArduinoJson)
   - Extract next 3-5 departures

2. Smart Countdown Calculator
   - Parse departure times (ISO 8601 format)
   - Calculate leave-home time:
     ```
     leaveTime = departureTime - travelTime - bufferTime
     countdownSeconds = leaveTime - currentTime
     ```
   - Handle multiple departures (skip if train already departed)
   - Auto-select next suitable train

3. Display Integration
   - Show countdown timer (MM:SS)
   - Display train info:
     - Destination (scrolling on LED matrix)
     - Track number
     - Departure time
   - Transport mode indicator (walk/bike/bus icon)
   - Delay warnings (if actualTime ≠ plannedTime)

**Deliverables:**
- Live train departures fetched from NS API
- Smart countdown calculation working
- Display shows "Leave in: 08:30" format
- Next train info visible

---

### Phase 4: Alert System
**Duration:** Smart notifications

#### Tasks:
1. Audio Alert Implementation
   - Connect buzzer to PWM-capable GPIO
   - Smart alert patterns:
     - **"Time to leave!"** - Single long beep
     - **"Hurry up!"** - Double beep (5 min before departure)
     - **"Train departing!"** - Rapid beeps (1 min before)
     - **"Train delayed"** - Descending tone
     - **"Train cancelled"** - Error tone

2. Visual Alert Implementation
   - RGB LED status indicator:
     - **Green:** Safe time (>10 min to leave)
     - **Yellow:** Get ready (5-10 min to leave)
     - **Orange:** Time to go (2-5 min to leave)
     - **Red blinking:** Leave now! (<2 min)
     - **Blue:** Train delayed
     - **Purple:** No departures found

3. LED Matrix Alerts
   - Scrolling urgent messages
   - Flashing display on critical alerts
   - Visual countdown progress bar

**Deliverables:**
- Context-aware audio alerts
- Visual LED feedback system
- Urgent message display

---

### Phase 5: Web UI & Configuration
**Duration:** User interface

#### Tasks:
1. Web UI Development
   - **Setup Page:**
     - WiFi configuration
     - NS API key input
     - Station code selection (autocomplete?)
   - **Configuration Page:**
     - Travel time settings (walk/bike/bus in minutes)
     - Buffer time slider (0-10 min)
     - Transport method selector
     - Alert preferences
   - **Dashboard:**
     - Live countdown display
     - Next 5 departures list
     - Current settings overview
     - Quick transport mode switch

2. REST API Implementation
   - `GET /api/status` - Current countdown & next train
   - `GET /api/departures` - List upcoming departures
   - `GET /api/config` - Get all settings
   - `POST /api/config` - Update settings
   - `POST /api/transport-mode` - Switch mode (walk/bike/bus)
   - `POST /api/refresh` - Force API refresh
   - `GET /api/health` - System health

3. OpenAPI Specification
   - Document all endpoints
   - JSON schema for requests/responses
   - Interactive API docs (Swagger UI)

4. Data Persistence
   - Save configuration to SPIFFS:
     - NS API key
     - Station code
     - Travel times (walk, bike, bus)
     - Active transport method
     - Buffer time
     - Alert settings
   - Auto-load on boot
   - Factory reset endpoint

**Deliverables:**
- Fully functional web UI
- REST API with OpenAPI docs
- Configuration persistence
- Mobile-responsive design

---

### Phase 6: Advanced Features (Optional)
**Duration:** Enhancement phase

#### Tasks:
1. Multiple Transport Providers
   - Add Google Maps Distance Matrix API
   - Calculate real-time travel times
   - Traffic-aware bike/walk times
   - Bus schedule integration

2. Multi-Station Support
   - Store multiple stations
   - Switch between stations via UI
   - Preset routes (home→work, home→city, etc.)

3. Smart Scheduling
   - Recurring departure preferences
   - Weekday vs weekend settings
   - Time-based auto-activation
   - Calendar API integration (Google Calendar)

4. Home Automation Integration
   - MQTT support for Home Assistant
   - Publish countdown status
   - Control via voice assistants
   - Trigger smart home scenes ("leaving home")

5. Mobile App
   - Flutter/React Native companion app
   - Push notifications
   - Remote configuration
   - Trip history tracking

6. Low Power Optimizations
   - Deep sleep between API calls
   - Wake on schedule or button press
   - Battery level monitoring (if portable)
   - E-paper display option for ultra-low power

**Deliverables:**
- Multi-provider transport data
- Smart scheduling system
- Home automation integration
- Mobile app (optional)
- Power-efficient modes

---

## Library Dependencies

### Required Libraries (Phase 1-5)
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

; Partition scheme for larger apps with web UI
board_build.partitions = huge_app.csv

lib_deps =
    ; Display Libraries (choose based on hardware)

    ; MAX7219 LED Matrix (RECOMMENDED for 8x8 matrix or 7-segment)
    majicdesigns/MD_MAX72XX@^3.5.1             ; MAX72XX LED matrix control
    majicdesigns/MD_Parola@^3.7.3              ; Text effects and animations

    ; Alternative: LedControl library for MAX7219
    wayoda/LedControl@^1.0.6                   ; Simple MAX7219 control

    ; OLED Displays (if using instead of MAX7219)
    ; adafruit/Adafruit SSD1306@^2.5.7          ; For OLED
    ; adafruit/Adafruit GFX Library@^1.11.3     ; Graphics library

    ; TFT Displays (if using instead of MAX7219)
    ; bodmer/TFT_eSPI@^2.5.0                     ; For TFT displays

    ; Input Libraries
    mathertel/RotaryEncoder@^1.5.3             ; Rotary encoder

    ; WiFi & Web Server (REQUIRED for NS API)
    esphome/ESPAsyncWebServer-esphome@^3.0.0   ; Async web server
    esphome/AsyncTCP-esphome@^2.0.1            ; Async TCP library

    ; HTTP Client & JSON (REQUIRED for NS API)
    bblanchon/ArduinoJson@^7.0.0               ; JSON parsing (NS API responses)

    ; Time Management (REQUIRED)
    arduino-libraries/NTPClient@^3.2.1         ; NTP time sync

    ; RTC Support
    adafruit/RTClib@^2.1.1                     ; DS3231 RTC module

    ; File System
    ; SPIFFS/LittleFS built-in to ESP32 core

    ; Preferences (built-in to ESP32 core)
    ; No external library needed
```

### Optional Libraries (Phase 6)
```ini
lib_deps =
    ; MQTT for Home Automation
    knolleary/PubSubClient@^2.8                ; MQTT client

    ; Additional timezone support
    ropg/ezTime@^0.8.3                         ; Advanced timezone handling

    ; OTA Updates
    ; Built-in to ESP32 core via ArduinoOTA
```

### Build Flags
```ini
build_flags =
    -D CORE_DEBUG_LEVEL=3                      ; Enable debug logging
    -D CONFIG_ARDUHAL_LOG_COLORS               ; Colored logs
    -D BOARD_HAS_PSRAM                         ; If using PSRAM variant
```

---

## Pin Configuration (Example)

### Configuration A: MAX7219 LED Matrix/7-Segment (Recommended)
```cpp
// MAX7219 Display (SPI)
#define MAX7219_DIN    23   // MOSI (Data In)
#define MAX7219_CLK    18   // SCK (Clock)
#define MAX7219_CS     5    // Chip Select (Load)
#define MAX_DEVICES    4    // Number of 8x8 matrices chained

// Rotary Encoder
#define ENC_CLK     25  // Clock pin
#define ENC_DT      26  // Data pin
#define ENC_SW      27  // Switch/button pin

// Buttons
#define BTN_START   32  // Start/Pause button
#define BTN_RESET   33  // Reset button
#define BTN_MODE    34  // Mode/Menu button

// Alerts
#define BUZZER_PIN  19  // PWM buzzer
#define LED_R       4   // RGB LED Red
#define LED_G       16  // RGB LED Green
#define LED_B       17  // RGB LED Blue

// Optional RTC (I2C - different pins than SPI display)
#define RTC_SDA     21  // I2C Data
#define RTC_SCL     22  // I2C Clock
```

### Configuration B: OLED Display (Alternative)
```cpp
// Display (I2C)
#define OLED_SDA    21
#define OLED_SCL    22
#define OLED_RST    -1  // Not used for most I2C displays

// (Same pin configuration for other components as above)
```

---

## User Interface Flow

### MAX7219 LED Matrix Display (4x 8x8 modules = 32x8 pixels)

#### Smart Countdown Display - Main Screen
```
┌────────────────────────────────┐
│ LEAVE IN: 08:15  🚶            │  Countdown + transport icon
└────────────────────────────────┘

Alternates every 5 seconds with:

┌────────────────────────────────┐
│ → DEN HAAG  SPR 6084           │  Destination + train info
│   DEP 18:45  TRACK 1           │  Departure time + track
└────────────────────────────────┘
```

#### Transport Mode Indicators (Icons)
```
🚶 Walk    (8 min configured)
🚴 Bike    (5 min configured)
🚌 Bus     (12 min configured)
```

#### Status Messages (Scrolling)
```
┌────────────────────────────────┐
│ FETCHING TRAINS...             │  Loading state
└────────────────────────────────┘

┌────────────────────────────────┐
│ TRAIN DELAYED +3 MIN           │  Delay notification
└────────────────────────────────┘

┌────────────────────────────────┐
│ TIME TO LEAVE NOW!             │  Alert message
└────────────────────────────────┘

┌────────────────────────────────┐
│ NO WIFI - CHECK CONNECTION     │  Error state
└────────────────────────────────┘
```

#### LED Matrix Layout Examples
```
Display Cycle (rotates every 5 sec):

1. Countdown:     "LEAVE 08:15 🚶"
2. Train Info:    "→ UTRECHT SPR"
3. Track/Time:    "T1 DEP 18:45"
4. Next Train:    "NEXT: 19:00"
```

### MAX7219 7-Segment Display (8 digits)

#### Smart Countdown - 7-Segment
```
┌─────────────────────────────────┐
│  L  0  8 •  1  5     [W]        │  "L 08:15" + Walk icon
└─────────────────────────────────┘

Where:
- "L" = Leave in
- "08:15" = Minutes:Seconds
- [W] = Walk mode (or B=Bike, U=Bus)
```

#### Alternative Layout (Station Display)
```
┌─────────────────────────────────┐
│  1  8 •  4  5  [  1  ]          │  "18:45" + Track 1
└─────────────────────────────────┘
```

### Web UI Dashboard

#### Main Dashboard View
```html
┌─────────────────────────────────────────┐
│  DEPARTURE COUNTDOWN                    │
│  ═══════════════════════════════════    │
│                                         │
│  🕐 Leave in: 08:15                     │
│  Transport: 🚶 Walk (8 min)             │
│  Buffer: 2 min                          │
│                                         │
│  Next Train:                            │
│  → Den Haag Centraal                    │
│  Departure: 18:45 (Track 1)             │
│  Train: NS Sprinter 6084                │
│  Status: On Time ✓                      │
│                                         │
│  [🚶 Walk] [🚴 Bike] [🚌 Bus]         │
│  [↻ Refresh]                            │
│                                         │
│  Upcoming Departures:                   │
│  ────────────────────                   │
│  18:45 → Den Haag    Track 1            │
│  19:00 → Utrecht     Track 2            │
│  19:15 → Amsterdam   Track 1            │
│                                         │
└─────────────────────────────────────────┘
```

#### Configuration Page
```html
┌─────────────────────────────────────────┐
│  CONFIGURATION                          │
│  ═══════════════════                    │
│                                         │
│  Station Settings                       │
│  ────────────────                       │
│  Station Code: [HTNC           ]        │
│  NS API Key:   [************** ]        │
│                                         │
│  Travel Times                           │
│  ────────────────                       │
│  🚶 Walk:  [8  ] minutes               │
│  🚴 Bike:  [5  ] minutes               │
│  🚌 Bus:   [12 ] minutes               │
│                                         │
│  Buffer Time:  [2  ] minutes           │
│  (Safety margin before departure)       │
│                                         │
│  Alert Settings                         │
│  ────────────────                       │
│  ☑ Audio alerts enabled                │
│  ☑ LED notifications                   │
│  Alert at leave time: ☑                │
│  Alert 5 min before: ☑                 │
│                                         │
│  [Save Configuration]                   │
│                                         │
└─────────────────────────────────────────┘
```

### Display Modes

1. **Smart Countdown Mode**: Shows leave time with transport mode
2. **Train Info Mode**: Next departure details
3. **Alert Mode**: Urgent messages (leave now, delay, etc.)
4. **Error Mode**: WiFi/API issues
5. **Setup Mode**: Initial configuration wizard

---

## State Machine Design

```
States:
- IDLE          : Waiting for user input
- SETTING_TIME  : User adjusting departure time
- COUNTDOWN     : Timer actively counting down
- PAUSED        : Timer paused mid-countdown
- ALERT_ACTIVE  : Time's up, alerting user
- MENU          : In configuration menu

Transitions:
IDLE → SETTING_TIME    : Press MODE button
SETTING_TIME → COUNTDOWN : Press START button
COUNTDOWN → PAUSED     : Press START button
PAUSED → COUNTDOWN     : Press START button
* → IDLE               : Press RESET button
COUNTDOWN → ALERT_ACTIVE : Time reaches 00:00:00
ALERT_ACTIVE → IDLE    : Press any button
* → MENU               : Long press MODE button
MENU → previous state  : Press MODE or RESET
```

---

## Testing Plan

### Unit Tests
1. Time calculation accuracy
2. Input debouncing reliability
3. Alert trigger timing
4. Storage read/write integrity

### Integration Tests
1. Display update responsiveness
2. Multi-alert coordination
3. Menu navigation flow
4. Power cycle persistence

### User Acceptance Tests
1. Set various countdown times
2. Verify alerts at correct intervals
3. Test all button combinations
4. Confirm preset functionality
5. Validate WiFi features (if implemented)

---

## Power Consumption Estimates

### MAX7219 LED Matrix/7-Segment
- **Idle (all LEDs off):** ~10mA @ 5V
- **Typical countdown display:** 200-400mA @ 5V (0.5-2W)
- **Full brightness, all LEDs:** Up to 1A @ 5V (5W for 4 modules)
- **Brightness level 5 (recommended):** ~150-250mA @ 5V

### OLED Display (Alternative)
- **Active Mode (Display On, WiFi Off):** ~80-120mA @ 5V (0.4-0.6W)
- **Active Mode (Display On, WiFi On):** ~150-200mA @ 5V (0.75-1.0W)

### Sleep Mode (Optional)
- ~10-50mA @ 5V (depends on display type)
- ~0.05-0.25W

### Battery Life Estimates

**With MAX7219 Display (brightness 5):**
- 2000mAh battery: ~6-10 hours
- 5000mAh battery: ~15-25 hours
- 10000mAh power bank: ~30-50 hours

**With OLED Display:**
- 2000mAh battery: ~16-25 hours (WiFi off)
- 2000mAh battery: ~10-13 hours (WiFi on)

---

## Enclosure Design Considerations

### Dimensions
- Display viewing angle
- Button accessibility
- Ventilation for ESP32
- USB port access

### Materials
- 3D printed case (PLA/PETG)
- Acrylic front panel
- Mounting options (wall/desk)

### Features
- Diffused LED light guide
- Speaker grille for buzzer
- Cable management

---

## Future Enhancements

1. **Mobile App Integration**
   - React Native or Flutter app
   - Push notifications
   - Remote control via Bluetooth/WiFi

2. **Multiple Countdown Support**
   - Track several departures simultaneously
   - Priority system for alerts

3. **Location-Based Features**
   - GPS integration
   - Travel time calculation (via Google Maps API)
   - Auto-adjust based on traffic

4. **Voice Integration**
   - Alexa/Google Home integration
   - Voice announcements via DFPlayer/MP3 module

5. **Data Logging**
   - Track departure history
   - Statistics dashboard
   - Export to CSV/JSON

---

## Development Checklist

- [ ] Phase 1: Hardware Setup
  - [ ] PlatformIO configuration
  - [ ] Display working
  - [ ] Inputs responsive

- [ ] Phase 2: Core Countdown
  - [ ] Timer logic implemented
  - [ ] UI displaying countdown
  - [ ] Time setting interface

- [ ] Phase 3: Alerts
  - [ ] Audio alerts working
  - [ ] Visual LED indicators
  - [ ] Multi-stage warnings

- [ ] Phase 4: Persistence
  - [ ] Settings saved to EEPROM
  - [ ] Preset management
  - [ ] Configuration menu

- [ ] Phase 5: Advanced Features
  - [ ] WiFi connectivity
  - [ ] Web interface
  - [ ] NTP synchronization
  - [ ] Smart features

---

## Initial Setup Guide

### First-Time Configuration

1. **Get NS API Key**
   - Visit [NS API Portal](https://apiportal.ns.nl/)
   - Create free account
   - Subscribe to "Reisinformatie API"
   - Copy your subscription key (Ocp-Apim-Subscription-Key)

2. **Find Your Station Code**
   - Search on [NS Stations](https://www.ns.nl/stations)
   - Or use common codes:
     - Houten Castellum: `HTNC`
     - Utrecht Centraal: `UT`
     - Amsterdam Centraal: `ASD`

3. **Hardware Assembly**
   - Connect MAX7219 display to ESP32 (SPI pins)
   - Wire buttons and rotary encoder
   - Add buzzer for alerts
   - Connect RTC module (I2C)
   - Power via USB (2A recommended)

4. **First Boot - WiFi Setup**
   - Device creates WiFi AP: "DepartureTimer-XXXX"
   - Connect to AP with phone/computer
   - Captive portal opens automatically
   - Enter your WiFi credentials
   - Device reboots and connects

5. **Web Configuration**
   - Find ESP32 IP address (check router or serial monitor)
   - Open browser: `http://ESP32_IP`
   - Configuration wizard guides you:
     - Enter NS API key
     - Select station code
     - Set travel times (walk/bike/bus)
     - Choose buffer time (default: 2 min)
     - Test connectivity

6. **Verify Operation**
   - Display shows "LEAVE IN: XX:XX"
   - Train info scrolls on LED matrix
   - Press button to switch transport modes
   - Check web dashboard for real-time data

### Quick Start Workflow

```
1. Power on ESP32
2. Connect to WiFi (automatic after first setup)
3. Sync time via NTP
4. Fetch departures from NS API
5. Calculate leave time = departure - travel - buffer
6. Display countdown
7. Alert when time to leave!
```

---

## Troubleshooting Guide

### NS API Issues

#### Problem: No departures fetched
**Solutions:**
1. Verify NS API key is correct in config
2. Check station code is valid (4 letters, e.g., "HTNC")
3. Test API manually: `curl -H "Ocp-Apim-Subscription-Key: YOUR_KEY" "https://gateway.apiportal.ns.nl/reisinformatie-api/api/v2/departures?station=HTNC"`
4. Check WiFi connection status
5. Monitor Serial output for HTTP errors
6. API rate limiting - reduce refresh frequency

#### Problem: JSON parsing errors
**Solutions:**
1. Increase JSON buffer size in code
2. Check ArduinoJson version (v7 recommended)
3. Enable debug logging to see raw API response
4. Handle malformed responses gracefully

---

## Troubleshooting Guide

### MAX7219 Display Issues

#### Problem: Display not working/blank
**Solutions:**
1. Check wiring connections (DIN, CLK, CS, VCC, GND)
2. Verify power supply is 5V and adequate (2A recommended)
3. Check SPI pins match code definitions
4. Try different CS pin (some ESP32 boards have issues with certain pins)
5. Add 1000µF capacitor between VCC and GND
6. Check if MAX7219 is in shutdown mode: `lc.shutdown(0, false);`
7. Verify HARDWARE_TYPE matches your module (FC16_HW, ICSTATION_HW, etc.)

#### Problem: Display shows garbage/random patterns
**Solutions:**
1. Check module orientation (Module 0 should be furthest from ESP32)
2. Verify MAX_DEVICES count matches physical modules
3. Poor quality jumper wires - use shorter, shielded cables
4. Add 100nF ceramic capacitor near each module's VCC/GND
5. Lower SPI clock speed: `SPI.setFrequency(1000000);` // 1MHz
6. Check for loose connections in daisy chain

#### Problem: Some modules work, others don't
**Solutions:**
1. Test each module individually
2. Check DOUT→DIN connections between modules
3. One faulty module can break the chain - bypass to test
4. Power supply insufficient - measure voltage at last module
5. Try powering modules from separate 5V rail

#### Problem: Display dims or flickers
**Solutions:**
1. Insufficient power supply current
2. Use thicker wires for VCC/GND
3. Add power injection at middle modules if using 4+
4. Reduce brightness: `display.setIntensity(5);`
5. Check for voltage drop (should be >4.5V at modules)

#### Problem: Text appears backwards or upside down
**Solutions:**
1. Module orientation issue
2. Try `display.setInvert(true);`
3. Adjust module order in chain
4. Configure hardware type correctly in code

### Rotary Encoder Issues

#### Problem: Encoder not responsive or skips values
**Solutions:**
1. Add hardware debouncing (0.1µF capacitors)
2. Use internal pull-ups: `pinMode(ENC_CLK, INPUT_PULLUP);`
3. Increase debounce delay in software
4. Check for loose connections
5. Try different GPIO pins (avoid ADC-only pins)

#### Problem: Encoder counts wrong direction
**Solutions:**
1. Swap CLK and DT pins in code
2. Or invert the count direction in software

### General ESP32 Issues

#### Problem: ESP32 keeps restarting
**Solutions:**
1. Power supply insufficient (needs 500mA minimum)
2. Remove displays and test - may be brownout
3. Add large capacitor (1000µF) to ESP32 power input
4. Check for code infinite loops or stack overflow
5. Monitor Serial for brownout detector messages

#### Problem: Upload fails
**Solutions:**
1. Hold BOOT button while uploading
2. Check USB cable (data cable, not charge-only)
3. Install/update CH340 or CP2102 drivers
4. Try different upload speed: `upload_speed = 115200`

---

## Resources & References

### Documentation
- [ESP32 Technical Reference](https://www.espressif.com/en/products/socs/esp32)
- [PlatformIO Docs](https://docs.platformio.org)
- [Arduino ESP32 Core](https://github.com/espressif/arduino-esp32)

### MAX7219 Libraries & Documentation
- [MD_MAX72XX Library](https://github.com/MajicDesigns/MD_MAX72XX)
- [MD_Parola Library](https://github.com/MajicDesigns/MD_Parola)
- [LedControl Library](https://github.com/wayoda/LedControl)
- [MAX7219 Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf)

### Display Libraries (Other)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)

### Tutorials
- ESP32 Timer Interrupts
- Rotary Encoder with ESP32
- ESP32 Web Server AsyncTCP
- MAX7219 LED Matrix with ESP32
- Creating custom fonts for LED matrices

---

## Notes

### General
- **MVP Strategy:** Phases 1-3 provide core smart countdown functionality
- Phase 4-5 add polish (alerts, web UI, full configuration)
- Phase 6 is entirely optional advanced features
- **Always connected:** WiFi required for NS API - no offline mode in basic version
- Consider power source early (USB recommended for always-on operation)
- Test alert sounds/volumes in actual use environment
- API calls should be rate-limited (every 30-60 seconds max)

### Smart System Tips
- **Travel Time Accuracy:**
  - Measure your actual walk/bike/bus times to station
  - Add 1-2 min buffer for variability
  - Consider weather conditions (bike takes longer in rain)
  - Morning rush vs evening times may differ

- **Station Selection:**
  - Some cities have multiple stations (e.g., Utrecht vs Utrecht Centraal)
  - Verify station code gives correct platform departures
  - NS API uses 4-letter codes (not always intuitive)

- **API Considerations:**
  - Free NS API tier has rate limits
  - Cache departure data - don't fetch every second
  - Handle API failures gracefully (show last known data)
  - Night trains may have different schedules

- **User Experience:**
  - Display should alternate between countdown and train info
  - Quick transport mode switching is essential (button press)
  - Web UI for configuration - physical controls for operation
  - Alert timing is critical - too early = annoying, too late = miss train

### MAX7219 Specific Tips
- **LED Matrix (8x8):**
  - Start with 1 module to test, then expand to 4
  - MD_Parola library saves significant development time
  - 32x8 pixels is enough for "HH:MM:SS" display
  - Consider scrolling for longer messages
  - Test viewing angle - best perpendicular to display
  - Brightness level 5-8 is usually optimal for indoor use

- **7-Segment Display:**
  - Extremely readable from distance
  - Lower power than full LED matrix
  - Limited to numeric display (perfect for countdown)
  - Can use 8 digits: "HH:MM:SS" + 2 extra for status icons
  - Simple LedControl library is sufficient

- **Power & Wiring:**
  - Always use a 2A+ power supply for 4 modules
  - Keep wires short (< 15cm) for data lines
  - Add capacitors (1000µF on power, 100nF per module)
  - Consider using a breadboard power supply module

- **Display Brightness:**
  - Ensure brightness is readable in various lighting conditions
  - Auto-brightness using LDR sensor (optional enhancement)
  - Brightness level 15 = very bright, high power consumption
  - Brightness level 1-3 = low power but may be dim in daylight

---

**Last Updated:** 2025-12-25
**Project Type:** Smart Departure Countdown with NS API Integration
**Project Status:** Planning Phase
**MVP Scope:** Phases 1-5 (Hardware + WiFi + NS API + Alerts + Web UI)
**Target Completion:** TBD based on implementation pace

---

## Quick Feature Summary

✅ **Core Features (MVP):**
- Real-time train departures from NS API
- Smart countdown: "Leave in X minutes"
- Multiple transport modes (walk/bike/bus)
- LED Matrix/7-Segment display
- Web UI for configuration
- REST API (OpenAPI)
- Alert system (audio + visual)

🔧 **Configuration:**
- NS API key
- Station code (e.g., HTNC)
- Travel times per mode
- Buffer time
- WiFi credentials

📱 **User Interaction:**
- Physical buttons: Transport mode switching
- Web dashboard: Live departures & config
- REST API: Mobile app integration

🚀 **Future Enhancements (Phase 6):**
- Google Maps integration
- Multi-station support
- Smart scheduling
- Home automation (MQTT)
- Mobile app
