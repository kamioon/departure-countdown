# How It Works

This project is a small ESP32-S3 system that shows when you should leave for the next train.
It connects to WiFi, fetches live departures from the NS API, then counts down based on your travel time.

## Big Picture Flow

1. Boot the ESP32 and load saved settings (WiFi + API key) from NVS.
2. If WiFi or API key is missing, ask for them on the Serial Monitor.
3. Connect to WiFi and sync time with NTP (RTC is optional).
4. Fetch departures from the NS API.
5. Calculate how long until you must leave (walk/bike/bus + buffer).
6. Show the countdown on the LED matrix and play alerts as time gets short.

## Main Parts of the Code

### main.cpp
`src/main.cpp` starts everything and runs the main loop:
- `initializeSystem()` sets up config, display, WiFi, time, and API.
- `updateDepartureData()` calls the NS API and stores departures.
- `handleInputs()` reacts to buttons (mode, refresh, reset).
- `loop()` refreshes data, updates the display, and triggers alerts.

### ConfigManager
`include/config.h` + `src/config_esp32.cpp` + `src/config_common.cpp` store settings in NVS:
- WiFi SSID/password
- NS API key
- Station code
- Travel times and buffer

If WiFi or API key is empty, `main.cpp` asks for them over Serial and saves them.

### TimeManager
`include/time_manager.h` + `src/time_manager_esp32.cpp` + `src/time_manager_common.cpp` handle time:
- Syncs time using NTP
- Uses the RTC if connected
- Formats timestamps for display

### NSApiClient
`include/ns_api.h` + `src/ns_api.cpp` handle API calls:
- Builds the NS API URL
- Sends HTTPS request
- Parses JSON response
- Keeps a list of departures

### CountdownCalculator
`include/countdown_calc.h` + `src/countdown_calc.cpp` compute leave time:
- Uses current time + travel time + buffer
- Picks the next suitable train
- Returns a countdown and state (safe/ready/urgent)

### DisplayManager
`include/display.h` + `src/display_esp32.cpp` control the LED matrix:
- Shows countdown
- Scrolls train info
- Shows status/errors (WiFi, API, time)

### AlertManager
`include/alerts.h` + `src/alerts_esp32.cpp` drives buzzer + RGB LED:
- Green/yellow/orange/red based on urgency
- Beeps for “leave soon” and “train departing” (audio disabled by default — enable via web UI or `audioAlertsEnabled`)
- A startup triple-beep always plays regardless of the audio setting

### WebServerManager
`include/web_server.h` + `src/web_server_esp32.cpp` runs an async HTTP server on port 80:
- Serves a browser-based configuration UI
- REST API endpoints: `GET/POST /api/config`, `GET /api/status`, `GET /api/departures`, `POST /api/transport`, `POST /api/fetch`
- Status is updated each display tick via `updateStatus()` so the web API always reflects live data
- Only started when WiFi is connected; fetch is triggered via callback so the web server never touches FreeRTOS tasks directly

### InputHandler
`include/input.h` + `src/input_esp32.cpp` reads buttons and encoder:
- MODE button cycles walk/bike/bus
- START button triggers manual refresh
- RESET long press clears saved settings

## How the Data Moves

- WiFi connects → time sync → API fetch → web server starts
- Departures → countdown calculator → display + alerts + web status cache
- Button or web UI changes travel mode → countdown recalculates → display updates

## Where to Start Reading

If you are new to embedded C++:
1. `src/main.cpp` to see the full flow.
2. `src/config_common.cpp` to understand saved settings.
3. `src/ns_api.cpp` to see the network request.
4. `src/display_esp32.cpp` and `src/alerts_esp32.cpp` for output behavior.
5. `src/web_server_esp32.cpp` for the HTTP server and REST API.

## Platform-Specific Files

This project keeps hardware code separate from test code:
- `*_esp32.cpp` files contain real hardware code for the ESP32 build.
- `*_stub.cpp` and `*_native.cpp` files are small no-op versions for native tests.

This keeps the main headers clean and avoids `#ifdef` blocks in core logic.

## Notes

- Secrets are not stored in source code.
- Settings are saved to NVS and survive power loss.
