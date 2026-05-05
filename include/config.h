#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

class Preferences;

enum TransportMode {
    WALK = 0,
    BIKE = 1,
    BUS  = 2
};

struct Config {
    char wifiSsid[33];
    char wifiPassword[65];
    char nsApiKey[65];
    char stationCode[9];
    int walkTime;
    int bikeTime;
    int busTime;
    TransportMode activeMode;
    int bufferTime;
    bool audioAlertsEnabled;
    bool ledAlertsEnabled;
    bool alertAtLeaveTime;
    bool alertFiveMinBefore;

    Config() {
        strcpy(wifiSsid, "");
        strcpy(wifiPassword, "");
        strcpy(nsApiKey, "");
        strcpy(stationCode, "");
        walkTime = 15;
        bikeTime = 8;
        busTime = 12;
        activeMode = WALK;
        bufferTime = 2;
        audioAlertsEnabled = true;
        ledAlertsEnabled = true;
        alertAtLeaveTime = true;
        alertFiveMinBefore = true;
    }
};

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    bool begin();
    bool load();
    bool save();
    bool factoryReset();
    Config& getConfig();

    void setNsApiKey(const String& apiKey);
    void setWiFiCredentials(const String& ssid, const String& password);
    void setStationCode(const String& code);
    void setTravelTime(TransportMode mode, int minutes);
    void setActiveMode(TransportMode mode);
    void setBufferTime(int minutes);
    void setAudioAlerts(bool enabled);
    void setLedAlerts(bool enabled);

    int getActiveTravelTime();
    int getTravelTime(TransportMode mode);
    bool validate();

    static String getModeName(TransportMode mode);
    static String getModeIcon(TransportMode mode);

private:
    Preferences* preferences;
    Config config;
    const char* NAMESPACE = "departure";

    void loadDefaults();
};

#endif // CONFIG_H
