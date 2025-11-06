#include "HardwareManager.h"
#include <Preferences.h>

HardwareManager hardware;

HardwareManager::HardwareManager() 
  : batteryPercentage(0.0), isCharging(false), batteryLedState(false) {
}

void HardwareManager::begin() {
  setupLEDs();
  setupButtons();
}

void HardwareManager::setupLEDs() {
  pinMode(POWER_LED_PIN, OUTPUT);
  pinMode(WIFI_LED_PIN, OUTPUT);
  pinMode(CHARGING_LED_PIN, OUTPUT);
  pinMode(BATTERY_LED_PIN, OUTPUT);
  
  // Startup LED sequence
  digitalWrite(POWER_LED_PIN, HIGH);
  delay(500);
  digitalWrite(WIFI_LED_PIN, HIGH);
  delay(500);
  digitalWrite(CHARGING_LED_PIN, HIGH);
  delay(500);
  digitalWrite(BATTERY_LED_PIN, HIGH);
  delay(500);
  
  // Turn off non-power LEDs
  digitalWrite(BATTERY_LED_PIN, LOW);
  delay(500);
  digitalWrite(CHARGING_LED_PIN, LOW);
  delay(500);
  digitalWrite(WIFI_LED_PIN, LOW);
  delay(500);
}

void HardwareManager::setupButtons() {
  pinMode(WIFI_PAIRING_BTN, INPUT_PULLDOWN);
  pinMode(HARD_RESET_BTN, INPUT_PULLDOWN);
  pinMode(CHARGING_STATUS_PIN, INPUT);
  
  // Configure wake-up sources
  esp_sleep_enable_ext0_wakeup((gpio_num_t)HARD_RESET_BTN, 1);
  esp_sleep_enable_ext1_wakeup((1ULL << WIFI_PAIRING_BTN), ESP_EXT1_WAKEUP_ANY_HIGH);
}

BootMode HardwareManager::detectBootMode() {
  esp_sleep_wakeup_cause_t wake = esp_sleep_get_wakeup_cause();
  
  // Debounce
  delay(100);
  bool wifiPressed = digitalRead(WIFI_PAIRING_BTN) == HIGH;
  bool resetPressed = digitalRead(HARD_RESET_BTN) == HIGH;
  
  if (wake == ESP_SLEEP_WAKEUP_EXT0)
    return BOOT_HARD_RESET;
  else if (wake == ESP_SLEEP_WAKEUP_EXT1)
    return BOOT_WIFI_PAIR;
  else if (resetPressed)
    return BOOT_HARD_RESET;
  else if (wifiPressed)
    return BOOT_WIFI_PAIR;
  else
    return BOOT_NORMAL;
}

float HardwareManager::readBatteryVoltage() {
  float sum = 0;
  for (int i = 0; i < BATTERY_SAMPLES; i++) {
    sum += analogReadMilliVolts(BATTERY_VOLTAGE_PIN);
    delay(5);
  }
  float avgADC = sum / BATTERY_SAMPLES;
  return (avgADC / 1000.0) * VOLTAGE_DIVIDER_RATIO;
}

float HardwareManager::calculateBatteryPercentage(float voltage) {
  if (voltage >= BATTERY_MAX_VOLTAGE) return 100.0;
  if (voltage <= BATTERY_MIN_VOLTAGE) return 0.0;
  
  float percentage = ((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100.0;
  return constrain(percentage, 0.0, 100.0);
}

void HardwareManager::updateBatteryStatus() {
  float voltage = readBatteryVoltage();
  batteryPercentage = calculateBatteryPercentage(voltage);
  isCharging = digitalRead(CHARGING_STATUS_PIN) == HIGH;
}

void HardwareManager::updateWiFiLED(bool apMode, bool connected) {
  if (apMode) {
    // Slow blink in AP mode
    digitalWrite(WIFI_LED_PIN, millis() % 2000 < 1000 ? HIGH : LOW);
  } else if (connected) {
    // Solid on when connected
    digitalWrite(WIFI_LED_PIN, HIGH);
  } else {
    // Off when disconnected
    digitalWrite(WIFI_LED_PIN, LOW);
  }
}

void HardwareManager::updateChargingLED() {
  isCharging = digitalRead(CHARGING_STATUS_PIN) == HIGH;
  digitalWrite(CHARGING_LED_PIN, isCharging ? HIGH : LOW);
}

void HardwareManager::updateBatteryLED() {
  if (batteryPercentage > 20) {
    digitalWrite(BATTERY_LED_PIN, LOW);
  } else if (batteryPercentage > 10) {
    digitalWrite(BATTERY_LED_PIN, batteryLedState ? HIGH : LOW);
    batteryLedState = !batteryLedState;
  } else {
    // Fast blink below 10%
    digitalWrite(BATTERY_LED_PIN, batteryLedState ? HIGH : LOW);
    batteryLedState = !batteryLedState;
  }
}

void HardwareManager::blinkAllLEDs(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(WIFI_LED_PIN, HIGH);
    digitalWrite(CHARGING_LED_PIN, HIGH);
    digitalWrite(BATTERY_LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(WIFI_LED_PIN, LOW);
    digitalWrite(CHARGING_LED_PIN, LOW);
    digitalWrite(BATTERY_LED_PIN, LOW);
    delay(delayMs);
  }
}

void HardwareManager::performHardReset() {
  // DEBUG_PRINTLN(F("========================================"));
  DEBUG_PRINTLN(F("   PERFORMING HARD RESET"));
  // DEBUG_PRINTLN(F("========================================"));
  
  blinkAllLEDs(10, 100);
  
  // Clear all preferences
  Preferences preferences;
  preferences.begin("wifiCreds", false);
  preferences.clear();
  preferences.end();
  
  preferences.begin("phaseConfig", false);
  preferences.clear();
  preferences.end();
  
  preferences.begin("ota", false);
  preferences.clear();
  preferences.end();
  
  DEBUG_PRINTLN(F("✅ All data erased. Restarting..."));
  delay(1000);
  ESP.restart();
}