#ifndef ENERGY_MONITOR_H
#define ENERGY_MONITOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <EmonLib.h>
#include "Config.h"

struct ThreePhaseEnergyMetrics {
  float voltage;
  float current[MAX_PHASES];
  float power[MAX_PHASES];
  float totalPower;
  float energyTotal;
  float battery;
  String timestamp;
  
  String toTransmissionFormat() const;
};

class EnergyMonitorManager {
public:
  EnergyMonitorManager();
  
  void begin();
  void readSensors(ThreePhaseEnergyMetrics &metrics);
  
  void setPhaseMode(PhaseMode mode);
  PhaseMode getPhaseMode() const { return currentPhaseMode; }
  String getPhaseString() const { return phaseTypeString; }
  
private:
  EnergyMonitor emon[MAX_PHASES];
  EnergyMonitor emonVoltage;
  PhaseMode currentPhaseMode;
  String phaseTypeString;
  
  int readAndAverageADC(int pin, int numSamples);
};

extern EnergyMonitorManager energyMonitor;

#endif // ENERGY_MONITOR_H