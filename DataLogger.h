#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>
#include "EnergyMonitor.h"

class DataLogger {
public:
  DataLogger();
  
  void printEnergyMetrics(const ThreePhaseEnergyMetrics& metrics);
  void logTransmissionStatus(bool success);
  void printSystemInfo();
  void printBootInfo(BootMode mode, unsigned long bootCount);
  
private:
  int dataTransmissionCount;
  int dataTransmissionFailures;
};

extern DataLogger logger;

#endif // DATA_LOGGER_H