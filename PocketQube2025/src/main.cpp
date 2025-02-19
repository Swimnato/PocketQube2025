#include <Arduino.h>
#include <Adafruit_MMC56x3.h>
#include <LSM6DSOX.h>
#include <SD.h>
#include "pinDefinitions.h"
#include "camera.h"

#define DEBUG true

CameraManager camera;

#define CAMERA_REFRESH_DELAY 10000
unsigned long lastCameraRefresh = 0;


#define SENSOR_REFRESH_DELAY 1000
unsigned long lastSensorRefresh = 0;

void updateSensors();
void updateCamera();

void setup() {
  #if DEBUG
    Serial.begin(9600);
    Serial.println("Starting QubeSat");
  #endif

  //Initialize SD Card
  while(!SD.begin(SD_CS)){
    Serial.println(F("SD Card Error!"));delay(1000);
  }

  camera.setup(CAM_CS);

  #if DEBUG
    Serial.println("\n\nQubeSat Initialized\n\n");
  #endif
}

void loop() {
  if(millis() - lastSensorRefresh >= SENSOR_REFRESH_DELAY) {
    Serial.println("Refreshing Sensors");
    lastSensorRefresh = millis();
    updateSensors();
  }

  if(millis() - lastCameraRefresh >= CAMERA_REFRESH_DELAY) {
    Serial.println("Capturing Photo");
    lastCameraRefresh = millis();
    camera.takePicture();
  }
}

void updateSensors(){

}