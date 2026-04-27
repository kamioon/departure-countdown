#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

class AsyncWebServer;
class AsyncWebServerRequest;

#include "config.h"
#include "ns_api.h"
#include "countdown_calc.h"

/**
 * @brief Web Server Manager
 *
 * Handles:
 * - WiFi connection management
 * - AsyncWebServer for web UI
 * - RESTful API endpoints
 * - Configuration interface
 */
class WebServerManager {
public:
    WebServerManager();
    ~WebServerManager();

    /**
     * @brief Initialize web server
     * @param config Configuration manager reference
     * @param apiClient NS API client reference
     * @param calculator Countdown calculator reference
     * @return true if initialization successful
     */
    bool begin(ConfigManager* config, NSApiClient* apiClient,
               CountdownCalculator* calculator);

    /**
     * @brief Connect to WiFi
     * @param ssid WiFi SSID
     * @param password WiFi password
     * @param timeout Timeout in seconds
     * @return true if connected
     */
    bool connectWiFi(const char* ssid, const char* password, int timeout = 30);

    /**
     * @brief Check if WiFi is connected
     * @return true if connected
     */
    bool isConnected();

    /**
     * @brief Get IP address
     * @return IP address string
     */
    String getIPAddress();

    /**
     * @brief Update web server (call in loop)
     */
    void update();

private:
    AsyncWebServer* server;

    ConfigManager* configManager;
    NSApiClient* nsApiClient;
    CountdownCalculator* countdownCalc;

    /**
     * @brief Setup API routes
     */
    void setupRoutes();

    /**
     * @brief Setup web UI routes
     */
    void setupWebUI();

    /**
     * @brief Handle GET /api/status
     */
    void handleApiStatus(AsyncWebServerRequest* request);

    /**
     * @brief Handle GET /api/departures
     */
    void handleApiDepartures(AsyncWebServerRequest* request);

    /**
     * @brief Handle GET /api/config
     */
    void handleApiConfig(AsyncWebServerRequest* request);

    /**
     * @brief Handle POST /api/config
     */
    void handleApiConfigPost(AsyncWebServerRequest* request, uint8_t* data, size_t len);

    /**
     * @brief Handle POST /api/transport-mode
     */
    void handleApiTransportMode(AsyncWebServerRequest* request);
};

#endif // WEB_SERVER_H
