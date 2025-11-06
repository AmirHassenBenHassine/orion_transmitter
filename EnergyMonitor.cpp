#include "EnergyMonitor.h"
#include "HardwareManager.h"
EnergyMonitorManager energyMonitor;

String ThreePhaseEnergyMetrics::toTransmissionFormat() const {
  StaticJsonDocument<512> doc;
  doc["deviceId"] = conf.getDeviceID();
  doc["battery"] = battery;
  doc["timestamp"] = timestamp;
  doc["voltage"] = voltage;
  doc["totalPower"] = totalPower;
  doc["energyTotal"] = energyTotal;
  
  JsonArray phases = doc.createNestedArray("phases");
  for (int i = 0; i < MAX_PHASES; i++) {
    JsonObject phase = phases.createNestedObject();
    phase["current"] = current[i];
    phase["power"] = power[i];
  }
  String output;
  serializeJson(doc, output);
  return output;
}

EnergyMonitorManager::EnergyMonitorManager() 
  : currentPhaseMode(THREE_PHASE), phaseTypeString("three") {
}

void EnergyMonitorManager::begin() {
  for (int i = 0; i < MAX_PHASES; i++) {
    emon[i].current(currentPins[i], currentCal[i]);
  }
  
  emonVoltage.voltage(VOLTAGE_PIN, voltageCal, 1.7);
  
  DEBUG_PRINTLN(F("✅ Energy Monitor initialized"));
}

void EnergyMonitorManager::setPhaseMode(PhaseMode mode) {
  currentPhaseMode = mode;
  phaseTypeString = (mode == SINGLE_PHASE) ? "single" : "three";
  
  DEBUG_PRINTLN(F("========================================"));
  DEBUG_PRINT(F("⚙️ Phase Mode: "));
  DEBUG_PRINTLN(mode == SINGLE_PHASE ? "SINGLE-PHASE" : "3-PHASE");
  DEBUG_PRINTLN(F("========================================"));
}

int EnergyMonitorManager::readAndAverageADC(int pin, int numSamples) {
  long sum = 0;
  int validReadings = 0;
  
  for (int i = 0; i < numSamples; i++) {
    int reading = analogRead(pin);
    if (reading >= 0 && reading <= 4095) {
      sum += reading;
      validReadings++;
    }
    delayMicroseconds(500);
  }
  
  return validReadings > 0 ? sum / validReadings : 0;
}

void EnergyMonitorManager::readSensors(ThreePhaseEnergyMetrics &metrics) {
  static unsigned long lastUpdateTime = 0;
  static float accumulatedEnergy = 0;
  unsigned long currentTime = millis();
  
  // Read voltage
  emonVoltage.calcVI(20, 2000);
  metrics.voltage = emonVoltage.Vrms;
  
  // Read battery
  metrics.battery = hardware.getBatteryPercentage();
  
  // Determine phases to read
  int phasesToRead = (currentPhaseMode == SINGLE_PHASE) ? 1 : MAX_PHASES;
  metrics.totalPower = 0;
  
  for (int i = 0; i < phasesToRead; i++) {
    float current = emon[i].calcIrms(1480);
    
    if (current < NOISE_THRESHOLD) current = 0;
    
    metrics.current[i] = current;
    metrics.power[i] = metrics.voltage * current;
    metrics.totalPower += metrics.power[i];
  }
  
  // Zero unused phases
  if (currentPhaseMode == SINGLE_PHASE) {
    for (int i = 1; i < MAX_PHASES; i++) {
      metrics.current[i] = 0;
      metrics.power[i] = 0;
    }
  }
  
  // Calculate energy
  if (lastUpdateTime > 0) {
    float timeElapsedHours = (currentTime - lastUpdateTime) / 3600000.0;
    accumulatedEnergy += metrics.totalPower * timeElapsedHours;
  }
  lastUpdateTime = currentTime;
  
  metrics.energyTotal = accumulatedEnergy / 1000.0;
}