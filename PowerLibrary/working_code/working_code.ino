// Example code on how to use our file
#include "powerMonitor.h"

#define DELAY 1000  // Wait 1000 ms between power samples
PowerMonitor powerMonitor;

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("Initializing INA260 Sensors...");
    if (!powerMonitor.initSensors()) {
        Serial.println("One or more INA260 sensors not found. Check wiring!");
        while (1);
    }
    Serial.println("All INA260 sensors detected!");
}

void loop() {
    Serial.println("Solar Panel Sensor Readings:");
    // Pass enum values (solarSensorEnum and battSensorEnum) to the read functions
    Serial.print("Current: "); Serial.print(powerMonitor.readCurrent(solarSensorEnum)); Serial.println(" mA");
    Serial.print("Voltage: "); Serial.print(powerMonitor.readVoltage(solarSensorEnum)); Serial.println(" mV");
    Serial.print("Power: "); Serial.print(powerMonitor.readPower(solarSensorEnum)); Serial.println(" mW");
    Serial.println();

    Serial.println("Battery Sensor Readings:");
    // Pass enum values (solarSensorEnum and battSensorEnum) to the read functions
    Serial.print("Current: "); Serial.print(powerMonitor.readCurrent(battSensorEnum)); Serial.println(" mA");
    Serial.print("Voltage: "); Serial.print(powerMonitor.readVoltage(battSensorEnum)); Serial.println(" mV");
    Serial.print("Power: "); Serial.print(powerMonitor.readPower(battSensorEnum)); Serial.println(" mW");
    Serial.println();

    delay(DELAY);  // Wait 1 second before next reading
}
