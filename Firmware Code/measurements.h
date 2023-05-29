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

#ifndef MEASUREMENTS_COMMANDS  
  #define MEASUREMENTS_COMMANDS
  

  // **************************** == Measurements Class == ************************
class MEASUREMENTS {
    private:
        INTERFACE_CLASS* interface=nullptr;
        M_WIFI_CLASS* mWifi=nullptr;
        DISPLAY_LCD_CLASS* display;
        ONBOARD_SENSORS* onBoardSensors ;

        unsigned long LAST_DATASET_UPLOAD = 0;
        unsigned long LAST_DATA_MEASUREMENTS = 0;
        unsigned long MAX_LATENCY_ALLOWED;
        
        long int lastMillisSensors;

        uint8_t NUMBER_OF_SENSORS_DATA_VALUES;
    
        char ****measurements = NULL; //pointer to pointer
        int measurements_current_pos=0;

        // external 3V3 power
        uint8_t ENABLE_3v3_PWR;
        // Voltage reference
        uint8_t VOLTAGE_REF_PIN;
        // External PWM / Digital IO Pin
        uint8_t EXT_IO_ANALOG_PIN;

        const float MCU_ADC_DIVIDER = 4096.0;
        uint8_t SELECTED_ADC_REF_RESISTANCE;

        // GBRL commands  *********************************************
        bool helpCommands( uint8_t sendTo );

    public:
           // ...................................................     
        typedef struct{
            String EXPERIMENTAL_DATA_FILENAME = "ER_measurements.csv";

            // Measurements: Live Data Acquisition  ******************************
            union Data {
            int i;
            float f;
            char chr[20];
            };  
            
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
            float MCU_VDD = 3.38;

        } config_strut;

        config_strut config;
        
        bool scheduleWait;
        long int waitTimeSensors;

        SemaphoreHandle_t MemLockSemaphoreDatasetFileAccess = xSemaphoreCreateMutex();
        bool datasetFileIsBusySaveData = false;
        bool datasetFileIsBusyUploadData = false;

        MEASUREMENTS();
        
        void init(INTERFACE_CLASS* interface, DISPLAY_LCD_CLASS* display,M_WIFI_CLASS* mWifi, ONBOARD_SENSORS* onBoardSensors );

        void settings_defaults();
        bool loadSettings( fs::FS &fs = LittleFS );
        bool saveSettings( fs::FS &fs = LittleFS );

        // **********************************
        void ReadExternalAnalogData();
        void runExternalMeasurements();
        void units();

        // ***********************************
        void initSaveDataset();
        bool saveDataMeasurements();

        bool initializeDataMeasurementsFile();

        bool initializeDynamicVar( int size1D, int size2D, int size3D);
        //Free Allocated memory
        void freeAllocatedMemory(int ****measurements, int nRow, int nColumn, int dim3);

        // GBRL commands  *********************************************
        bool commands(String $BLE_CMD, uint8_t sendTo );
};


#endif
