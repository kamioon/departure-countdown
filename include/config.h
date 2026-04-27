#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

class Preferences;

/**
 * @brief Transport mode enumeration
 */
enum TransportMode {
    WALK = 0,
    BIKE = 1,
    BUS = 2
};

/**
 * @brief Configuration structure
 */
struct Config {
    char wifiSsid[33];             // WiFi SSID (max 32 chars)
    char wifiPassword[65];         // WiFi password (max 64 chars)
    char nsApiKey[65];              // NS API subscription key
    char stationCode[5];            // Station code (e.g., "HTNC")
    int walkTime;                   // Walk time in minutes
    int bikeTime;                   // Bike time in minutes
    int busTime;                    // Bus time in minutes
    TransportMode activeMode;       // Active transport mode
    int bufferTime;                 // Buffer time in minutes
    bool audioAlertsEnabled;        // Audio alerts on/off
    bool ledAlertsEnabled;          // LED alerts on/off
    bool alertAtLeaveTime;          // Alert when it's time to leave
    bool alertFiveMinBefore;        // Alert 5 min before departure

    Config() {
        strcpy(wifiSsid, "");
        strcpy(wifiPassword, "");
        strcpy(nsApiKey, "");
        strcpy(stationCode, "HTNC");
        walkTime = 20;
        bikeTime = 8;
        busTime = 12;
        activeMode = WALK;
        bufferTime = 2;
        audioAlertsEnabled = false;
        ledAlertsEnabled = true;
        alertAtLeaveTime = true;
        alertFiveMinBefore = true;
    }
};

/**
 * @brief Manages system configuration
 *
 * Handles:
 * - Save/load settings to Preferences (NVS)
 * - Configuration validation
 * - Factory reset functionality
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    /**
     * @brief Initialize configuration manager
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Load configuration from storage
     * @return true if load successful
     */
    bool load();

    /**
     * @brief Save configuration to storage
     * @return true if save successful
     */
    bool save();

    /**
     * @brief Reset configuration to factory defaults
     * @return true if reset successful
     */
    bool factoryReset();

    /**
     * @brief Get current configuration
     * @return Reference to current config
     */
    Config& getConfig();

    /**
     * @brief Set NS API key
     * @param apiKey API key string
     */
    void setNsApiKey(const String& apiKey);

    /**
     * @brief Set WiFi credentials
     * @param ssid WiFi SSID
     * @param password WiFi password
     */
    void setWiFiCredentials(const String& ssid, const String& password);

    /**
     * @brief Set station code
     * @param code Station code (e.g., "HTNC")
     */
    void setStationCode(const String& code);

    /**
     * @brief Set travel time for a mode
     * @param mode Transport mode
     * @param minutes Travel time in minutes
     */
    void setTravelTime(TransportMode mode, int minutes);

    /**
     * @brief Set active transport mode
     * @param mode Transport mode to activate
     */
    void setActiveMode(TransportMode mode);

    /**
     * @brief Set buffer time
     * @param minutes Buffer time in minutes
     */
    void setBufferTime(int minutes);

    /**
     * @brief Get travel time for active mode
     * @return Travel time in minutes
     */
    int getActiveTravelTime();

    /**
     * @brief Get travel time for specific mode
     * @param mode Transport mode
     * @return Travel time in minutes
     */
    int getTravelTime(TransportMode mode);

    /**
     * @brief Enable/disable audio alerts
     * @param enabled true to enable
     */
    void setAudioAlerts(bool enabled);

    /**
     * @brief Enable/disable LED alerts
     * @param enabled true to enable
     */
    void setLedAlerts(bool enabled);

    /**
     * @brief Validate configuration
     * @return true if configuration is valid
     */
    bool validate();

    /**
     * @brief Get transport mode name
     * @param mode Transport mode
     * @return Mode name as string
     */
    static String getModeName(TransportMode mode);

    /**
     * @brief Get transport mode icon
     * @param mode Transport mode
     * @return Mode icon as string
     */
    static String getModeIcon(TransportMode mode);

private:
    Preferences* preferences;
    Config config;
    const char* NAMESPACE = "departure";

    /**
     * @brief Load default configuration
     */
    void loadDefaults();
};

#endif // CONFIG_H
