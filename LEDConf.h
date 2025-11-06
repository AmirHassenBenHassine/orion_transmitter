#ifndef LED_CONF_H
#define LED_CONF_H

#include <Arduino.h>
#include "Config.h"
#include "HardwareManager.h"
#include "Communications.h"

void taskPowerLED(void* pvParameters);
void taskWiFiLED(void* pvParameters);
void taskChargingLED(void* pvParameters);
void taskBatteryLED(void* pvParameters);
void createLEDTasks();

#endif // LED_CONF_H