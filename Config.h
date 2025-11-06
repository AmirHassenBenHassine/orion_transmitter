#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
// Feature Flags - Disable unused features
#define ENABLE_BLE 1
#define ENABLE_OTA 1
#define DEBUG_MODE 1         // 0 = Production, 1 = Debug

// Debug macros
#if DEBUG_MODE == 1
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

// Pin Definitions - Sensors
#define MAX_PHASES 3
#define VOLTAGE_PIN 32
#define CURRENT_PIN_R 39
#define CURRENT_PIN_Y 34
#define CURRENT_PIN_B 35

// Pin Definitions - LEDs
#define POWER_LED_PIN 14
#define WIFI_LED_PIN 27
#define CHARGING_LED_PIN 26
#define BATTERY_LED_PIN 25

// Pin Definitions - Buttons
#define WIFI_PAIRING_BTN 4
#define HARD_RESET_BTN 13
#define CHARGING_STATUS_PIN 33

// Pin Definitions - Battery
#define BATTERY_VOLTAGE_PIN 36
#define BATTERY_SAMPLES 20
#define VOLTAGE_DIVIDER_RATIO 2.0
#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_MIN_VOLTAGE 3.0

// Timing Constants
#define DATA_SEND_INTERVAL 30000        // 30 seconds
#define TRIGGER_CHECK_INTERVAL 60000    // 1 minute
#define WAKEUP_TIME_US (15*60000000)    // 15 minutes
#define LED_BLINK_FAST 200
#define LED_BLINK_SLOW 500
#define LED_BLINK_AP 1000

// Measurement Constants
#define CURRENT_NOISE_THRESHOLD 0.09
#define NOISE_THRESHOLD 0.1
#define MAX_CURRENT 100.0
#define NUM_SAMPLES 100

// Network Configuration
#define MQTT_PORT 1883
#define LOCAL_BROKER_HOST "orion.local"
#define ESP_HOSTNAME "esp32"
#define AP_SSID "OrionSetup"
#define AP_PASSWORD "Orion2025"

// OTA Configuration
#define FIRMWARE_URL "https://github.com/AmirHassenBenHassine/OTA_Github/raw/refs/heads/main/finalOrion_transmiter2.1.ino.bin"
#define OTA_CHECK_INTERVAL 86400000 // 24 hours

// BLE Configuration
// #define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// MQTT Topics
#define TOPIC_ENERGY_METRICS "energy/metrics"
#define TOPIC_WIFI_SCAN "orion/scan"
#define TOPIC_WIFI_CONFIG "orion/config"
#define TOPIC_WIFI_CONFIRM "orion/confirm"
#define TOPIC_WIFI_REFRESH "orion/refresh"
#define TOPIC_WIFI_STATUS "orion/status"
#define TOPIC_TRIGGER "orion/trigger"
#define TOPIC_ESP32_PHASE "esp32/phase/config"
#define TOPIC_ESP32_STATUS "esp32/status"

// NTP Configuration
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 3600
#define DAYLIGHT_OFFSET_SEC 3600

// Calibration Values
extern float voltageCal;
extern float currentCal[MAX_PHASES];
extern const int currentPins[MAX_PHASES];

// Boot Modes
enum BootMode {BOOT_NORMAL,BOOT_WIFI_PAIR,BOOT_HARD_RESET};

// Phase Modes
enum PhaseMode {SINGLE_PHASE,THREE_PHASE};

class Config {
public:
  Config();

  String getDeviceID() {return WiFi.macAddress();}
  String getIPAddress() {if (WiFi.status() == WL_CONNECTED) return WiFi.localIP().toString(); return "N/A";}
  int getRSSI() {if (WiFi.status() == WL_CONNECTED) return WiFi.RSSI(); return 0;}
  bool WiFiConnected() {return WiFi.status() == WL_CONNECTED;}
};

extern Config conf;
#endif // CONFIG_H