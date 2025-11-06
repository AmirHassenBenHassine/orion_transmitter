#include "Config.h"

float voltageCal = 321.17;
float currentCal[MAX_PHASES] = {8.7, 8.7, 8.7};
const int currentPins[MAX_PHASES] = {CURRENT_PIN_R, CURRENT_PIN_Y, CURRENT_PIN_B};


  String getDeviceID() {return WiFi.macAddress();}
  String getIPAddress() {if (WiFi.status() == WL_CONNECTED) return WiFi.localIP().toString(); return "N/A";}
  int getRSSI() {if (WiFi.status() == WL_CONNECTED) return WiFi.RSSI(); return 0;}
  bool WiFiConnected() {return WiFi.status() == WL_CONNECTED;}