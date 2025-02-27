#include <Adafruit_MMC56x3.h>
#include <LSM6DSOX.h>
//#include <cmath> //may be needed, not super sure


//********************************************************************************************************** */
//MMC5603 functions
class MMCModule {
    private:
    bool magflag = false;
    Adafruit_MMC5603 mag = Adafruit_MMC5603(12345);
    double magnet_data[3];
    double degrees;

    void collect_mag(){
        double temp[3];
        sensors_event_t event;
        mag.getEvent(&event);
        temp[0] = event.magnetic.x;
        temp[1] = event.magnetic.y;
        temp[2] = event.magnetic.z;
    }
    // double collect_mag(){ //may need alpha filter
    //     double result[3];
    //     sensors_event_t event;
    //     mag.getEvent(&event);
    //     result[0] = event.magnetic.x;
    //     result[1] = event.magnetic.y;
    //     result[2] = event.magnetic.z;
    //     return result;
    // }

    // double collect_deg(){
    //     sensors_event_t event;
    //     mag.getEvent(&event);
    //     return atan2(event.magnetic.y, event.magnetic.x) * 180 / 3.14159;
    // }
    //collects N samples and averages the middle ones
    String collect_N_mag(uint8_t N){// N >= 3
        if (N < 3) return "N too small";  

        sensors_event_t mag_event[N];  
        float magnitudes[N];  

        // Collect data and compute magnitudes
        for (int i = 0; i < N; i++) {
            mag.getEvent(&mag_event[i]);
            float x = mag_event[i].magnetic.x;
            float y = mag_event[i].magnetic.y;
            float z = mag_event[i].magnetic.z;
            magnitudes[i] = sqrt(x * x + y * y + z * z);
            delay(5);
        }

        // Sort based on precomputed magnitudes
        std::sort(mag_event, mag_event + N, [&](const sensors_event_t &a, const sensors_event_t &b) {
            return magnitudes[&a - mag_event] < magnitudes[&b - mag_event];
        }); 

        // Compute the average excluding the lowest and highest values  
        float avg_x = 0, avg_y = 0, avg_z = 0;  
        for (int i = 1; i < N - 1; i++) {  
            avg_x += mag_event[i].magnetic.x;  
            avg_y += mag_event[i].magnetic.y;  
            avg_z += mag_event[i].magnetic.z;  
        }  

        avg_x /= (N - 2);  
        avg_y /= (N - 2);  
        avg_z /= (N - 2);  

        return "X: " + String(avg_x, 3) + " |Y: " + String(avg_y, 3) + " |Z: " + String(avg_z, 3);  
    }

    String collect_N_deg(uint8_t N){
        if(magflag) return "MMC5603 not initialized";

        sensors_event_t deg_event[N];
        float degrees[N];

        //for loop for getting the degrees
        for (int i = 0; i < N; i++) {
            mag.getEvent(&deg_event[i]);
            degrees[i] = atan2(deg_event[i].magnetic.y, deg_event[i].magnetic.x) * 180 / 3.14159;
            delay(5);
        }

        //sort
        std::sort(degrees, degrees + N);

        //average
        float avg_degree = 0;
        for (int i = 1; i < N - 1; i++) {
            avg_degree += degrees[i];
        }
        avg_degree /= (N - 2);
        return String(avg_degree, 2) + "°";
    }

    public:

    String init_mag(){
        if(!mag.begin(MMC56X3_DEFAULT_ADDRESS, &Wire)){
            magflag = true;
            return "MMC5603 Failure";
        }
        else {
            magflag = false;
            return "MMC5603 Initialized";
        }
    }

    double get_data(){ //x, y, z, degrees
        if (magflag) return -1;
        double result[4];
        double temp[3] = collect_mag();
        result[0] = temp[0];
        result[1] = temp[1];
        result[2] = temp[2];
        result[3] = collect_deg();
        return result;
    }
    String tick_mag(uint8_t N){ // N is the number of samples to be averaged, at least 3
        if (magflag) return "MMC5603 not initialized";
        return "Mag Sensor (uT) " + collect_N_mag(N);
    }

    String tick_deg(uint8_t N){ // N is the number of samples to be averaged, at least 3
        if (magflag) return "MMC5603 not initialized";
        return "Degrees: " + collect_N_deg(N);
    }

};
//********************************************************************************************************** */
//LMS6DSOX functions

class LMSModule{
    private:

    bool accflag = false;
    
    double collect_acc(){
        double result[3];
        if (!IMU.accelerationAvailable()) return "LSM6DSOX: Acceleration not available";
        IMU.readAcceleration(result[0], result[1], result[2]);
        return result;
    }

    double collect_gyro(){
        double result[3];
        if (!IMU.gyroscopeAvailable()) return "LSM6DSOX: Gyroscope not available";
        IMU.readGyroscope(result[0], result[1], result[2]);
        return result;
    }

    double collect_temp(){ //Redundant?
        double result;
        if (!IMU.temperatureAvailable()) return "LSM6DSOX: Temperature not available";
        bool readSuccess = IMU.readTemperatureFloat(result);
        if(!readSuccess) return "LSM6DSOX: Temperature not available";
        return double(result);
    }

    String collect_N_acc(uint8_t N){
        if (accflag) return "LSM6DSOX not initialized";
        if (N < 3) return "N too small";

        float acc_data[N][3];  // Store x, y, z
        float magnitudes[N];

        // Collect data and compute magnitudes
        for (int i = 0; i < N; i++) {
            while (!IMU.accelerationAvailable());
            IMU.readAcceleration(acc_data[i][0], acc_data[i][1], acc_data[i][2]);

            float x = acc_data[i][0], y = acc_data[i][1], z = acc_data[i][2];
            magnitudes[i] = sqrt(x * x + y * y + z * z);

            delay(5);
        }

        //these next few lines would not compile so I removed them until we can find a better solution.

        // Sort indices based on magnitude
        /*std::sort(acc_data, acc_data + N, [&](const float a[3], const float b[3]) {
            return magnitudes[&a - acc_data] < magnitudes[&b - acc_data];
        });*/

        // Compute the average excluding the lowest and highest values
        float avg_x = 0, avg_y = 0, avg_z = 0;
        for (int i = 1; i < N - 1; i++) {
            avg_x += acc_data[i][0];
            avg_y += acc_data[i][1];
            avg_z += acc_data[i][2];
        }

        avg_x /= (N - 2);
        avg_y /= (N - 2);
        avg_z /= (N - 2);

        return "X: " + String(avg_x, 3) + " |Y: " + String(avg_y, 3) + " |Z: " + String(avg_z, 3);

    }

    String collect_N_gyro(uint8_t N){
        if (accflag) return "LSM6DSOX not initialized";
        if (N < 3) return "N too small";

        float gyro_data[N][3];  // Store x, y, z
        float magnitudes[N];

        // Collect data and compute magnitudes
        for (int i = 0; i < N; i++) {
            if (!IMU.gyroscopeAvailable()) return "LSM6DSOX: Gyroscope not available";
            IMU.readGyroscope(gyro_data[i][0], gyro_data[i][1], gyro_data[i][2]);

            float x = gyro_data[i][0], y = gyro_data[i][1], z = gyro_data[i][2];
            magnitudes[i] = sqrt(x * x + y * y + z * z);

            delay(5);
        }

        //these next few lines would not compile so I removed them until we can find a better solution.

        // Sort indices based on magnitude
        /*std::sort(gyro_data, gyro_data + N, [&](const float a[3], const float b[3]) {
            return magnitudes[&a - gyro_data] < magnitudes[&b - gyro_data];
        });*/

        // Compute the average excluding the lowest and highest values
        float avg_x = 0, avg_y = 0, avg_z = 0;
        for (int i = 1; i < N - 1; i++) {
            avg_x += gyro_data[i][0];
            avg_y += gyro_data[i][1];
            avg_z += gyro_data[i][2];
        }

        avg_x /= (N - 2);
        avg_y /= (N - 2);
        avg_z /= (N - 2);

        return "X: " + String(avg_x, 3) + " |Y: " + String(avg_y, 3) + " |Z: " + String(avg_z, 3);
    }

    String collect_N_temp(uint8_t N){
        if (accflag) return "LSM6DSOX not initialized";
        if (N < 3) return "N too small";

        float temp_data[N];
        for (int i = 0; i < N; i++) {
            if (!IMU.temperatureAvailable()) return "LSM6DSOX: Temperature not available";
            bool readSuccess = IMU.readTemperatureFloat(temp_data[i]);
            if(!readSuccess) return "LSM6DSOX: Temperature not available";
            delay(5);
        }

        std::sort(temp_data, temp_data + N);

        float avg_temp = 0;
        for (int i = 1; i < N - 1; i++) {
            avg_temp += temp_data[i];
        }
        avg_temp /= (N - 2);

        return String(avg_temp, 3);

    }

    public:

    double get_data(){
        double result[7];
        double temp[3] = collect_acc();
        result[0] = temp[0];
        result[1] = temp[1];
        result[2] = temp[2];
        temp = collect_gyro();
        result[3] = temp[0];
        result[4] = temp[1];
        result[5] = temp[2];
        result[6] = collect_temp();
        return result;

    }


    String init_LSM6DOX(){
        if(!IMU.begin()){
            accflag = true;
            return "LSM6DSOX Failure";
        }
        else {
            accflag = false;
            return "LSM6DSOX Initialized";
        }
    }

    String tick_acc(uint8_t N){ // N is the number of samples to be averaged, at least 3
        if (accflag) return "LSM6DSOX not initialized";
        return "Accel (m/s^2) " + collect_N_acc(N);
    }

    String tick_gyro(uint8_t N){ // N is the number of samples to be averaged, at least 3
        if (accflag) return "LSM6DSOX not initialized";
        return "Gyro (rad/s) " + collect_N_gyro(N);
    }


    String tick_temp(uint8_t N){
        if (accflag) return "LSM6DSOX not initialized";
        return "Temperature (°C): " + collect_N_temp(N);
    }

};


//********************************************************************************************************** */
/*
#include "MS5611.h"
MS5611 MS5611(0x77);
//needs fast baud rate
MS5611.begin();
MS5611.getAddress();
M55611.reset(1); //not sure how this resets it. Maybe calls some bias getter.
MS5611.setOversampling(OSR_ULTRA_HIGH);

MS5611.read()
//should output MS5611_READ_OK
MS5611.getPressure();
MS5611.getPressurePascal();
MS5611.getTemperature();




*/

/*
  There are 5 oversampling settings, each corresponding to a different amount of milliseconds
  The higher the oversampling, the more accurate the reading will be, however the longer it will take.
  OSR_ULTRA_HIGH -> 8.22 millis
  OSR_HIGH       -> 4.11 millis
  OSR_STANDARD   -> 2.1 millis
  OSR_LOW        -> 1.1 millis
  OSR_ULTRA_LOW  -> 0.5 millis   Default = backwards compatible
*/

