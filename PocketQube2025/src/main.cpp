#define DEBUG true

#if DEBUG
  #pragma message "Compiling in debug mode"
#endif

#include <Arduino.h>
#include <SD.h>
#include "pinDefinitions.h"
#include "camera.h"
#include "dataLogger.h"
#include "sensors.h"
#include "powerMonitor.h"

CameraManager camera;

DataPersistance datalogger;

MMCModule magneticSensor;

LMSModule gyroAccelSensor;

MS5611Module atmosphericSensor;

PowerMonitor powerMonitor;

int lastPhotoTaken = 0;

#define CAMERA_REFRESH_DELAY 10000
unsigned long lastCameraRefresh = 0;

#define CSV_UPATE_DELAY 10000
unsigned long lastCSVUpdate = 0;

#define SENSOR_REFRESH_DELAY 1000
unsigned long lastSensorRefresh = 0;

void updateSensors();
void updateCamera();

void setup() {

  #if DEBUG
    Serial.begin(9600);
    Serial.println("\n\nStarting QubeSat");
  #else
    noInterrupts();
  #endif

  //Initialize SD Card
  while(!SD.begin(SD_CS)){
    Serial.println("SD Card Error!");
    delay(1000);
  }

  datalogger.init();

  magneticSensor.init_mag();

  gyroAccelSensor.init_LSM6DOX();

  atmosphericSensor.init_MS5611();

  powerMonitor.initSensors();

  camera.setup(CAM_CS);

  #if DEBUG
    Serial.println("\nQubeSat Initialized\n\n");
  #endif

  interrupts();
}

void loop() {

  

  if(millis() - lastSensorRefresh >= SENSOR_REFRESH_DELAY || !lastSensorRefresh || millis() < lastSensorRefresh) {
    #if DEBUG
      Serial.println("Refreshing Sensors");
    #endif
    lastSensorRefresh = millis();
    updateSensors();
  }

  if(millis() - lastCameraRefresh >= CAMERA_REFRESH_DELAY || millis() < lastCameraRefresh) {
    #if DEBUG
      Serial.println("Capturing Photo");
    #endif
    lastCameraRefresh = millis();
    lastPhotoTaken = camera.takePicture();
  }

  if(millis() - lastCSVUpdate >= CSV_UPATE_DELAY || millis() < lastCSVUpdate){
    #if DEBUG
      Serial.println("Updating CSV");
    #endif
    lastCSVUpdate = millis();
    datalogger.addToCSV(lastPhotoTaken);
    if(lastPhotoTaken)
      lastPhotoTaken = 0;
  }

  interrupts();
}

void updateSensors(){
  static double dataIn[NUM_ITEMS_IN_CSV];
  
  datalogger.addData(dataIn);
}