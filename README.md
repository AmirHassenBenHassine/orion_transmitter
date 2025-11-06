# Orion Energy Monitor

## 💻 Source Code Files

### Core System
- **Config.h** / **Config.cpp**
  - System configuration
  - Pin definitions
  - Timing constants
  - Calibration values

- **main_v2.ino**
  - Main program
  - Setup and loop functions
  - Task coordination

### Hardware Management
- **HardwareManager.h** / **HardwareManager.cpp**
  - LED control
  - Button handling
  - Battery monitoring
  - Boot mode detection

- **LEDConf.h** / **LEDConf.cpp**
  - LED FreeRTOS tasks
  - Power-optimized task scheduling
  - Task handle management

### Communications
- **Communications.h** / **Communications.cpp**
  - WiFi management
  - MQTT client
  - Network configuration
  - Phase configuration

### Sensors & Monitoring
- **EnergyMonitor.h** / **EnergyMonitor.cpp**
  - Three-phase energy monitoring
  - Sensor reading
  - Power calculations
  - Energy accumulation

### Utilities
- **DataLogger.h** / **DataLogger.cpp**
  - System logging
  - Metrics display
  - Transmission status

- **Utils.h** / **Utils.cpp**
  - NTP time synchronization
  - OTA updates
  - Utility functions

---

## 📅 Version Information

**Optimization Version:** 2.0
**Date:** November 2025
**Target Platform:** ESP32
**Framework:** Arduino / ESP-IDF

**Optimized Modules:**
- All core system files
- All hardware management
- All communication modules
- All monitoring and logging
- Main program structure

---

