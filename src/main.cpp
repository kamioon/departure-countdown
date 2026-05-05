#include <Arduino.h>
#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "config.h"
#include "time_manager.h"
#include "ns_api.h"
#include "countdown_calc.h"
#include "display.h"
#include "alerts.h"
#include "input.h"
#include "web_server.h"
#include "pins.h"

// System managers
ConfigManager configManager;
TimeManager timeManager;
NSApiClient nsApiClient;
CountdownCalculator countdownCalc;
DisplayManager displayManager;
AlertManager alertManager;
InputHandler inputHandler;
WebServerManager webServerManager;

// System state
bool systemInitialized = false;
unsigned long lastApiUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastModeSwitch = 0;
bool showBikeMode = false;  // Alternate between Walk and Bike
const unsigned long API_UPDATE_INTERVAL = 120000;  // 2 minutes
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;  // 1 second
const unsigned long MODE_SWITCH_INTERVAL = 5000;  // 5 seconds

// Background fetch
static SemaphoreHandle_t nsMutex = NULL;
static volatile bool fetchInProgress = false;

// Cached departure timestamps used to keep the countdown ticking during a background fetch.
// These are plain unsigned longs (not pointers), so they're safe across task boundaries.
struct DepCache {
    unsigned long leaveTime = 0;
    unsigned long departureTime = 0;
    String direction = "";
    bool valid = false;
};
static DepCache cacheWalk, cacheBike;

static void triggerFetch();  // forward declaration — defined after initializeSystem

static String readSerialLine(const char* prompt, bool echoInput = true, bool maskInput = false,
                             unsigned long timeoutMs = 30000) {
    Serial.print(prompt);
    String line;
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        while (Serial.available() > 0) {
            char c = Serial.read();
            if (c == '\r') {
                continue;
            }
            if (c == '\n') {
                line.trim();
                if (echoInput) {
                    Serial.println();
                }
                return line;
            }
            if (c == '\b' || c == 0x7f) {
                if (line.length() > 0) {
                    line.remove(line.length() - 1);
                    if (echoInput) {
                        Serial.print("\b \b");
                    }
                }
                continue;
            }
            line += c;
            if (echoInput) {
                if (maskInput) {
                    Serial.print('*');
                } else {
                    Serial.print(c);
                }
            }
        }
        delay(10);
    }
    line.trim();
    if (echoInput && line.length() > 0) {
        Serial.println();
    }
    return line;
}

static bool isYesResponse(const String& response) {
    return response.length() > 0 && (response[0] == 'y' || response[0] == 'Y');
}

static bool promptWiFiCredentials() {
    String ssid = readSerialLine("WiFi SSID: ");
    if (ssid.length() == 0) {
        Serial.println("WiFi credentials not provided");
        return false;
    }
    String password = readSerialLine("WiFi Password (blank for open): ");
    configManager.setWiFiCredentials(ssid, password);
    configManager.save();
    Serial.println("WiFi credentials saved");
    return true;
}

static bool promptNsApiKey() {
    String apiKey = readSerialLine("NS API key: ");
    if (apiKey.length() == 0) {
        Serial.println("NS API key not provided");
        return false;
    }
    configManager.setNsApiKey(apiKey);
    configManager.save();
    Serial.println("NS API key saved");
    return true;
}

struct AlertSelection {
    const CountdownInfo* info;
    TransportMode mode;
};

static AlertSelection selectAlertInfo(const CountdownInfo& walkInfo,
                                      const CountdownInfo& bikeInfo) {
    // Use countdown state rather than departure pointer (pointer may be null during background fetch)
    bool walkValid = walkInfo.state != COUNTDOWN_IDLE &&
                     walkInfo.state != COUNTDOWN_ERROR &&
                     walkInfo.secondsUntilLeave > 0;
    bool bikeValid = bikeInfo.state != COUNTDOWN_IDLE &&
                     bikeInfo.state != COUNTDOWN_ERROR &&
                     bikeInfo.secondsUntilLeave > 0;

    if (walkValid && bikeValid) {
        if (walkInfo.secondsUntilLeave <= bikeInfo.secondsUntilLeave) {
            return {&walkInfo, WALK};
        }
        return {&bikeInfo, BIKE};
    }

    if (walkValid) {
        return {&walkInfo, WALK};
    }

    if (bikeValid) {
        return {&bikeInfo, BIKE};
    }

    return {nullptr, WALK};
}
static void beepStartup() {
    // Audible startup check that works for passive (tone) and active (DC) buzzers.
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);
    for (int i = 0; i < 3; i++) {
        tone(BUZZER_PIN, 2000, 300);
        delay(350);
    }
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(250);
    digitalWrite(BUZZER_PIN, LOW);
}



bool initializeSystem() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=== Departure Countdown System ===");
    Serial.println("Initializing...\n");

    Serial.print("Loading configuration... ");
    if (!configManager.begin()) {
        Serial.println("FAILED");
        return false;
    }
    Serial.println("OK");

    Serial.print("Initializing display... ");
    if (!displayManager.begin()) {
        Serial.println("FAILED");
        return false;
    }
    Serial.println("OK");
    beepStartup();

    displayManager.showStartupAnimation();
    Config& config = configManager.getConfig();

    if (strlen(config.wifiSsid) == 0) {
        Serial.println("WiFi SSID not set. Enter credentials via Serial (press Enter to skip).");
        displayManager.showMessage("SET WIFI");
        promptWiFiCredentials();
    } else {
        String reconfig = readSerialLine("Reconfigure WiFi? (y/N): ", true, false, 5000);
        if (isYesResponse(reconfig)) {
            displayManager.showMessage("SET WIFI");
            promptWiFiCredentials();
        }
    }

    if (strlen(config.nsApiKey) == 0) {
        Serial.println("NS API key not set. Enter key via Serial (press Enter to skip).");
        displayManager.showMessage("SET API");
        promptNsApiKey();
    } else {
        String reconfigApi = readSerialLine("Reconfigure NS API key? (y/N): ", true, false, 5000);
        if (isYesResponse(reconfigApi)) {
            displayManager.showMessage("SET API");
            promptNsApiKey();
        }
    }

    for (int attempt = 0; attempt < 2; attempt++) {
        displayManager.showMessage("WIFI...");

        if (strlen(config.wifiSsid) == 0) {
            Serial.println("WiFi credentials not set. Configure via Serial.");
            displayManager.showError("NO WIFI");
            delay(3000);
            break;
        }

        Serial.print("Connecting to WiFi... ");

        WiFi.disconnect(true);
        delay(100);

        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);

        // Set power save mode off for stable connection
        WiFi.setSleep(false);

        WiFi.begin(config.wifiSsid, config.wifiPassword);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 40) {  // Increased timeout
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println(" FAILED");
            Serial.println("WiFi connection failed. Please check credentials.");
            displayManager.showError("NO WIFI");
            if (attempt == 0) {
                String retry = readSerialLine("Re-enter WiFi credentials? (y/N): ", true, false, 30000);
                if (isYesResponse(retry)) {
                    displayManager.showMessage("SET WIFI");
                    promptWiFiCredentials();
                    continue;
                }
            }
            delay(3000);
            break;
        }

        Serial.println(" OK");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("DNS: ");
        Serial.println(WiFi.dnsIP());
        Serial.print("Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");

        // Configure DNS servers explicitly (helps with some routers)
        IPAddress primaryDNS(8, 8, 8, 8);       // Google DNS
        IPAddress secondaryDNS(1, 1, 1, 1);     // Cloudflare DNS
        WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), primaryDNS, secondaryDNS);

        Serial.println("DNS configured: 8.8.8.8, 1.1.1.1");

        // Wait for DNS to stabilize
        delay(2000);

        displayManager.showMessage("WIFI OK");
        delay(1000);
        break;
    }

    // Initialize time manager
    Serial.print("Initializing time... ");
    if (!timeManager.begin()) {
        Serial.println("FAILED");
        displayManager.showError("TIME INIT");
        delay(2000);
        // Continue - time will sync later
    } else {
        Serial.println("OK");

        unsigned long currentTime = timeManager.getCurrentTime();
        String timeStr = timeManager.formatTime(currentTime);
        Serial.print("Current time: ");
        Serial.println(timeStr);

        time_t rawtime = (time_t)currentTime;
        struct tm* timeinfo = localtime(&rawtime);
        char dateBuffer[20];
        sprintf(dateBuffer, "%04d-%02d-%02d",
                timeinfo->tm_year + 1900,
                timeinfo->tm_mon + 1,
                timeinfo->tm_mday);
        Serial.print("Current date: ");
        Serial.println(dateBuffer);
    }

    Serial.print("Initializing NS API... ");
    String apiKey = String(configManager.getConfig().nsApiKey);
    nsApiClient.setTimeManager(&timeManager);
    if (apiKey.length() == 0) {
        Serial.println("NO API KEY");
        displayManager.showMessage("NO API KEY");
        delay(2000);
    } else if (!nsApiClient.begin(apiKey)) {
        Serial.println("FAILED");
        displayManager.showError("API INIT");
        delay(2000);
    } else {
        Serial.println("OK");
    }

    Serial.print("Initializing countdown... ");
    if (!countdownCalc.begin(&configManager, &timeManager)) {
        Serial.println("FAILED");
        return false;
    }
    Serial.println("OK");

    Serial.print("Initializing alerts... ");
    if (!alertManager.begin(&configManager)) {
        Serial.println("FAILED");
        return false;
    }
    Serial.println("OK");

    Serial.print("Initializing input... ");
    if (!inputHandler.begin()) {
        Serial.println("FAILED");
        return false;
    }
    Serial.println("OK");

    // Create the mutex that serialises access to NSApiClient
    nsMutex = xSemaphoreCreateMutex();

    // Start web server (only when WiFi is up)
    if (WiFi.status() == WL_CONNECTED) {
        webServerManager.begin(&configManager, &nsApiClient, &countdownCalc);
        webServerManager.setFetchCallback(triggerFetch);
        webServerManager.setDisplayConnected(true);  // display is always init'd before we reach here
    }

    Serial.println("\n=== System Ready ===\n");
    displayManager.showMessage("READY");
    delay(1000);

    return true;
}

// ---------------------------------------------------------------------------
// Background fetch task — runs in a separate FreeRTOS task so the display
// loop is never blocked by a slow HTTPS request.
// ---------------------------------------------------------------------------

static void fetchTaskFn(void*) {
    // Hold the API mutex for the entire fetch so updateDisplay() falls back
    // to cached timestamps while new data is being downloaded.
    xSemaphoreTake(nsMutex, portMAX_DELAY);

    String stationCode = String(configManager.getConfig().stationCode);
    if (stationCode.length() == 0) {
        Serial.println("[fetch] No station code");
    } else {
        Serial.println("[fetch] Starting background fetch for " + stationCode);
        int count = nsApiClient.fetchDepartures(stationCode, 10);
        if (count < 0) {
            Serial.println("[fetch] Failed: " + nsApiClient.getLastError());
        } else {
            Serial.print("[fetch] Got ");
            Serial.print(count);
            Serial.println(" departures");
        }
    }

    xSemaphoreGive(nsMutex);
    fetchInProgress = false;
    vTaskDelete(NULL);
}

static void triggerFetch() {
    if (fetchInProgress) return;
    if (!nsMutex) return;
    fetchInProgress = true;
    // 10 kB stack — SSL + HTTP + JSON parsing need plenty of space
    xTaskCreate(fetchTaskFn, "nsFetch", 10240, NULL, 1, NULL);
    Serial.println("Background fetch started");
}

// Recompute a CountdownInfo purely from cached timestamps — no pointer access.
static CountdownInfo recalcFromCache(const DepCache& cache, unsigned long currentTime) {
    CountdownInfo info;
    info.leaveTime = cache.leaveTime;
    info.departureTime = cache.departureTime;
    info.direction = cache.direction;
    info.departure = nullptr;
    info.secondsUntilLeave = (long)(cache.leaveTime - currentTime);
    info.secondsUntilDeparture = (long)(cache.departureTime - currentTime);
    info.state = CountdownCalculator::getCountdownState(info.secondsUntilLeave);
    info.message = CountdownCalculator::getStateMessage(info.state);
    return info;
}

// When a background fetch is in progress the API mutex is held by the fetch task,
// so we fall back to cached departure timestamps to keep the countdown ticking live.
void updateDisplay() {
    unsigned long currentTime = timeManager.getCurrentTime();
    int bufferTime = configManager.getConfig().bufferTime;
    int walkTime = configManager.getTravelTime(WALK);
    int bikeTime = configManager.getTravelTime(BIKE);

    CountdownInfo infoWalk, infoBike;
    bool walkAvail = false, bikeAvail = false;

    if (xSemaphoreTake(nsMutex, 0) == pdTRUE) {
        // ---- Fresh data path (mutex acquired, no fetch in progress) ----
        TransportMode originalMode = configManager.getConfig().activeMode;

        const Departure* dWalk = nsApiClient.getNextDeparture(currentTime, walkTime, bufferTime);
        const Departure* dBike = nsApiClient.getNextDeparture(currentTime, bikeTime, bufferTime);

        if (dWalk) {
            configManager.getConfig().activeMode = WALK;
            infoWalk = countdownCalc.calculate(dWalk);
            // Cache plain values — safe to read without the mutex later
            cacheWalk.leaveTime = infoWalk.leaveTime;
            cacheWalk.departureTime = infoWalk.departureTime;
            cacheWalk.direction = dWalk->direction;
            cacheWalk.valid = true;
            infoWalk.direction = dWalk->direction;
            infoWalk.departure = nullptr;  // clear pointer before releasing mutex
            walkAvail = true;
        }
        if (dBike) {
            configManager.getConfig().activeMode = BIKE;
            infoBike = countdownCalc.calculate(dBike);
            cacheBike.leaveTime = infoBike.leaveTime;
            cacheBike.departureTime = infoBike.departureTime;
            cacheBike.direction = dBike->direction;
            cacheBike.valid = true;
            infoBike.direction = dBike->direction;
            infoBike.departure = nullptr;
            bikeAvail = true;
        }

        configManager.getConfig().activeMode = originalMode;
        xSemaphoreGive(nsMutex);

        if (!walkAvail && !bikeAvail) {
            // No catchable trains — trigger a background refresh
            Serial.println("No catchable departures, triggering refresh");
            triggerFetch();
            displayManager.showMessage("SYNC");
            return;
        }
    } else {
        // ---- Cached path (fetch task holds the mutex) ----
        // Recalculate countdown from stored timestamps; no pointer access needed.
        if (cacheWalk.valid) {
            infoWalk = recalcFromCache(cacheWalk, currentTime);
            // Treat a long-departed train as invalid so we don't show stale data forever
            walkAvail = (infoWalk.secondsUntilDeparture > -60);
        }
        if (cacheBike.valid) {
            infoBike = recalcFromCache(cacheBike, currentTime);
            bikeAvail = (infoBike.secondsUntilDeparture > -60);
        }
        if (!walkAvail && !bikeAvail) {
            displayManager.showMessage("SYNC");
            return;
        }
    }

    // Alternate between Walk and Bike every 5 seconds
    unsigned long now = millis();
    if (now - lastModeSwitch >= MODE_SWITCH_INTERVAL) {
        showBikeMode = !showBikeMode;
        lastModeSwitch = now;
    }

    if (showBikeMode && !bikeAvail) showBikeMode = false;
    if (!showBikeMode && !walkAvail) showBikeMode = true;

    CountdownInfo& info = showBikeMode ? infoBike : infoWalk;
    TransportMode displayMode = showBikeMode ? BIKE : WALK;

    displayManager.showCountdown(info, displayMode, false);

    AlertSelection alertSel = selectAlertInfo(infoWalk, infoBike);
    if (alertSel.info) {
        alertManager.update(*alertSel.info, alertSel.mode);
    }

    // Push a status snapshot so the web API always has fresh data
    webServerManager.updateStatus(
        walkAvail ? infoWalk.secondsUntilLeave : 0,
        bikeAvail ? infoBike.secondsUntilLeave : 0,
        fetchInProgress,
        walkAvail ? infoWalk.direction : "",
        bikeAvail ? infoBike.direction : "",
        WiFi.RSSI()
    );

    Serial.print("Leave in: ");
    Serial.print(CountdownCalculator::formatCountdown(info.secondsUntilLeave));
    Serial.print(" [");
    Serial.print(fetchInProgress ? "SYNC" : "live");
    Serial.print("] | ");
    Serial.println(CountdownCalculator::getStateName(info.state));
}

void handleInputs() {
    ButtonEvent event = inputHandler.getButtonEvent();

    switch (event) {
        case BTN_MODE_PRESS:
            // Cycle transport mode
            {
                Config& config = configManager.getConfig();
                int nextMode = (int)config.activeMode + 1;
                if (nextMode > BUS) nextMode = WALK;
                config.activeMode = (TransportMode)nextMode;
                configManager.save();

                Serial.print("Transport mode changed to: ");
                Serial.println(ConfigManager::getModeName(config.activeMode));

                displayManager.showMessage(ConfigManager::getModeName(config.activeMode));
                delay(1000);
            }
            break;

        case BTN_START_PRESS:
            // Force refresh (background — display keeps running)
            Serial.println("Manual refresh requested");
            triggerFetch();
            break;

        case BTN_RESET_LONG_PRESS:
            // Factory reset
            Serial.println("Factory reset requested");
            displayManager.showMessage("RESET");
            delay(1000);
            configManager.factoryReset();
            displayManager.showMessage("DONE");
            delay(1000);
            ESP.restart();
            break;

        default:
            break;
    }
}

void setup() {
    systemInitialized = initializeSystem();

    if (!systemInitialized) {
        Serial.println("FATAL: System initialization failed");
        while (true) {
            delay(1000);
        }
    }

    // Kick off the first fetch in the background — display stays active immediately
    if (WiFi.status() == WL_CONNECTED) {
        triggerFetch();
    }
}

void loop() {
    unsigned long now = millis();

    inputHandler.update();
    handleInputs();
    timeManager.update();

    if (WiFi.status() == WL_CONNECTED && !fetchInProgress &&
        now - lastApiUpdate >= API_UPDATE_INTERVAL) {
        triggerFetch();
        lastApiUpdate = now;
    }

    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = now;
    }

    displayManager.update();
    delay(10);
}
