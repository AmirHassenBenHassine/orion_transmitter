#include "Config.h"
#include "HardwareManager.h"
#include "EnergyMonitor.h"
#include "Communications.h"
#include "DataLogger.h"
#include "Utils.h"
#include "LEDConf.h"
#include <WebServer.h>
// Global Variables
WebServer server(80);
ThreePhaseEnergyMetrics metrics;
bool tasksCompleted = false;
unsigned long lastDataSendTime = 0;
unsigned long lastSensorTime = 0;
unsigned long lastMqttCheck = 0;

// RTC Data
RTC_DATA_ATTR unsigned long savedRunTime = 0;
RTC_DATA_ATTR unsigned long bootCount = 0;

// Function Prototypes
void handleWakeUpTasks();
void handleTrigger();

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // DEBUG_PRINTLN(F("\n\n========================================"));
  DEBUG_PRINTLN(F("=======ORION ENERGY MONITOR======="));
  // DEBUG_PRINTLN(F("========================================\n"));
  
  // Initialize hardware
  analogReadResolution(12);
  hardware.begin();
  
  // Detect boot mode
  BootMode bootMode = hardware.detectBootMode();
  bootCount++;
  
  logger.printBootInfo(bootMode, bootCount);
  
  // Create LED tasks
  createLEDTasks();
  
  // Handle boot modes
  switch (bootMode) {
    case BOOT_HARD_RESET:
      hardware.performHardReset();
      break;
      
    case BOOT_WIFI_PAIR:
      comms.setupWiFiProtocol(true);
      break;
      
    case BOOT_NORMAL:
    default:
      comms.setupWiFiProtocol(false);
      break;
  }
  
  // Initialize modules
  energyMonitor.begin();
  comms.loadPhaseConfiguration();
  
  // Configure NTP if connected
  if (WiFi.status() == WL_CONNECTED) {
    utils.configureNTP();
  }
  
  // Connect to MQTT
  comms.connectToMQTT();
  
  // Check for OTA updates
  if (bootCount == 1 || bootCount % 96 == 0) {
    DEBUG_PRINTLN(F("🔄 Checking for updates..."));
    utils.performOTA();
  }
  
  // Setup BLE
  // comms.setupBLE();
  
  // Print system info
  logger.printSystemInfo();
  
  // Initialize timing
  lastSensorTime = millis();
  lastMqttCheck = millis();
  lastDataSendTime = 0; // Force first reading
  
  // Run initial tasks
  handleWakeUpTasks();
}

void loop() {
  unsigned long now = millis();
  
  comms.mqttLoop();
  server.handleClient();
  
  // Check for sensor reading interval
  if (now - lastSensorTime >= DATA_SEND_INTERVAL) {
    handleWakeUpTasks();
    lastSensorTime = now;
  }
  
  // Check for MQTT triggers
  if (now - lastMqttCheck >= TRIGGER_CHECK_INTERVAL) {
    DEBUG_PRINTLN(F("\n--- Trigger Check ---"));
    comms.mqttLoop();
    
    if (comms.hasTrigger()) {
      DEBUG_PRINTLN(F("✓ Trigger detected"));
      handleTrigger();
      comms.clearTrigger();
      delay(2000);
    }
    
    lastMqttCheck = now;
  }
  
  delay(100);
  
  // Enter light sleep
  DEBUG_PRINTLN(F("💤 Entering light sleep (15 min)..."));
  esp_sleep_enable_timer_wakeup(WAKEUP_TIME_US);
  esp_light_sleep_start();
  
  delay(10);
}

void handleWakeUpTasks() {
  static bool waitingMessagePrinted = false;
  static unsigned long lastDotTime = 0;
  
  if (millis() - lastDataSendTime >= DATA_SEND_INTERVAL) {
    lastDataSendTime = millis();
    
    DEBUG_PRINTLN(F("\n========================================"));
    DEBUG_PRINTLN(F("   READING SENSORS"));
    DEBUG_PRINTLN(F("========================================"));
    
    // Update battery status
    hardware.updateBatteryStatus();
    
    // Read sensors
    energyMonitor.readSensors(metrics);
    metrics.timestamp = utils.getFormattedTimestamp();
    
    // Send data
    comms.publishEnergyData(metrics);
    logger.printEnergyMetrics(metrics);
    logger.logTransmissionStatus(true);
    
    tasksCompleted = true;
    waitingMessagePrinted = false;
  } else {
    if (!waitingMessagePrinted) {
      unsigned long remaining = (DATA_SEND_INTERVAL - (millis() - lastDataSendTime)) / 1000;
      DEBUG_PRINTF("⏰ Next reading in %lu seconds\n", remaining);
      waitingMessagePrinted = true;
      lastDotTime = millis();
    }
    
    if (millis() - lastDotTime >= 500) {
      DEBUG_PRINT(".");
      lastDotTime = millis();
    }
  }
}

void handleTrigger() {
  String action = comms.getTriggerAction();
  
  if (action == "reset") {
    DEBUG_PRINTLN(F("🔄 Trigger: RESET WiFi"));
    
    Preferences preferences;
    preferences.begin("wifiCreds", false);
    preferences.clear();
    preferences.end();
    
    comms.setupWiFiProtocol(true);
  } 
  else if (action == "config") {
    DEBUG_PRINTLN(F("⚙️ Trigger: CONFIG WiFi"));
    comms.setupWiFiProtocol(true);
  }
}