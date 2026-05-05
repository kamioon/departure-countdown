#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <functional>

class AsyncWebServer;
class AsyncWebServerRequest;

#include "config.h"
#include "ns_api.h"
#include "countdown_calc.h"

class WebServerManager {
public:
    using FetchCallback = std::function<void()>;

    WebServerManager();
    ~WebServerManager();

    bool begin(ConfigManager* config, NSApiClient* apiClient, CountdownCalculator* calculator);
    bool isConnected();
    String getIPAddress();
    void update();  // no-op — AsyncWebServer is fully async
    void setFetchCallback(FetchCallback cb);

    // Call from the main loop after each display update so /api/status always
    // returns fresh data without touching the NSApiClient buffer from the network task.
    void updateStatus(long walkSeconds, long bikeSeconds, bool fetching,
                      const String& walkDir, const String& bikeDir, int rssi);

    // Call once after begin(), after confirming the display initialised.
    void setDisplayConnected(bool connected);

private:
    AsyncWebServer* server;

    ConfigManager* configManager;
    NSApiClient* nsApiClient;
    CountdownCalculator* countdownCalc;
    FetchCallback fetchCallback;

    // Written from the main loop, read from request handlers.
    // Plain POD values so cross-task reads are safe without a mutex.
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
