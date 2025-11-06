#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include <Arduino.h>
#include "Config.h"

class HardwareManager {
public:
  HardwareManager();
  
  // Initialization
  void begin();
  void setupLEDs();
  void setupButtons();
  
  // Boot Mode Detection
  BootMode detectBootMode();
  
  // Battery Management
  float readBatteryVoltage();
  float calculateBatteryPercentage(float voltage);
  float getBatteryPercentage() { return batteryPercentage; }
  bool isDeviceCharging() { return isCharging; }
  void updateBatteryStatus();
  
  // LED Control (called by FreeRTOS tasks)
  void updateWiFiLED(bool apMode, bool connected);
  void updateChargingLED();
  void updateBatteryLED();
  void blinkAllLEDs(int times, int delayMs);
  
  // Hard Reset
  void performHardReset();
  
private:
  float batteryPercentage;
  bool isCharging;
  bool batteryLedState;
};

extern HardwareManager hardware;

#endif // HARDWARE_MANAGER_H