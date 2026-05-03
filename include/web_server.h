#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <functional>

class AsyncWebServer;
class AsyncWebServerRequest;

#include "config.h"
#include "ns_api.h"
#include "countdown_calc.h"

/**
 * @brief Web Server Manager
 *
 * Handles:
 * - AsyncWebServer for web UI (port 80)
 * - RESTful API endpoints for smarthome integration
 * - Configuration read/write via JSON
 */
class WebServerManager {
public:
    using FetchCallback = std::function<void()>;

    WebServerManager();
    ~WebServerManager();

    /**
     * @brief Initialize and start the web server
     * @param config Configuration manager reference
     * @param apiClient NS API client reference
     * @param calculator Countdown calculator reference
     * @return true if server started successfully
     */
    bool begin(ConfigManager* config, NSApiClient* apiClient,
               CountdownCalculator* calculator);

    /**
     * @brief Check if WiFi is connected
     */
    bool isConnected();

    /**
     * @brief Get device IP address string
     */
    String getIPAddress();

    /**
     * @brief Update web server (no-op; AsyncWebServer is fully async)
     */
    void update();

    /**
     * @brief Register callback invoked when a fetch is requested via the API
     */
    void setFetchCallback(FetchCallback cb);

    /**
     * @brief Push current countdown status into the web server status cache.
     *        Call this from the main loop after each display update so the
     *        /api/status endpoint always returns fresh data without needing
     *        to touch the NSApiClient buffer from the network task.
     */
    void updateStatus(long walkSeconds, long bikeSeconds, bool fetching,
                      const String& walkDir, const String& bikeDir, int rssi);

    /**
     * @brief Report whether the LED matrix display is connected.
     *        Call once after begin(), typically from initializeSystem().
     */
    void setDisplayConnected(bool connected);

private:
    AsyncWebServer* server;

    ConfigManager* configManager;
    NSApiClient* nsApiClient;
    CountdownCalculator* countdownCalc;
    FetchCallback fetchCallback;

    // Snapshot of system state — written from the main loop, read from
    // request handlers. Plain POD values so cross-task reads are safe.
    struct StatusCache {
        long walkSecondsUntilLeave = 0;
        long bikeSecondsUntilLeave = 0;
        bool fetchInProgress = false;
        String walkDir;
        String bikeDir;
        int rssi = 0;
        bool displayConnected = false;
    } statusCache;

    void setupRoutes();
    void setupWebUI();

    void handleApiStatus(AsyncWebServerRequest* request);
    void handleApiDepartures(AsyncWebServerRequest* request);
    void handleApiConfig(AsyncWebServerRequest* request);
    void handleApiConfigPost(AsyncWebServerRequest* request, uint8_t* data, size_t len);
    void handleApiTransportMode(AsyncWebServerRequest* request, uint8_t* data, size_t len);
};

#endif // WEB_SERVER_H
