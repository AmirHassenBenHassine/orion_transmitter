#include "LEDConf.h"

void taskPowerLED(void* pvParameters) {
  pinMode(POWER_LED_PIN, OUTPUT);
  digitalWrite(POWER_LED_PIN, HIGH);
  
  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void taskWiFiLED(void* pvParameters) {
  pinMode(WIFI_LED_PIN, OUTPUT);
  
  while (true) {
    if (comms.isAPMode()) {
      // Blink in AP mode
      digitalWrite(WIFI_LED_PIN, HIGH);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      digitalWrite(WIFI_LED_PIN, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else if (comms.isWiFiConnected()) {
      // Solid when connected
      digitalWrite(WIFI_LED_PIN, HIGH);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else {
      // Off when disconnected
      digitalWrite(WIFI_LED_PIN, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void taskChargingLED(void* pvParameters) {
  pinMode(CHARGING_LED_PIN, OUTPUT);
  pinMode(CHARGING_STATUS_PIN, INPUT);
  
  while (true) {
    hardware.updateChargingLED();
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void taskBatteryLED(void* pvParameters) {
  pinMode(BATTERY_LED_PIN, OUTPUT);
  
  while (true) {
    hardware.updateBatteryStatus();
    
    float batteryPct = hardware.getBatteryPercentage();
    
    if (batteryPct > 20) {
      digitalWrite(BATTERY_LED_PIN, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else if (batteryPct > 10) {
      // Slow blink
      hardware.updateBatteryLED();
      vTaskDelay(500 / portTICK_PERIOD_MS);
    } else {
      // Fast blink
      hardware.updateBatteryLED();
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}

void createLEDTasks() {
  xTaskCreatePinnedToCore(taskPowerLED, "PowerLED", 1536, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskWiFiLED, "WiFiLED", 1536, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(taskChargingLED, "ChargingLED", 1536, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskBatteryLED, "BatteryLED", 1536, NULL, 1, NULL, 1);
}