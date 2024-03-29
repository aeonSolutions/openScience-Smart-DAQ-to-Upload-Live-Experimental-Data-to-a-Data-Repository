/*
 Copyright (c) 2023 Miguel Tomas, http://www.aeonlabs.science

License Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
You are free to:
   Share — copy and redistribute the material in any medium or format
   Adapt — remix, transform, and build upon the material

The licensor cannot revoke these freedoms as long as you follow the license terms. Under the following terms:
Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made. 
You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.

NonCommercial — You may not use the material for commercial purposes.

ShareAlike — If you remix, transform, or build upon the material, you must distribute your contributions under
 the same license as the original.

No additional restrictions — You may not apply legal terms or technological measures that legally restrict others
 from doing anything the license permits.

Notices:
You do not have to comply with the license for elements of the material in the public domain or where your use
 is permitted by an applicable exception or limitation.
No warranties are given. The license may not give you all of the permissions necessary for your intended use. 
For example, other rights such as publicity, privacy, or moral rights may limit how you use the material.


Before proceeding to download any of AeonLabs software solutions for open-source development
 and/or PCB hardware electronics development make sure you are choosing the right license for your project. See 
https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/wiki/AeonLabs-Solutions-for-Open-Hardware-&-Source-Development
 for Open Hardware & Source Development for more information.

*/
#include "Arduino.h"
#include "interface_class.h"
#include "m_wifi.h"
#include "m_display_lcd.h"

#include "esp32-hal-psram.h"
// #include "rom/cache.h"
extern "C" 
{
#include <esp_himem.h>
#include <esp_spiram.h>
}

#include <semphr.h>
#include "onboard_sensors.h"

#include "sensors/ds18b20.h"
#include "sensors/aht20.h"
#include "sensors/sht3x.h"

#ifndef MEASUREMENTS_COMMANDS  
  #define MEASUREMENTS_COMMANDS
  

  // **************************** == Measurements Class == ************************
class MEASUREMENTS {
    private:
        INTERFACE_CLASS* interface;
        M_WIFI_CLASS* mWifi = NULL ;
        DISPLAY_LCD_CLASS* display = NULL;
        ONBOARD_SENSORS* onBoardSensors = NULL;

        unsigned long LAST_DATASET_UPLOAD = 0;
        unsigned long LAST_DATA_MEASUREMENTS = 0;
        unsigned long MAX_LATENCY_ALLOWED;
        
        long int lastMillisSensors;

        uint8_t NUMBER_OF_SENSORS_DATA_VALUES;
    
        float **measurements = NULL; //pointer to pointer
        String* measurementsOnBoard;
        int measureIndex[2];

        const float MCU_ADC_DIVIDER = 4096.0;
        uint8_t SELECTED_ADC_REF_RESISTANCE;

        // GBRL commands  *********************************************
        bool helpCommands(String $BLE_CMD, uint8_t sendTo );
        bool history(String $BLE_CMD, uint8_t sendTo);
        bool measurementInterval(String $BLE_CMD, uint8_t sendTo);
        bool cfg_commands(String $BLE_CMD, uint8_t sendTo);
        bool gbrl_summary_measurement_config( uint8_t sendTo);
        bool sw_commands(String $BLE_CMD, uint8_t sendTo);

    public:
        // external 3V3 power
        uint8_t ENABLE_3v3_PWR_PIN;
        // Voltage reference
        uint8_t VOLTAGE_REF_PIN;
        // External PWM / Digital IO Pin
        uint8_t EXT_IO_ANALOG_PIN;

        bool hasNewMeasurementValues;
        float last_measured_probe_temp;
        float last_measured_time_delta;     
        int DATASET_NUM_SAMPLES;
        int DATASET_NUM_SAMPLES_TOTAL;
        
        bool Measurments_EN;
        bool Measurments_NEW;
        String measurement_Start_Time;

           // ...................................................     
        typedef struct{
            String EXPERIMENTAL_DATA_FILENAME = "measurements.csv";
            
            // Measurements: RAM Storage of Live Data  ******************************
            // array size is the number of sensors to do data collection
            int NUM_SAMPLE_SAMPLING_READINGS;
            long int SAMPLING_INTERVAL;
            int MEASUREMENTS_BUFFER_SIZE;

            // Measurements: Planning / Schedule  **********************************
            unsigned long UPLOAD_DATASET_DELTA_TIME;
            unsigned long MEASUREMENT_INTERVAL;

            // Reference resistance values for the embbeded resistivimeter : loaded from config file
            float ADC_REF_RESISTANCE[4];

            // configuration: PCB specific
            float    MCU_VDD = 3.38;

            bool     channel_1_switch_en;
            uint8_t  channel_1_switch_on_pos;
            bool     channel_2_switch_en;


        } config_strut;

        config_strut config;
        
        bool scheduleWait;
        long int waitTimeSensors;

        SemaphoreHandle_t MemLockSemaphoreDatasetFileAccess = xSemaphoreCreateMutex();
        bool datasetFileIsBusySaveData = false;
        bool datasetFileIsBusyUploadData = false;

        // external sensors __________________________________________
        SHT3X_SENSOR*     sht3x;
        AHT20_SENSOR*     aht20;
        DS18B20_SENSOR*   ds18b20;
        String ch2_sensor_type;


        MEASUREMENTS();
        
        void init(INTERFACE_CLASS* interface, DISPLAY_LCD_CLASS* display,M_WIFI_CLASS* mWifi, ONBOARD_SENSORS* onBoardSensors );

        void settings_defaults();
        // **********************************
        void readSensorMeasurements();
        void readExternalAnalogData();
        void readOnboardSensorData();
        void runExternalMeasurements();
        void readChannel2SensorMeasurements(int pos);
        void units();

        // ***********************************
        void initSaveDataset();
        bool saveDataMeasurements();

        bool initializeDataMeasurementsFile();
        bool initializeSensors();

        bool initializeDynamicVar( int size1D, int size2D);
        //Free Allocated memory
        void freeAllocatedMemory(int nRow, int nColumn);

        // GBRL commands  *********************************************
        bool gbrl_commands(String $BLE_CMD, uint8_t sendTo);

        // Setup configuration and settings *******************************************
        bool readSettings( fs::FS &fs = LittleFS );
        bool saveSettings( fs::FS &fs = LittleFS  );

};


#endif
