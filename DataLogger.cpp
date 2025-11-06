#include "DataLogger.h"
#include "Config.h"

DataLogger logger;

DataLogger::DataLogger() 
  : dataTransmissionCount(0), dataTransmissionFailures(0) {
}

void DataLogger::printEnergyMetrics(const ThreePhaseEnergyMetrics& metrics) {
  DEBUG_PRINTLN("\n========================================");
  DEBUG_PRINTLN("   ENERGY MONITORING DATA");
  DEBUG_PRINTLN("========================================");
  DEBUG_PRINTF("Timestamp          : %s\n", metrics.timestamp.c_str());
  DEBUG_PRINTF("Battery            : %.2f%%\n", metrics.battery);
  DEBUG_PRINTF("Voltage            : %.2f V\n", metrics.voltage);
  DEBUG_PRINTF("Total Power        : %.2f W\n", metrics.totalPower);
  DEBUG_PRINTF("Energy Total       : %.3f kWh\n", metrics.energyTotal);
  
  int phasesToShow = (energyMonitor.getPhaseMode() == SINGLE_PHASE) ? 1 : MAX_PHASES;
  
  for (int i = 0; i < phasesToShow; i++) {
    DEBUG_PRINTF("\n  Phase %d:\n", i + 1);
    DEBUG_PRINTF("    Current        : %.2f A\n", metrics.current[i]);
    DEBUG_PRINTF("    Power          : %.2f W\n", metrics.power[i]);
  }
  DEBUG_PRINTLN("========================================\n");
}

void DataLogger::logTransmissionStatus(bool success) {
  dataTransmissionCount++;
  if (!success) {
    dataTransmissionFailures++;
    DEBUG_PRINTLN("❌ Transmission failed!");
  } else {
    DEBUG_PRINTLN("✅ Transmitted successfully");
  }
  
  if (dataTransmissionCount % 10 == 0) {
    float successRate = ((float)(dataTransmissionCount - dataTransmissionFailures) / dataTransmissionCount) * 100;
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("📊 Transmission Statistics:");
    DEBUG_PRINTF("  Total: %d | Failures: %d | Success: %.2f%%\n", 
                  dataTransmissionCount, dataTransmissionFailures, successRate);
    DEBUG_PRINTLN("========================================");
  }
}

void DataLogger::printSystemInfo() {
  DEBUG_PRINTLN(F("========================================"));
  DEBUG_PRINTLN(F("   SYSTEM INFORMATION"));
  DEBUG_PRINTLN(F("========================================"));
  DEBUG_PRINTF("  Device ID: %s\n", conf.getDeviceID().c_str());
  
  if (conf.WiFiConnected()) {
    DEBUG_PRINTF("  IP Address: %s\n", conf.getIPAddress().c_str());
    DEBUG_PRINTF("  RSSI: %d dBm\n", conf.getRSSI());
  }
  DEBUG_PRINTF("  Free Heap: %d bytes\n", ESP.getFreeHeap());
  DEBUG_PRINTF("  Uptime: %lu seconds\n", millis() / 1000);
  DEBUG_PRINTLN("========================================");
}

void DataLogger::printBootInfo(BootMode mode, unsigned long bootCount) {
  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINTLN(F("   BOOT INFORMATION"));
  DEBUG_PRINTLN(F("========================================"));
  DEBUG_PRINTF("  Boot Count: %lu\n", bootCount);
  DEBUG_PRINT(F("  Boot Mode: "));
  
  switch (mode) {
    case BOOT_NORMAL:
      DEBUG_PRINTLN(F("NORMAL"));
      break;
    case BOOT_WIFI_PAIR:
      DEBUG_PRINTLN(F("WIFI PAIRING"));
      break;
    case BOOT_HARD_RESET:
      DEBUG_PRINTLN(F("HARD RESET"));
      break;
  }
  
  DEBUG_PRINTLN(F("========================================\n"));
}