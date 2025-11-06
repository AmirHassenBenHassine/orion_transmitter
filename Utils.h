#ifndef UTILS_H
#define UTILS_H

// #include <Arduino.h>
#include <time.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include "Config.h"

class Utils {
public:
  Utils();
  
  // NTP Functions
  void configureNTP();
  String getFormattedTimestamp();
  
  // OTA Functions
  void performOTA();
  bool checkForUpdate();
  
private:
  Preferences preferences;
  unsigned long lastOTACheck;
};

extern Utils utils;

#endif // UTILS_H