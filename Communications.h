#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

#include <Arduino.h>
#include <PubSubClient.h>
// #include <NimBLEDevice.h>
#include <Preferences.h>
#include "Config.h"
#include "EnergyMonitor.h"
#include "Utils.h"

class CommunicationsManager {
public:
  CommunicationsManager();
  
  // WiFi Management
  bool connectToWiFi(const char* ssid, const char* password);
  bool connectToSavedWiFi();
  void saveWiFiCredentials(const char* ssid, const char* password);
  void setupWiFiProtocol(bool forceAP = false);
  void handleNewWiFiCredentials();
  bool isWiFiConnected() { return wifiConnected; }
  bool isAPMode() { return wifiAPMode; }

  // MQTT Management
  void connectToMQTT();
  void publishEnergyData(const ThreePhaseEnergyMetrics &metrics);
  void publishESP32Status();
  void publishConfirmation(const String& ssid);
  void publishWiFiStatus(String mode);
  void scanAndSendWiFiNetworks();
  void mqttLoop() { client.loop(); }
  bool isMQTTConnected() { return client.connected(); }
  
  // BLE Management
  // void setupBLE();
  // bool isBLEConnected() { return isBLEEnabled && pServer && pServer->getConnectedCount() > 0; }
  // void sendViaBLE(const String& data);
  
  // Phase Configuration
  void handlePhaseConfiguration(String payload);
  void loadPhaseConfiguration();
  
  // Trigger Management
  bool hasTrigger() { return hasTriggerAction; }
  String getTriggerAction() { return triggerAction; }
  void clearTrigger() { hasTriggerAction = false; triggerAction = ""; }
  
  bool hasNewWiFiCredentials() { return hasNewCredentials; }
  
  String newSSID;
  String newPassword;
  bool hasNewCredentials;
  
private:
  WiFiClient espClient;
  PubSubClient client;
  Preferences preferences;
  
  // NimBLEServer* pServer;
  // NimBLECharacteristic* pCharacteristic;
  // bool isBLEEnabled;
  
  bool isConnectedToMQTT;
  bool wifiConnected;
  bool wifiAPMode;
  
  bool hasTriggerAction;
  String triggerAction;
  
  static void mqttCallback(char* topic, byte* payload, unsigned int length);
};

extern CommunicationsManager comms;

#endif // COMMUNICATIONS_H