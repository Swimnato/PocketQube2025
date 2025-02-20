#include <SD.h>

#define PATH_TO_LOG String("sensors/")
#define PATH_TO_LOG_POS PATH_TO_LOG + "pos.txt"
#define CSV_LENGTH 1000
#define SAMPLES_TO_AVERAGE 10

#define NUM_ITEMS_IN_CSV 10
enum columnValues : byte {
    TIME,
    ACCELERATION_X,
    ACCELERATION_Y,
    ACCELERATION_Z,
    PRESSURE,
    TEMP,
    CURRENT_IN,
    CURRENT_OUT,
    ALTITUTE,
    PHOTO
};

class DataPersistance{
    private:
        const char* columnLabels = "Time,Accel_x,Accel_y,Accel_z,Pressure,Temp,Solar Current In,Current Consumption,Altitute(calculated),Photo#";

        double rawData[SAMPLES_TO_AVERAGE][NUM_ITEMS_IN_CSV];
        unsigned row = 0;

        int calculateAltitude(double pressure){
            //TODO implement this to be more accurite
            return (1-pow(pressure/1013.25,.190284)*145366.45);
        }

        void appendData(File* destination, double* data, int photoNumber){
            noInterrupts();
            for(byte col = 0; col < NUM_ITEMS_IN_CSV; col++){
                switch(col){
                    case ALTITUTE:
                        destination->print(calculateAltitude(data[ALTITUTE]));
                        break;
                    case PHOTO:
                        if(photoNumber){
                            destination->print(photoNumber);
                        }
                        break;
                    default:
                        destination->print(data[col]);
                        break;
                }
                destination->print(',');
                interrupts();
            }
            noInterrupts();
            destination->println();
            interrupts();
        }

        double* averageData(){
            double* dataOut = new double[NUM_ITEMS_IN_CSV];
            
            for(byte col = 0; col < NUM_ITEMS_IN_CSV - 2; col++){
                for(unsigned short sample = 0; sample < SAMPLES_TO_AVERAGE; sample++){
                    dataOut[col] += rawData[sample][col];
                }
                dataOut[col] /= SAMPLES_TO_AVERAGE;
            }

            return dataOut;
        }
    public:

        void init(){
            if(!SD.exists(PATH_TO_LOG)){
                SD.mkdir(PATH_TO_LOG);
            }
            if(!SD.exists(PATH_TO_LOG_POS)){
                File positionFile = SD.open(PATH_TO_LOG_POS, FILE_WRITE);
                positionFile.println(0);
                positionFile.println(0);
                positionFile.close();
            }
        }

        void addData(double* data){
            for(byte col = 0; col < NUM_ITEMS_IN_CSV; col++){
                switch(col){
                    case ALTITUTE:
                        break;
                    case PHOTO:
                        break;
                    default:
                        rawData[row][col] = data[col];
                        break;
                }
            }
            row++;
            if(row >= SAMPLES_TO_AVERAGE){
                row = 0;
            }
        }

        void addToCSV(int photoNumber){
            static unsigned position, fileNumber;
            static bool writeHeader;
            writeHeader = false;

            noInterrupts();
            File positionFile = SD.open(PATH_TO_LOG_POS, FILE_READ);
            positionFile.seek(0);
            fileNumber = positionFile.parseInt();
            position = positionFile.parseInt();
            positionFile.close();

            interrupts();
            position++;
            if(fileNumber == 0 || position > CSV_LENGTH){
                fileNumber++;
                position = 1;
                writeHeader = true;
            }

            noInterrupts();
            positionFile = SD.open(PATH_TO_LOG_POS, O_RDWR);
            positionFile.println(fileNumber);
            positionFile.println(position);
            positionFile.close();

            File CSV = SD.open(PATH_TO_LOG + fileNumber + ".csv", FILE_WRITE);
            if(writeHeader)
                CSV.println(columnLabels);
            interrupts();
            double* data = averageData();
            noInterrupts();
            appendData(&CSV, data, photoNumber);
            delete data;
        }

};