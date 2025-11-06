// #include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "Communications.h"

CommunicationsManager comms;

CommunicationsManager::CommunicationsManager() 
  : client(espClient), 
    // pServer(nullptr), 
    // pCharacteristic(nullptr),
    // isBLEEnabled(false),
    hasNewCredentials(false),
    isConnectedToMQTT(false),
    wifiConnected(false),
    wifiAPMode(false),
    hasTriggerAction(false) {
}

bool CommunicationsManager::connectToWiFi(const char* ssid, const char* password) {
  WiFi.disconnect(true);
  delay(1000);
  WiFi.begin(ssid, password);
  
  DEBUG_PRINT(F("🔌 Connecting to WiFi"));
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    DEBUG_PRINT(F("."));
    attempts++;
    
    if (attempts % 10 == 0) {
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(ssid, password);
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTLN(F("\n✅ Connected!"));
    DEBUG_PRINT(F("IP: "));
    DEBUG_PRINTLN(WiFi.localIP());
    return true;
  }
  
  DEBUG_PRINTLN(F("\n❌ Failed"));
  return false;
}

bool CommunicationsManager::connectToSavedWiFi() {
  preferences.begin("wifiCreds", true);
  int networksCount = preferences.getInt("count", 0);
  
  for (int i = 0; i < networksCount; i++) {
    String ssid = preferences.getString(("ssid" + String(i)).c_str(), "");
    String password = preferences.getString(("pass" + String(i)).c_str(), "");
    
    if (ssid.length() > 0 && connectToWiFi(ssid.c_str(), password.c_str())) {
      preferences.end();
      return true;
    }
  }
  
  preferences.end();
  return false;
}

void CommunicationsManager::saveWiFiCredentials(const char* ssid, const char* password) {
  preferences.begin("wifiCreds", false);
  int networksCount = preferences.getInt("count", 0);
  
  // Check if exists
  for (int i = 0; i < networksCount; i++) {
    String savedSSID = preferences.getString(("ssid" + String(i)).c_str(), "");
    if (savedSSID == ssid) {
      DEBUG_PRINTLN(F("ℹ️ Credentials exist"));
      preferences.end();
      return;
    }
  }
  
  preferences.putString(("ssid" + String(networksCount)).c_str(), ssid);
  preferences.putString(("pass" + String(networksCount)).c_str(), password);
  preferences.putInt("count", networksCount + 1);
  preferences.end();
  
  DEBUG_PRINTLN(F("✅ Credentials saved!"));
}

void CommunicationsManager::connectToMQTT() {
  client.setServer(LOCAL_BROKER_HOST, MQTT_PORT);
  client.setCallback(mqttCallback);
  client.setBufferSize(2048);
  
  int attempts = 0;
  while (!client.connected() && attempts < 10) {
    DEBUG_PRINT(F("🔌 MQTT connecting..."));
    if (client.connect("ESP32Client")) {
      DEBUG_PRINTLN(F(" ✅ Connected!"));
      isConnectedToMQTT = true;
      
      client.subscribe(TOPIC_WIFI_REFRESH, 1);
      client.subscribe(TOPIC_WIFI_CONFIG, 1);
      client.subscribe(TOPIC_TRIGGER, 1);
      client.subscribe(TOPIC_ESP32_PHASE, 1);
      
      DEBUG_PRINTLN(F("📡 Subscribed to all topics"));
      publishESP32Status();
    } else {
      DEBUG_PRINT(F(" ❌ Failed: "));
      DEBUG_PRINTLN(client.state());
      attempts++;
      delay(2000);
    }
  }
}

void CommunicationsManager::publishEnergyData(const ThreePhaseEnergyMetrics &metrics) {
  String payload = metrics.toTransmissionFormat();
  
  if (WiFi.status() == WL_CONNECTED && client.connected()) {
    if (client.publish(TOPIC_ENERGY_METRICS, payload.c_str())) {
      DEBUG_PRINTLN(F("✅ MQTT published"));
    }
  }
  //  else if (isBLEConnected()) {
  //   sendViaBLE(payload);
  // }
}

void CommunicationsManager::publishESP32Status() {
  if (!client.connected()) return;
  
  StaticJsonDocument<512> doc;
  doc["deviceId"] = WiFi.macAddress();
  doc["status"] = "ok";
  doc["phase"] = energyMonitor.getPhaseString();
  doc["phaseMode"] = (energyMonitor.getPhaseMode() == SINGLE_PHASE) ? "single" : "three";
  
  if (WiFi.status() == WL_CONNECTED) {
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
  }
  
  doc["uptime"] = millis() / 1000;
  
  String statusJson;
  serializeJson(doc, statusJson);
  
  if (client.publish(TOPIC_ESP32_STATUS, statusJson.c_str(), true)) {
    DEBUG_PRINTLN(F("✅ Status published"));
  }
}

void CommunicationsManager::scanAndSendWiFiNetworks() {
  DEBUG_PRINTLN(F("🔍 Scanning WiFi..."));
  int n = WiFi.scanNetworks();
  
  StaticJsonDocument<512> jsonDoc;
  JsonArray networks = jsonDoc.to<JsonArray>();
  
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      JsonObject network = networks.createNestedObject();
      network["ssid"] = WiFi.SSID(i);
      network["rssi"] = WiFi.RSSI(i);
      network["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }
  }
  
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);
  client.publish(TOPIC_WIFI_SCAN, jsonResponse.c_str());
  DEBUG_PRINTLN(F("✅ Scan results published"));
}

void CommunicationsManager::handlePhaseConfiguration(String payload) {  
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, payload)) {
    DEBUG_PRINTLN(F("❌ JSON parse error"));
    return; }
  String phaseStr = doc["phase"].as<String>();
  
  if (phaseStr == "single") {
    energyMonitor.setPhaseMode(SINGLE_PHASE);
    preferences.begin("phaseConfig", false);
    preferences.putString("mode", "single");
    preferences.end();
    DEBUG_PRINTLN(F("✅ Phase mode: SINGLE"));

  } else if (phaseStr == "three") {
    energyMonitor.setPhaseMode(THREE_PHASE);
    preferences.begin("phaseConfig", false);
    preferences.putString("mode", "three");
    preferences.end();
    DEBUG_PRINTLN(F("✅ Phase mode: THREE"));
  }
  // Debounced status publish (wait for MQTT to stabilize)
  xTaskCreatePinnedToCore([](void*){
    delay(2000); 
    comms.publishESP32Status();
    vTaskDelete(nullptr);
  }, "delayedPublish", 2048, nullptr, 1, nullptr, 1);
}

void CommunicationsManager::loadPhaseConfiguration() {
  preferences.begin("phaseConfig", true);
  String savedMode = preferences.getString("mode", "three");
  preferences.end();
  
  DEBUG_PRINT("📂 Loading phase: ");
  DEBUG_PRINTLN(savedMode);
  
  energyMonitor.setPhaseMode(savedMode == "single" ? SINGLE_PHASE : THREE_PHASE);
}

// void CommunicationsManager::setupBLE() {
//   NimBLEDevice::init("ESP32-Orion");
//   pServer = NimBLEDevice::createServer();
//   NimBLEService* pService = pServer->createService(SERVICE_UUID);
//   pCharacteristic = pService->createCharacteristic(
//     CHARACTERISTIC_UUID,
//     NIMBLE_PROPERTY::NOTIFY
//   );
//   pService->start();
//   NimBLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
//   NimBLEDevice::getAdvertising()->start();
//   isBLEEnabled = true;
//   DEBUG_PRINTLN(F("✅ BLE initialized"));
// }

// void CommunicationsManager::sendViaBLE(const String& data) {
//   if (isBLEConnected()) {
//     pCharacteristic->setValue(data.c_str());
//     pCharacteristic->notify();
//     DEBUG_PRINTLN(F("✅ BLE transmitted"));
//   }
// }

void CommunicationsManager::handleNewWiFiCredentials() {
  if (!hasNewCredentials) return;
  
  publishConfirmation(newSSID);
  delay(1000);
  
  DEBUG_PRINT(F("🔌 Connecting to: "));
  DEBUG_PRINTLN(newSSID);
  WiFi.begin(newSSID.c_str(), newPassword.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    DEBUG_PRINT(F("."));
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTLN(F("\n✅ Connected!"));
    saveWiFiCredentials(newSSID.c_str(), newPassword.c_str());
    wifiAPMode = false;
    wifiConnected = true;
  }
  
  hasNewCredentials = false;
}

void CommunicationsManager::publishConfirmation(const String& ssid) {
  String confirmMessage = "{\"status\": \"success\"}";
  client.publish(TOPIC_WIFI_CONFIRM, confirmMessage.c_str(), true);
  DEBUG_PRINTLN(F("✅ Confirmation published"));
}

void CommunicationsManager::publishWiFiStatus(String mode) {
  String ip = (mode == "AP") ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
  String statusMsg = "{\"mode\":\"" + mode + "\", \"ip\":\"" + ip + "\"}";
  client.publish(TOPIC_WIFI_STATUS, statusMsg.c_str(), true);
}

void CommunicationsManager::setupWiFiProtocol(bool forceAP) { 
  if (!forceAP && connectToSavedWiFi()) {
    DEBUG_PRINTLN(F("✅ Connected to saved network"));
    WiFi.softAPdisconnect(true);
    utils.configureNTP();

    wifiAPMode = false;     
    wifiConnected = true;     

    if (!MDNS.begin(ESP_HOSTNAME)) {
      DEBUG_PRINTLN(F("❌ mDNS setup failed"));
    } else {
      DEBUG_PRINTLN(F("✅ mDNS started"));
    }
    return;  // Done, connected to WiFi
  }

  wifiAPMode = true;          
  wifiConnected = false;  
  // Otherwise → start AP mode
  DEBUG_PRINTLN(F("📡 Starting AP mode..."));
  WiFi.softAP(AP_SSID, AP_PASSWORD, 6, 0);
  WiFi.mode(WIFI_AP_STA);     

  unsigned long startMillis = millis();
  const unsigned long maxWaitTime = 900000; // 15 minutes

  while (WiFi.status() != WL_CONNECTED && millis() - startMillis < maxWaitTime) {
    connectToMQTT();
    client.loop();
    handleNewWiFiCredentials();
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTLN(F("✅ WiFi configured! Restarting..."));
    wifiAPMode = false;     
    wifiConnected = true;  
    publishWiFiStatus("STA");
    delay(2000);
    ESP.restart();
  } else {
    DEBUG_PRINTLN(F("⏰ Timeout, continuing in AP mode"));
    publishWiFiStatus("AP");
    delay(2000);
    ESP.restart();
  }
}

void CommunicationsManager::mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (length == 0) return;
  
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  DEBUG_PRINTF("📩 MQTT [%s]: %s\n", topic, message.c_str());
  
  String topicStr = String(topic);
  
  // Handle Phase Configuration
  if (topicStr == TOPIC_ESP32_PHASE) {
    comms.handlePhaseConfiguration(message);
  }
  // Handle Trigger
  else if (topicStr == TOPIC_TRIGGER) {
    DynamicJsonDocument doc(128);
    if (deserializeJson(doc, message) == DeserializationError::Ok) {
      comms.triggerAction = doc["action"].as<String>();
      comms.hasTriggerAction = true;
      comms.client.publish(TOPIC_TRIGGER, "", true);
    }
  }
  // Handle WiFi Scan
  else if (topicStr == TOPIC_WIFI_REFRESH) {
    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, message) == DeserializationError::Ok && doc["action"] == "scan") {
      comms.scanAndSendWiFiNetworks();
      comms.client.publish(TOPIC_WIFI_REFRESH, "", true);
    }
  }
  // Handle WiFi Config
  else if (topicStr == TOPIC_WIFI_CONFIG) {
    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, message) == DeserializationError::Ok) {
      comms.newSSID = doc["ssid"].as<String>();
      comms.newPassword = doc["password"].as<String>();
      comms.hasNewCredentials = true;
    }
  }
}