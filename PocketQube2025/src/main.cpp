#include <Arduino.h>
#include <Adafruit_MMC56x3.h>
#include <LSM6DSOX.h>

// put function declarations here:
Adafruit_MMC5603 mmc = Adafruit_MMC5603(12345);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  while (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");

    IMU.end();
  }

  if (!mmc.begin(MMC56X3_DEFAULT_ADDRESS, &Wire)) {  // I2C mode
    /* There was a problem detecting the MMC5603 ... check your connections */
    Serial.println("Ooops, no MMC5603 detected ... Check your wiring!");
    while (1) delay(10);
  }

  /* Display some basic information on this sensor */
  mmc.printSensorDetails();

  mmc.setDataRate(100); // in Hz, from 1-255 or 1000
  mmc.setContinuousMode(true);

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration in g's");
  Serial.println("X\tY\tZ");
}

void loop() {
  static float x, y, z;

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    Serial.print("Acceleration: ");
    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);
  }

  if(IMU.gyroscopeAvailable()){
    IMU.readGyroscope(x, y, z);

    Serial.print("Gyro: ");
    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);
  }
  
  if(IMU.temperatureAvailable()){
    float temp;
    IMU.readTemperatureFloat(temp);
    Serial.print("Temp: ");
    Serial.println(temp);
  }

  sensors_event_t event;
  mmc.getEvent(&event);

  // Display the results (magnetic vector values are in micro-Tesla (uT))
  Serial.print("X: ");
  Serial.print(event.magnetic.x);
  Serial.print("  ");
  Serial.print("Y: ");
  Serial.print(event.magnetic.y);
  Serial.print("  ");
  Serial.print("Z: ");
  Serial.print(event.magnetic.z);
  Serial.print("  ");
  Serial.println("uT");

  // Delay before the next sample
  delay(10);
}