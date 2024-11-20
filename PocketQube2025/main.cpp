/***********************************************************
 * 
 * Code for Weather-Ballon launch
 * Author: Michael Williams
 * Organization: BYU Spacecraft Club
 * 
 * First version: 2/16/2023
 * 
 * for use with Seeeed Studio XIAO SAMD21 processor chip
 * 
************************************************************/

// MacroDirectives and library includes
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SD.h>
#include <ArduCAM.h>
#include <LoRa.h>
// #include "memorysaver.h"


// Global constants

#define SEALEVELPRESSURE_HPA   1013.25           // Pressure at sea level to be used in getting altitude.
#define CAPTURE_ALTITUDE       0          // Altitude to start capture, units in meters
#define IMAGE_INTERVAL_TIME    500           // Time in between image captures in milliseconds
#define TRANSMIT_INTERVAL_TIME 500              // Time in between transmissions in milliseconds 

// original Altitude reading on setup
float groundAltitude;



// Pin Naming for chip select
#define Pressure_Sensor_Pin   3
#define SD_Card_Pin           7 //was 7
#define Camera_Pin            2
#define Radio_Pin             0
#define Radio_Reset           1 //1
//#define Radio_DID0            1 //0

// // HardWare pins on SAMD21
 #define SDA   4
 #define SCL   5
 #define SCK   8
 #define MISO  9
 #define MOSI  10

// All forward function declarations
void perception();
bool reachedAltitude();
void planning();
void action();
float read_altitude();
float read_pressure();
float read_humidity();
float read_pressure();
void read_image(String name);
void write_logfile(String message);
void take_image();
void ping();
void init_camera();
void write_new_captures();
void fsmMain();
bool imageTrigger();
bool TransmissionTrigger();
String getImageName();
String getTime();


// sensor initializations

Adafruit_BME280 pressure_sensor(Pressure_Sensor_Pin); // Object for pressure sensor
ArduCAM myCAM( OV2640, Camera_Pin ); // Camera
File logfile;  // file handles 
File imageLog;

/*********************************************************
 * 
 * Global Flags for use in state machines and Actions
 * 
 * 
**********************************************************/
bool START = false;
int imageIndex = 0;
unsigned long lastCaptureTime = 0; // storage of the last time a image was taken gotten from the millis() function
unsigned long lastTransmitTime = 0; // storage of the last time radio was transmited was taken gotten from the millis() function


// fsmMain signals 
// State Machine States
#define IDLE      0
#define HUB       1
#define IMAGE     2
#define WRITE     3
#define READ      4
#define TRANSMIT  5


// Input Signals to control state Machine states
int AltitudeReached   = 1;
int CaptureDone       = 0;
int WriteDone         = 0;
// int Transmit          = 0;
int ReadDone          = 0;
int TransmissionSent  = 0;
// bool CaptureImage      = true;

// Output Signals to control robot actions
int ACTION            = 0;
#define WAIT_IMAGE      0
#define TAKE_IMAGE      1 
#define WRITE_IMAGE     2 
#define READ_IMAGE      3
#define TRANSMIT_IMAGE  4 

// IMPORTANT: radio code while here is not tested due to what is belived to be a hardware issue


void setup() {
  // add all setup commands here

  // Serial Setup
  Serial.begin(9600);
  //I2C
  Wire.begin();
  SPI.begin();


  // CS pin setup
  pinMode(SD_Card_Pin, OUTPUT); // setup the SD Card chip select
  digitalWrite(SD_Card_Pin, HIGH); // disable SD Card

  pinMode(Camera_Pin, OUTPUT); // setup the camera chip select
  digitalWrite(Camera_Pin, HIGH); // disable camera

  pinMode(Pressure_Sensor_Pin, OUTPUT); // setup the BME chip(altimiter) select
  digitalWrite(Pressure_Sensor_Pin, HIGH);  // disable BME chip sensor

  // pinMode(Radio_Pin, OUTPUT);
  // digitalWrite(Radio_Pin, HIGH);


  // SD card setup
  if(!SD.begin(SD_Card_Pin)){
    // Serial.println("Failed to Initalize SD Card");
  }

  // Camera Setup
  init_camera();

  // BME(Pressure sensor) setup
  pressure_sensor.begin();
  groundAltitude = read_altitude();

  // Radio setup
  digitalWrite(Radio_Pin, LOW);
  // LoRa.setPins(Radio_Pin, Radio_Reset, Radio_DID0);

  // if(!LoRa.begin(433E6)) {
  //   Serial.println("Starting LoRa failed!");
  //   //while(1);
  // }
  // digitalWrite(Radio_Pin, HIGH); // disable the radio

  // pinMode(0, INPUT);
  Serial.println("Setup Complete");

  // Logfile opening
  write_logfile("-----BEGIN LOGFILE-----");
  write_logfile("Ground Altitude read: " + String(groundAltitude) + " meters");
  write_logfile("Target Altitude: " + String(CAPTURE_ALTITUDE) + " meters");

}

void loop() {
  // Principle of Perception, Planning, Action
  //      Also included a log function for logging events, 
  //      could be done interspersed through the program instead

  //perception();
  
    planning();
    action();
  
  // log();
  // test only section:
  // delay(1000);

}



/*
  * SECTION 1
  * Perception
  * 
  * Sensors:
  * BME (atmospheric sensor)
  * 
  * 5 * (float)analogRead(0) / 1024 > 4.5
  * this function is not used for cautionary purposed
  * if it was used it would pull the procssor from the wait state after reaching the target altitude
*/
void perception() {
  if( reachedAltitude()) {
    AltitudeReached = 1;
    START = true;
  } else {
    START = false;
    AltitudeReached = 0;
  }
}

// returns a bool wether they have reached the target altidute
bool reachedAltitude() {
  if ((read_altitude() - groundAltitude) > CAPTURE_ALTITUDE) {
    return true;
  } else {
    return false;
  }
}



/*
  * SECTION 2 
  * Planning
  * 
  * State Machines:
  * 
*/
void planning() {
  fsmMain();
  
}

void fsmMain() {
  static int mainState = HUB;
  // Serial.println(mainState);

  switch(mainState) {
    case IDLE:
      ACTION = WAIT_IMAGE;
      
      // State Transition logic
      if(AltitudeReached) {
        write_logfile("Target Altitude reached - Above target");
        mainState = HUB;
      }
      break;
    
    case HUB:
      ACTION = WAIT_IMAGE;

      // State Transition Logic
      if(imageTrigger()) {
        mainState = IMAGE; // take the photo
      } else if (TransmissionTrigger()) {
        mainState = TRANSMIT; // trigger the radio/transmit the photo
      }
      break;

    case IMAGE:
      ACTION = TAKE_IMAGE;
      // State Transition Logic
      mainState = WRITE;
      break;

    case WRITE:
      ACTION = WRITE_IMAGE;
      // State Transition Logic
      mainState = HUB;
      break;

    case READ:
      ACTION = READ_IMAGE;
      // State Transition Logic
      if(ReadDone) {
        mainState = TRANSMIT;
      }
      break;

    case TRANSMIT:
      ACTION = TRANSMIT_IMAGE;
      // State Transition Logic
      if(TransmissionSent) {
        mainState = HUB;
      }
      break;
  }
}

// returns a bool wether the async camera capture has completed
bool isCameraDone(){
  if(myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) 
    return true;
  else 
    return false;
}

// wether to caputure the image in this case elapsed time
bool imageTrigger() {
  if(millis() - lastCaptureTime > IMAGE_INTERVAL_TIME) {
    return true;
  }
  else {
    return false;
  }
  
}

// wether to trigger the radio in this case elapsed time
bool TransmissionTrigger() {
  if(millis() - lastTransmitTime > TRANSMIT_INTERVAL_TIME) {
    return true;
  }
  else {
    return false;
  }
  
}

/*
* SECTION 3
* Action
* 
* Functions:
* 
*/
void action() {
  switch(ACTION) {
    case WAIT_IMAGE:
      // For now, do nothing
    break;
    case TAKE_IMAGE:

      take_image(); // trigger the image caputer
      lastCaptureTime = millis(); // set the previous capture time
      while(!isCameraDone()); // wait for the caputre to complete

      break;
    case WRITE_IMAGE:

      write_new_captures();

      break;
    case READ_IMAGE:
      // FIXME:: add functions to read the image needed
      break;
    case TRANSMIT_IMAGE:

      //ping(); // transmit
      lastTransmitTime = millis(); // set the last transmit time
      break;
  }
  
}

/*
 * 
 * SECTION 4 
 * Functions for other use
 * 
 * Subsections:
 * BME pressure sensor reading
 * SD card control 
 * Camera operations
 * Radio operations
 * 
*/

// BME pressure sensor reading
float read_altitude() { // units meters

  digitalWrite(Pressure_Sensor_Pin, LOW);
  float result = pressure_sensor.readAltitude(SEALEVELPRESSURE_HPA);
  digitalWrite(Pressure_Sensor_Pin, HIGH);
  
  return result;
}
float read_pressure() { // units hpa

  digitalWrite(Pressure_Sensor_Pin, LOW);
  float result = pressure_sensor.readPressure() / 100.0F;
  digitalWrite(Pressure_Sensor_Pin, HIGH);

 return result;
}
float read_humidity() { // units %

  digitalWrite(Pressure_Sensor_Pin, LOW);
  float result = pressure_sensor.readHumidity();
  digitalWrite(Pressure_Sensor_Pin, HIGH);

  return result;
}
float read_temperature() { // units °C

  digitalWrite(Pressure_Sensor_Pin, LOW);
  float result = pressure_sensor.readTemperature();
  digitalWrite(Pressure_Sensor_Pin, HIGH);

  return result;
}

// SD card control Functions
// writes new image captures if avalable else will return
// names for the files are dictated by the getImageName() function which is called once if their is a image to write
void write_new_captures(){
  if(!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) return; // for async
  // get the name of the new image
  String name = getImageName();
  File outfile = SD.open(name, O_WRITE | O_CREAT); // the arduino sd library is limited to file names of 8 characters wide by 3 wide in extention
  if(!outfile){ // if failed to create the image file
    write_logfile("Failed to create image file");
    return;
  }
  write_logfile("Image Capture Ready");
  Serial.println("TEST");
  uint32_t length = myCAM.read_fifo_length(); //read image length
  myCAM.set_fifo_burst(); //Set fifo burst mode for easy reads
  uint8_t data;
  while(length > 0){
    myCAM.CS_LOW(); // active the camera
    myCAM.set_fifo_burst(); // has to be reset as far a I know every time TODO!: look into this
    data = SPI.transfer(0x00); // read the data
    myCAM.CS_HIGH(); // deactive the camera for sd
    outfile.write(data); // write to file
    length--;
  }
  write_logfile("Finished Image Write");
  myCAM.clear_fifo_flag(); // clear the image capture ready flag
  outfile.close();
  if(!SD.exists(name)){
    write_logfile("Created Image file and wrote to it but it doesn't exist after writing");
  }

}

//TODO!: implment this function
// takes an id of the image and preps the image for radio tranmission
// this implmentation is mostly undetermined but was left in place for the future
void read_image(int id) {

}

// logs a message to a logfile as well as serial
void write_logfile(String message) {
  logfile = SD.open("logfile.txt", FILE_WRITE);

   if (logfile) {
    logfile.print(getTime() + " ");
    logfile.println(message);
    logfile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening logfile.txt");
  }
  Serial.println(message);
}

//trigger image caputure async
void take_image() {
  //clear flags
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  //start flags
  write_logfile("Starting Camera Capture");
  myCAM.start_capture();

}

//initalize camera
void init_camera(){
  //reset camera
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  //check test register
  while(1){ // just writes to a test port and if it can read it back then it is good
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    uint8_t temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55){
      write_logfile("Failed to connect to Camera via SPI");
      delay(10000);
      continue; // retry until good connection
    }else{
      write_logfile("Succesfully connected to Camera module via SPI");
      break;   // good connection continue with the code
    }
  }
  //check the camera type
  uint8_t vid, pid;
  while(1){
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      write_logfile("Failed to find the Correct Camera Module");
      delay(1000);
      continue;
    }else{
      break;
    }
  }
  //setup camera settings
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
}


// Radio control Functions
void ping() {
  static int Rad_Count = 0;

  Serial.print("Sending packet: ");
  Serial.println(Rad_Count);

  // Send packet
  LoRa.beginPacket();
  LoRa.print("helloasdfasdfasdf ");
  LoRa.print(Rad_Count);
  LoRa.endPacket();

  Rad_Count++;
}

// returns the time sence start formated as a string
String getTime(){
  int ts = millis() / 1000;
  int tm = ts / 60;
  int th = tm / 60;
  tm = tm % 60;
  ts = ts % 60;
  String timeStamp = String(th) + ":" + String(tm); 
  timeStamp +=":" + String(ts);
  return timeStamp;
}

//returns the name of the next image as a string
//all it dose is increment a static int for each name giving each image a unique id
String getImageName(){
  static int nextWrite = 200;

  String name = String(nextWrite);

  String imageName = name + ".jpg";
  nextWrite++;


  // Write the image name and timestamp to a logfile
  String message = "Name - " + imageName + "    Time - " + getTime() + "    Altitude - " + read_altitude();

  // writes the name to a log file to track info
  imageLog = SD.open("ImgLog.txt", FILE_WRITE);
  if(imageLog) {
    imageLog.println(message);
    imageLog.close();
  } else {
    write_logfile("Failed to write to the image log");
  }
  write_logfile(message);

  return imageName;
}
