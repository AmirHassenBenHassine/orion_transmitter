#include "Utils.h"

Utils utils;

Utils::Utils() : lastOTACheck(0) {
}

void Utils::configureNTP() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  
  DEBUG_PRINT(F("⏰ Syncing time"));
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 1000000000 && attempts < 10) {
    delay(500);
    DEBUG_PRINT(F("."));
    now = time(nullptr);
    attempts++;
  }
  
  if (now > 1000000000) {
    DEBUG_PRINTLN(F(" ✅ Synchronized"));
  } else {
    DEBUG_PRINTLN(F(" ❌ Failed"));
  }
}

String Utils::getFormattedTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time Not Available";
  }
  
  char buffer[80];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

bool Utils::checkForUpdate() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  const char* headers[] = {"ETag", "Last-Modified"};
  http.collectHeaders(headers, 2);
  http.begin(client, FIRMWARE_URL);
  
  int httpCode = http.sendRequest("HEAD");
  
  if (httpCode == HTTP_CODE_OK) {
    preferences.begin("ota", true);
    String currentETag = preferences.getString("fw_etag", "");
    preferences.end();
    
    String newETag = http.header("ETag");
    
    DEBUG_PRINTF("ETag: Current=%s, New=%s\n", 
                  currentETag.c_str(), newETag.c_str());
    
    if (currentETag.equals(newETag)) {
      DEBUG_PRINTLN(F("✅ Firmware up to date"));
      return false;
    }
    
    preferences.begin("ota", false);
    preferences.putString("fw_etag", newETag);
    preferences.end();
    
    return true;
  }
  
  DEBUG_PRINTF("❌ Update check failed: %d\n", httpCode);
  return false;
}

void Utils::performOTA() {
  if (!checkForUpdate()) {
    return;
  }
  
  DEBUG_PRINTLN(F("========================================"));
  DEBUG_PRINTLN(F("   FIRMWARE UPDATE AVAILABLE"));
  DEBUG_PRINTLN(F("========================================"));
  
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  http.begin(client, FIRMWARE_URL);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    
    if (contentLength <= 0) {
      DEBUG_PRINTLN(F("❌ Invalid content length"));
      return;
    }
    
    if (!Update.begin(contentLength)) {
      DEBUG_PRINTLN(F("❌ Not enough space"));
      return;
    }
    
    size_t written = Update.writeStream(*http.getStreamPtr());
    
    if (written != contentLength) {
      DEBUG_PRINTF("❌ Download incomplete: %d/%d\n", written, contentLength);
      return;
    }
    
    if (!Update.end()) {
      DEBUG_PRINTLN(F("❌ Update failed"));
      return;
    }
    
    if (Update.isFinished()) {
      DEBUG_PRINTLN(F("✅ Update successful! Rebooting..."));
      delay(1000);
      ESP.restart();
    }
    
    if (Update.hasError()) {
      DEBUG_PRINTLN(F("❌ Update error"));
      Update.printError(Serial);
      return;
    }
  } else {
    DEBUG_PRINTF("❌ HTTP error: %d\n", httpCode);
  }
  
  http.end();
}