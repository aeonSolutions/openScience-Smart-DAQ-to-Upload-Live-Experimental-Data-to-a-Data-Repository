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

#include "measurements.h"
#include "Arduino.h"
#include"m_math.h"
#include "m_atsha204.h"
#include "FS.h"
#include <LittleFS.h>
#include "m_atsha204.h"
#include "lcd_icons.h"
#include "m_file_functions.h"


MEASUREMENTS::MEASUREMENTS() {
    // external 3V3 power
    this->ENABLE_3v3_PWR_PIN = 38;
    // Voltage reference
    this->VOLTAGE_REF_PIN = 7;
    // External PWM / Digital IO Pin
    this->EXT_IO_ANALOG_PIN = 6;
    this->SELECTED_ADC_REF_RESISTANCE = 0;
}


//****************************************************************
void MEASUREMENTS::init(INTERFACE_CLASS* interface, DISPLAY_LCD_CLASS* display,M_WIFI_CLASS* mWifi, ONBOARD_SENSORS* onBoardSensors ){    
    this->interface = interface;
    this->interface->mserial->printStrln("\ninit measurements library ...");
    this->mWifi = mWifi;
    this->display = display;
    this->onBoardSensors =  onBoardSensors;

    // ADC 
    pinMode(this->EXT_IO_ANALOG_PIN, INPUT);

    this->ds18b20 = new DS18B20_SENSOR();
    this->ds18b20->init(this->interface, this->EXT_IO_ANALOG_PIN);

    // ADC Power
    pinMode(this->ENABLE_3v3_PWR_PIN, OUTPUT);

    this->settings_defaults();

    this->interface->mserial->printStrln("Initializing Dynamic memory:");
    this->interface->mserial->printStrln("Number of SAMPLING READINGS:"+ String(this->config.NUM_SAMPLE_SAMPLING_READINGS));
    this->interface->mserial->printStrln("Number of Sensor Data values per reading:" + String(this->NUMBER_OF_SENSORS_DATA_VALUES));
    this->interface->mserial->printStrln("PSRAM buffer size:" + String(this->config.MEASUREMENTS_BUFFER_SIZE));
    this->display->tftPrintText(0,160,(char*) String("Buff. size:"+ String(this->config.MEASUREMENTS_BUFFER_SIZE)).c_str(),2,"center", TFT_WHITE, true); 
    delay(2000);

    String units="";
    // 1D number of sample readings ; 2D number of sensor data measuremtns; 3D RAM buffer size
    if ( false == this->initializeDynamicVar(this->config.NUM_SAMPLE_SAMPLING_READINGS, (int) this->NUMBER_OF_SENSORS_DATA_VALUES , this->config.MEASUREMENTS_BUFFER_SIZE) )
    {
      this->interface->mserial->printStrln("Error Initializing Measruments Buffer.");
      this->display->tftPrintText(0,160,"ERR Exp. data Buffer",2,"center", TFT_WHITE, true); 
      this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_RED;
      this->interface->onBoardLED->statusLED(100, 5);
      // TODO : what to do when memeory alloc is NULL

    }else{
        this->interface->mserial->printStrln("Measurements Buffer Initialized successfully.");
        float bufSize= sizeof(char)*(this->config.NUM_SAMPLE_SAMPLING_READINGS * this->NUMBER_OF_SENSORS_DATA_VALUES * this->config.MEASUREMENTS_BUFFER_SIZE); // bytes
        units=" B";
        if (bufSize>1024){
            bufSize=bufSize/1024;
            units=" Kb";
        }
        this->interface->mserial->printStrln("Buffer size:" + String(bufSize) + units);
        this->display->tftPrintText(0,160,(char*) String("Buf size:"+String(bufSize)+ units).c_str(),2,"center", TFT_WHITE, true); 
        delay(1000);
        this->display->tftPrintText(0,160,"Exp. Data RAM ready",2,"center", TFT_WHITE, true); 
        this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_GREEN;
        this->interface->onBoardLED->statusLED(100, 2);
    }


    this->interface->mserial->printStrln("done.");
}

// ****************************************************************************
void MEASUREMENTS::settings_defaults(){

    this->config.NUM_SAMPLE_SAMPLING_READINGS = 16;
    this->config.SAMPLING_INTERVAL = 0;
    this->config.MEASUREMENTS_BUFFER_SIZE = 10;

    this->config.UPLOAD_DATASET_DELTA_TIME= this->config.NUM_SAMPLE_SAMPLING_READINGS*this->config.SAMPLING_INTERVAL + 120000; // 10 min
    this->config.MEASUREMENT_INTERVAL= this->config.NUM_SAMPLE_SAMPLING_READINGS*this->config.SAMPLING_INTERVAL + 00000; // 1 min
    this->LAST_DATASET_UPLOAD = 0;
    this->LAST_DATA_MEASUREMENTS = 0;
    this->MAX_LATENCY_ALLOWED = (unsigned long)(this->config.MEASUREMENT_INTERVAL/2);

    // Reference resistances : loaded from config file
    uint8_t SELECTED_ADC_REF_RESISTANCE=0;
    
    this->config.ADC_REF_RESISTANCE[0] = 1032;
    this->config.ADC_REF_RESISTANCE[1] = 19910;
    this->config.ADC_REF_RESISTANCE[2] = 198000;
    this->config.ADC_REF_RESISTANCE[3] = 2000000;

    this->config.channel_1_switch_en = false;
    this->config.channel_1_switch_on_pos = 0;
    this->config.channel_2_sensor_type = "off"; 

      // ADC Power
    pinMode(ENABLE_3v3_PWR_PIN, OUTPUT);
    digitalWrite(ENABLE_3v3_PWR_PIN,LOW); // disabled
}

// *****************************************************
void MEASUREMENTS::units(){
  float refRes;
  String units=" Ohm";
  refRes=this->config.ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE];
  if(this->config.ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE]>10000){
     refRes=this->config.ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE]/1000;
    units="k Ohm";     
  }else if(this->config.ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE]>1000000){
     refRes=this->config.ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE]/1000000;
     units="M Ohm";
  }
  
  this->display->tftPrintText(0,160,(char*)String("Selected ref R:\n"+String(refRes)+ units).c_str(),2,"center", TFT_WHITE, true); 
  this->interface->mserial->printStrln("ADC_REF_RESISTANCE="+String(this->config.ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE])+" Ohm");
  delay(2000);

}

// ************************************************************************
// *********************************************************
//  create new CSV ; delimeted dataset file 
bool MEASUREMENTS::initializeDataMeasurementsFile(){
  if (LittleFS.exists("/" +  this->config.EXPERIMENTAL_DATA_FILENAME)){
    return true;
  }
  // create new CSV ; delimeted dataset file 
  
  File expFile = LittleFS.open("/" + this->config.EXPERIMENTAL_DATA_FILENAME,"w");
  if (expFile){
    this->interface->mserial->printStr("Creating a new dataset CSV file and adding header ...");
    String DataHeader[this->config.NUM_SAMPLE_SAMPLING_READINGS];
    
    DataHeader[0]="Date&Time";
    DataHeader[1]="ANALOG RAW (0-4095)";
    DataHeader[2]="Vref (Volt)";
    DataHeader[3]="V (Volt)";
    DataHeader[4]="R (Ohm)";
    DataHeader[5]="AHT TEMP (*C)";
    DataHeader[6]="AHT Humidity (%)";
    DataHeader[7]="Accel X";
    DataHeader[8]="Accel Y";
    DataHeader[9]="Accel Z";
    DataHeader[10]="Gyro X";
    DataHeader[11]="Gyro Y";
    DataHeader[12]="Gyro Z";
    DataHeader[13]="LSM6DS3 Temp (*C)";
    DataHeader[14]="Bus non-errors";
    DataHeader[15]="Bus Erros";
    DataHeader[16]="Validation";
    DataHeader[17]="CHIP ID";          
    
    String lineRowOfData="";
    for (int j = 0; j < this->config.NUM_SAMPLE_SAMPLING_READINGS; j++) {
      lineRowOfData= lineRowOfData + DataHeader[j] +";";
    }
    expFile.println(lineRowOfData);        
    expFile.close();
    this->interface->mserial->printStrln("header added to the dataset file.");
  }else{
    this->interface->mserial->printStrln("Error creating file(2): " + this->config.EXPERIMENTAL_DATA_FILENAME);
    return false;
  }
  this->interface->mserial->printStrln("");  
  return true;
}

// *********************************************************
bool MEASUREMENTS::saveDataMeasurements(){
  this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_GREEN;
  this->interface->onBoardLED->statusLED(100, 0);

  File expFile = LittleFS.open("/" + this->config.EXPERIMENTAL_DATA_FILENAME,"a+");
  if ( expFile ){
    this->interface->mserial->printStrln("Saving data measurements to the dataset file ...");
    for (int k = 0; k <= measurements_current_pos; k++) {
      for (int i = 0; i < this->config.NUM_SAMPLE_SAMPLING_READINGS; i++) {
        String lineRowOfData="";
        for (int j = 0; j < this->config.NUM_SAMPLE_SAMPLING_READINGS; j++) {
            lineRowOfData= lineRowOfData + measurements[k][i][j] +";";
        }
        lineRowOfData+= macChallengeDataAuthenticity(this->interface, lineRowOfData) + ";" + CryptoICserialNumber(this->interface);
        this->interface->mserial->printStrln(lineRowOfData);
        expFile.println(lineRowOfData);        
      }
    }
    expFile.close();
    delay(500);
    this->interface->mserial->printStrln("collected data measurements stored in the dataset CSV file.("+ this->config.EXPERIMENTAL_DATA_FILENAME +")");
    measurements_current_pos=0;
    this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_GREEN;
    this->interface->onBoardLED->statusLED(100, 0);
    return true;
  }else{
    this->interface->mserial->printStrln("Error creating CSV dataset file(3): " + this->config.EXPERIMENTAL_DATA_FILENAME);
    this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_RED;
    this->interface->onBoardLED->statusLED(100, 2);
    return false;
  }
}
// ******************************************************************************
 void MEASUREMENTS::initSaveDataset(){
    this->interface->mserial->printStrln("starting"); 
    // SAVE DATA MEASUREMENTS ****
    xSemaphoreTake(this->MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      this->datasetFileIsBusySaveData=true;
    xSemaphoreGive(this->MemLockSemaphoreDatasetFileAccess); // exit critical section 
    delay(1000);
    this->saveDataMeasurements();
    xSemaphoreTake(this->MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      this->datasetFileIsBusySaveData=false;
    xSemaphoreGive(this->MemLockSemaphoreDatasetFileAccess); // exit critical section 
  
 }

// ***********************************************************************************
void MEASUREMENTS::runExternalMeasurements(){
  if (this->config.MEASUREMENT_INTERVAL < ( millis() -  this->LAST_DATA_MEASUREMENTS ) ){
    this->LAST_DATA_MEASUREMENTS=millis(); 
    this->interface->mserial->printStrln("");
    this->interface->mserial->printStrln("Reading Specimen electrical response...");

    xSemaphoreTake(this->interface->McuFreqSemaphore, portMAX_DELAY); // enter critical section
      this->interface->McuFrequencyBusy=true;
      this->interface->setMCUclockFrequency(this->interface->SAMPLING_FREQUENCY);
      this->interface->mserial->printStrln("Setting to ADC read CPU Freq = " +String(getCpuFrequencyMhz()));
      this->interface->McuFrequencyBusy=false;
    xSemaphoreGive(this->interface->McuFreqSemaphore); // exit critical section    

    this->readSensorMeasurements();
    
    this->interface->mserial->printStrln("Saving collected data....");
    if(datasetFileIsBusyUploadData){
      this->interface->mserial->printStr("file is busy....");
      this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_RED;
      this->interface->onBoardLED->statusLED(100, 2);
    }
    if(this->measurements_current_pos+1 > this->config.MEASUREMENTS_BUFFER_SIZE){
      this->interface->mserial->printStr("[mandatory wait]");  
      this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_RED;
      this->interface->onBoardLED->statusLED(100, 0);
      while (datasetFileIsBusyUploadData){
      }
      this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_GREEN;
      this->interface->onBoardLED->statusLED(100, 1);
      this->initSaveDataset();
    }else{ // measurements buffer is not full
      long latency=millis();
      long delta= millis()-latency;
      while (datasetFileIsBusyUploadData || (delta < MAX_LATENCY_ALLOWED)){
        delta= millis()-latency;
        delay(500);
      }
      if(datasetFileIsBusyUploadData){
        this->measurements_current_pos++;
        this->interface->mserial->printStr("skipping file save.");  
      }else{
        this->initSaveDataset();
      }       
    }

    xSemaphoreTake(this->interface->McuFreqSemaphore, portMAX_DELAY); // enter critical section
      this->interface->McuFrequencyBusy=true;
      this->interface->setMCUclockFrequency( this->interface->MIN_MCU_FREQUENCY);
      this->interface->mserial->printStrln("Setting to min CPU Freq. = " +String(getCpuFrequencyMhz()));
      this->interface->McuFrequencyBusy=false;
    xSemaphoreGive(this->interface->McuFreqSemaphore); // exit critical section    

    this->scheduleWait=false;
    xSemaphoreTake(this->interface->MemLockSemaphoreCore1, portMAX_DELAY); // enter critical section
      this->waitTimeSensors=0;
    xSemaphoreGive(this->interface->MemLockSemaphoreCore1); // exit critical section
  }

  if (this->scheduleWait){
    this->interface->mserial->printStrln("");
    this->interface->mserial->printStrln("Waiting (schedule) ..");
  }

  if (millis()-lastMillisSensors > 60000){    
    int waitTimeWIFI =0;
    xSemaphoreTake(this->interface->MemLockSemaphoreCore1, portMAX_DELAY); // enter critical section
      lastMillisSensors=millis();
      this->interface->mserial->printStrln("Sensor Acq. in " + String((this->config.MEASUREMENT_INTERVAL/60000)-waitTimeSensors) + " min </\> Upload Experimental Data to a Dataverse Repository in " + String((this->config.UPLOAD_DATASET_DELTA_TIME/60000)-waitTimeWIFI) + " min");
      waitTimeSensors++;
    xSemaphoreGive(this->interface->MemLockSemaphoreCore1); // exit critical section
  }
}

// *************************************************************************
void MEASUREMENTS::readOnboardSensorData(int i, int pos){
    this->onBoardSensors->request_onBoard_Sensor_Measurements();

    this->measurements[i][pos][this->measurements_current_pos]= (char*) String( this->onBoardSensors->aht_temp ).c_str();
    this->measurements[i][pos+1][this->measurements_current_pos]= (char*) String( this->onBoardSensors->aht_humidity ).c_str();

    this->measurements[i][pos+2][this->measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_Motion_X).c_str();  
    this->measurements[i][pos+3][this->measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_Motion_Y).c_str();
    this->measurements[i][pos+4][this->measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_Motion_Z).c_str();

    this->measurements[i][pos+5][this->measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_GYRO_X).c_str();
    this->measurements[i][pos+6][this->measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_GYRO_Y).c_str();  
    this->measurements[i][pos+7][this->measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_GYRO_Z).c_str();

    this->measurements[i][pos+8][this->measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_TEMP).c_str();
    this->measurements[i][pos+9][this->measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_errors).c_str();
}

// ****************************************************************
void MEASUREMENTS::readChannel2SensorMeasurements(int i, int pos){
 // SHT3x sensor
  if(this->config.channel_2_sensor_type == "sht3x"){
    if (this->config.channel_1_switch_on_pos == 0){ // channel 1 is disabled
      this->measurements [i][pos][this->measurements_current_pos]= (char*) this->interface->rtc.getDateTime(true).c_str();
      pos++;
    }
    this->measurements [i][pos][this->measurements_current_pos]= (char*) String(this->sht3x->measurement[0]).c_str(); // Temp
    this->measurements [i][pos][this->measurements_current_pos]= (char*) String(this->sht3x->measurement[1]).c_str();  // Humidity
  }
   
  // AHT20 sensor
  if(this->config.channel_2_sensor_type == "aht20"){
    if (this->config.channel_1_switch_on_pos == 0){ // channel 1 is disabled
      this->measurements [i][pos][this->measurements_current_pos]= (char*) this->interface->rtc.getDateTime(true).c_str();
      pos++;
    }
    this->measurements [i][pos][this->measurements_current_pos]= (char*) String(this->aht20->measurement[0]).c_str(); // Temp
    this->measurements [i][pos][this->measurements_current_pos]= (char*) String(this->aht20->measurement[1]).c_str();  // Humidity
  }

 // other sensors 
 // ...

}
// *****************************************************************************
void MEASUREMENTS::readSensorMeasurements() {
  if(this->config.channel_1_switch_on_pos == 1){ // DS18B20 sensor
    int zerosCount=0;
    for (byte i = 0; i < this->config.NUM_SAMPLE_SAMPLING_READINGS; i++) {
      this->interface->mserial->printStrln("");
      this->interface->mserial->printStr(String(i));
      this->interface->mserial->printStr(": ");

      this->readOnboardSensorData(i, 0); // 10 readings

      this->ds18b20->requestMeasurements(); 
      this->measurements [i][10][this->measurements_current_pos]= (char*) this->interface->rtc.getDateTime(true).c_str();
      this->measurements [i][11][this->measurements_current_pos]= (char*) String(this->ds18b20->measurement[0]).c_str();
      
      this->readChannel2SensorMeasurements(i,12);
      
      delay(this->config.SAMPLING_INTERVAL);
    }
  }else if (this->config.channel_1_switch_on_pos > 1){
    this->readExternalAnalogData();
  }

}

// *********************************************************
void MEASUREMENTS::readExternalAnalogData() {
  float adc_ch_analogRead_raw; 
  float ADC_CH_REF_VOLTAGE; 
  float adc_ch_measured_voltage; 
  float adc_ch_calcukated_e_resistance; 

  float adc_ch_measured_voltage_Sum = 0;
  float adc_ch_calcukated_e_resistance_sum = 0;
  int num_valid_sample_measurements_made = 0;
    
  //this->display->tft.pushImage (10,65,250,87,AEONLABS_16BIT_BITMAP_LOGO);

  this->display->tft.fillRect(10, 65 , 250, 87, TFT_BLACK);
  this->display->tft.pushImage (this->display->TFT_CURRENT_X_RES-75,0,16,18,MEASURE_ICON_16BIT_BITMAP);
  
  // ToDo: Keep ON or turn it off at the end of measurments
  // Enable power to ADC and I2C external plugs
  digitalWrite(this->ENABLE_3v3_PWR_PIN,HIGH); //enable 3v3

  /*
  Set the attenuation for a particular pin
  Default is 11 db
    0	     0 ~ 750 mV     ADC_0db
    2.5	   0 ~ 1050 mV    ADC_2_5db
    6	     0 ~ 1300 mV    ADC_6db
    11	   0 ~ 2500 mV    ADC_11db
  */
  analogSetPinAttenuation(EXT_IO_ANALOG_PIN, ADC_11db);
  analogSetPinAttenuation(VOLTAGE_REF_PIN, ADC_11db);
  float MCU_VDD = 3.30;

  ADC_CH_REF_VOLTAGE = analogRead(VOLTAGE_REF_PIN)/MCU_ADC_DIVIDER * MCU_VDD;

  this->display->tftPrintText(0,25,(char*) String(String(this->config.NUM_SAMPLE_SAMPLING_READINGS)+ " measurements\nsamples requested\n\nSampling interval\n"+String(this->config.SAMPLING_INTERVAL)+" ms\n\nADC CH OUT: "+String(ADC_CH_REF_VOLTAGE)+" Volt").c_str(),2,"left", TFT_WHITE, true); 
  delay(2000);

  int zerosCount=0;
  for (byte i = 0; i < this->config.NUM_SAMPLE_SAMPLING_READINGS; i++) {
    this->interface->mserial->printStrln("");
    this->interface->mserial->printStr(String(i));
    this->interface->mserial->printStr(": ");

    adc_ch_analogRead_raw = analogRead(EXT_IO_ANALOG_PIN);
    num_valid_sample_measurements_made = num_valid_sample_measurements_made + 1;
    // Vref
    ADC_CH_REF_VOLTAGE = analogRead(VOLTAGE_REF_PIN)/MCU_ADC_DIVIDER * MCU_VDD;
    
    // ADC Vin
    adc_ch_measured_voltage = adc_ch_analogRead_raw  * ADC_CH_REF_VOLTAGE / MCU_ADC_DIVIDER;
    
    /*
    How accurate is an Arduino Ohmmeter?
    Gergely Makan, Robert Mingesz and Zoltan Gingl
    Department of Technical Informatics, University of Szeged, Árpád tér 2, 6720, Szeged, Hungary
    https://arxiv.org/ftp/arxiv/papers/1901/1901.03811.pdf
    */
    adc_ch_calcukated_e_resistance = (this->config.ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE] * adc_ch_analogRead_raw ) / (MCU_ADC_DIVIDER- adc_ch_analogRead_raw);


    this->interface->mserial->printStr("ADC Vref raw: ");
    this->interface->mserial->printStrln(String(adc_ch_analogRead_raw));

    this->interface->mserial->printStr("ADC Vref: ");
    this->interface->mserial->printStr(String(ADC_CH_REF_VOLTAGE));
    this->interface->mserial->printStrln(" Volt");

    this->interface->mserial->printStr("ADC CH IN raw: ");
    this->interface->mserial->printStrln(String(adc_ch_analogRead_raw));

    this->interface->mserial->printStr("ADC CH READ: ");
    this->interface->mserial->printStr(String(adc_ch_measured_voltage));
    this->interface->mserial->printStrln(" Volt");
    this->interface->mserial->printStr("Calc. E.R.: ");
    this->interface->mserial->printStr(String(adc_ch_calcukated_e_resistance));
    this->interface->mserial->printStrln("  Ohm");

    this->readOnboardSensorData(i, 0); // 10 readings

    // SAMPLING_READING POS,  SENSOR POS, MEASUREMENTS_BUFFER_ POS
    this->measurements [i][10][this->measurements_current_pos]= (char*) this->interface->rtc.getDateTime(true).c_str();
    this->measurements [i][11][this->measurements_current_pos]= (char*) String(adc_ch_analogRead_raw).c_str();
    this->measurements [i][12][this->measurements_current_pos]= (char*) String(ADC_CH_REF_VOLTAGE).c_str();
    this->measurements [i][13][this->measurements_current_pos]= (char*) String(adc_ch_measured_voltage).c_str();
    this->measurements [i][14][this->measurements_current_pos]= (char*) String(adc_ch_calcukated_e_resistance).c_str();
    
    this->readChannel2SensorMeasurements(i,15);

    delay(this->config.SAMPLING_INTERVAL);

    if (adc_ch_analogRead_raw==0) {
      zerosCount++;
    }else{
      adc_ch_calcukated_e_resistance_sum = adc_ch_calcukated_e_resistance_sum + adc_ch_calcukated_e_resistance;
      adc_ch_measured_voltage_Sum = adc_ch_measured_voltage_Sum + adc_ch_measured_voltage;
    }
  }

  this->display->tft.pushImage (this->display->TFT_CURRENT_X_RES-75,0,16,18,MEASURE_GREY_ICON_16BIT_BITMAP);

  if (zerosCount>0) {
    this->display->tftPrintText(0,25,(char*) String(String(zerosCount)+" zero(s) found\nconsider chg ref. R\nswitch on the DAQ" ).c_str(),2,"left", TFT_WHITE, true); 
    this->interface->mserial->printStrln("Zero value measur. founda: "+String(adc_ch_analogRead_raw));
    this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_RED;
    this->interface->onBoardLED->statusLED(100, 2);
  }

  // ToDo: Keep ON or turn it off at the end of measurments
  // Enable power to ADC and I2C external plugs
  digitalWrite(this->ENABLE_3v3_PWR_PIN,LOW); 
  
  float adc_ch_measured_voltage_avg = adc_ch_measured_voltage_Sum / num_valid_sample_measurements_made;
  float adc_ch_calcukated_e_resistance_avg = adc_ch_calcukated_e_resistance_sum / num_valid_sample_measurements_made;
  String units="Ohm";
  if (adc_ch_calcukated_e_resistance_avg>1000){
    adc_ch_calcukated_e_resistance_avg=adc_ch_calcukated_e_resistance_avg/1000;
    units="kOhm";
  }
  this->display->tftPrintText(0,25,(char*) String("Total data samples: \n"+String(num_valid_sample_measurements_made)+"/"+String(this->config.NUM_SAMPLE_SAMPLING_READINGS)+"\n\n"+"Avg ADC CH volt.:\n"+String(adc_ch_measured_voltage_avg)+" Volt\n\nAverage ADC CH R:\n"+String(adc_ch_calcukated_e_resistance_avg)+" "+units  ).c_str(),2,"left", TFT_WHITE, true); 

  delay(5000);
}

// ******************************************************
bool MEASUREMENTS::initializeDynamicVar(  int size1D, int size2D, int size3D){    
    int i1D = 0; //Variable for looping Row
    int i2D = 0; //Variable for looping column
    int i3D=0; 

    //Init memory allocation
    this->measurements = (char ****)heap_caps_malloc(size1D * sizeof(char***), MALLOC_CAP_SPIRAM);
    
    //Check memory validity
    if(this->measurements == NULL){
       this->interface->mserial->printStrln("Heap men Alloc. FAIL");
      return false;
    }

    //Allocate memory for 1D (rows)
    for (i1D =0 ; i1D < size1D ; i1D++){
        this->measurements[i1D] = (char ***)heap_caps_malloc(size2D * sizeof(char**), MALLOC_CAP_SPIRAM);
        //Check memory validity
        if(this->measurements[i1D] == NULL){
         // this->freeAllocatedMemory(measurements,i1D);
          this->interface->mserial->printStrln("FAIL 1D alloc");
          return false;
        }
    }

    //Allocate memory for 2D (column)
    for (i1D =0 ; i1D < size1D ; i1D++){
      for (i2D =0 ; i2D < size2D ; i2D++){
        this->measurements[i1D][i2D] = (char **)heap_caps_malloc(size3D * sizeof(char*), MALLOC_CAP_SPIRAM);
        //Check memory validity
        if(this->measurements[i1D][i2D] == NULL){
          //this->freeAllocatedMemory(measurements,i1D);
          this->interface->mserial->printStrln("FAIL 2D alloc");
          return false;
        }
      }
    }

    //Allocate memory for 3D (floor)
    for (i1D =0 ; i1D < size1D ; i1D++) {
        for (i2D =0 ; i2D < size2D ; i2D++){
          for (i3D =0 ; i3D < size3D ; i3D++){
            this->measurements[i1D][i2D][i3D] =  (char *)heap_caps_malloc(20 * sizeof(char), MALLOC_CAP_SPIRAM);
            //Check memory validity
            if(this->measurements[i1D][i2D][i3D] == NULL){
              //this->freeAllocatedMemory(measurements,i1D);
              this->interface->mserial->printStrln("FAIL 3D alloc");
              return false;
            }
          }
        }
    }
    return true;   
  } // initializeDynamicVar

  //Free Allocated memory
  void MEASUREMENTS::freeAllocatedMemory(int ****measurements, int nRow, int nColumn, int dim3){
      int iRow = 0;
      int iCol=0;
      int i3D=0; 
      
      for (iRow =0; iRow < nRow; iRow++){
        for (iCol =0 ; iCol < nColumn ; iCol++){
          for (i3D =0 ; i3D < dim3 ; i3D++){
            free(measurements[iRow][iCol][i3D]); // free allocated memory
          }
          free(measurements[iRow][iCol]); // free allocated memory                       
        }
        free(measurements[iRow]); // free allocated memory    
      }
      free(measurements);
  }

// --------------------------------------------------------------------------

bool MEASUREMENTS::saveSettings(fs::FS &fs){
    this->interface->mserial->printStrln( this->interface->DeviceTranslation("save_daq_settings")  + "...");

    if (fs.exists("/measurements.cfg") )
        fs.remove("/measurements.cfg");

    File settingsFile = fs.open("/measurements.cfg", FILE_WRITE); 
    if ( !settingsFile ){
        this->interface->mserial->printStrln( this->interface->DeviceTranslation("err_create_daq_settings") + ".");
        settingsFile.close();
        return false;
    }

    settingsFile.print( String(this->config.MEASUREMENT_INTERVAL) + String(';'));

    settingsFile.close();
    return true;
}
// --------------------------------------------------------------------

bool MEASUREMENTS::readSettings(fs::FS &fs){    
    File settingsFile = fs.open("/measurements.cfg", FILE_READ);
    if (!settingsFile){
        this->interface->mserial->printStrln( this->interface->DeviceTranslation("err_notfound_daq_settings")  + ".");
        settingsFile.close();
        return false;
    }
    if (settingsFile.size() == 0){
        this->interface->mserial->printStrln( this->interface->DeviceTranslation("err_invalid_daq_settings") + ".");
        settingsFile.close();
        return false;    
    }

    String temp= settingsFile.readStringUntil(';');

    this->config.MEASUREMENT_INTERVAL = atol(settingsFile.readStringUntil( ';' ).c_str() ); 

    settingsFile.close();
    return true;
}

// -------------------------------------------------------------------------------

// *********************************************************
// GBRL commands --------------------------------------------------------------
 bool MEASUREMENTS::helpCommands(String $BLE_CMD, uint8_t sendTo){
    if($BLE_CMD != "$?" && $BLE_CMD !="$help" )
        return false;

    String dataStr="Measurements GBRL Commands:\n" \
                    "$view ch1              - View port 1 configuration\n" \
                    "$set sw off            - Set all switch off\n" \
                    "$set sw1 on            - Position 1 on the Switch for the Temperature Sensor\n" \
                    "$set sw2 on            - Position 2 on the Switch: Ohmmeter 1K dvider\n" \
                    "$set sw3 on            - Position 3 on the Switch: Ohmmeter 20K dvider\n" \
                    "$set sw4 on            - Position 4 on the Switch: Ohmmeter 200K dvider\n" \
                    "$set sw5 on            - Position 5 on the Switch: Ohmmeter 2M dvider\n" \
                    "\n" \                           
                    "$view ch2              - View port 2 configuration\n" \
                    "$set ch2 [off/sht3x]   - Connect the selected sensor to port 2 (I2C)\n" \
                    "\n" \                    
                    "$ufid                  - "+ this->interface->DeviceTranslation("ufid") +" unique fingerprint ID\n" \
                    "$me new                - "+ this->interface->DeviceTranslation("me_new") +"\n" \
                    "$me start              - "+ this->interface->DeviceTranslation("me_start") +"\n" \
                    "$me end                - "+ this->interface->DeviceTranslation("me_end") +"\n" \
                    "$me status             - "+ this->interface->DeviceTranslation("me_status") +"\n" \
                    "\n" \        
                    "$history               - "+ this->interface->DeviceTranslation("history") +"\n" \
                    "$ns                    - "+ this->interface->DeviceTranslation("ns") +"\n" \
                    "$mi                    - "+ this->interface->DeviceTranslation("mi") +"\n" \      
                    "$set mi [sec]          - "+ this->interface->DeviceTranslation("set_mi") +"\n\n";

    this->interface->sendBLEstring( dataStr, sendTo);
      
    return false;
 }

// -------------------------------------------------------------------------------


bool MEASUREMENTS::gbrl_commands(String $BLE_CMD, uint8_t sendTo){
    String dataStr="";

    if($BLE_CMD.indexOf("$set ch2")>-1){
      dataStr = "";
      if ($BLE_CMD == "$set ch2 sht3x"){
        dataStr = "A SHT3x sensor is now connected to channel 2.\n";
        this->config.channel_2_sensor_type = "sht3x";
      }
      if ($BLE_CMD == "$set ch2 off"){
        dataStr = "Channel 2 is now disabled.\n";
        this->config.channel_2_sensor_type = "off";
      }
      if (dataStr != ""){
        this->interface->sendBLEstring( dataStr + "\n" , sendTo); 
        this->saveSettings();
        return true;
      }
    }

    if ($BLE_CMD == "$view ch2"){
      dataStr = "Channel 2 current configuration is : " + String(this->config.channel_2_sensor_type) +"\n";
      this->interface->sendBLEstring( dataStr , sendTo); 
      return true;
    }

    if ($BLE_CMD == "$view ch1"){
      dataStr = "Current configuration on the channel 1 switch is :\n";
      if (this->config.channel_1_switch_en){
        dataStr += "Enabled, switch " + String(this->config.channel_1_switch_on_pos) + " is ON all other are set to OFF.\n";
      } else {
        dataStr += "Disabled. (all switches are set to OFF)\n";
      }
      this->interface->sendBLEstring( dataStr + "\n" , sendTo); 
      return true;
    }

    if($BLE_CMD.indexOf("$set sw")>-1){
      return this->sw_commands( $BLE_CMD,  sendTo);
    }

    if ($BLE_CMD == "$ufid"){
      if (this->DATASET_NUM_SAMPLES == 0){
          dataStr = this->interface->DeviceTranslation("no_data_entries") + "." +String(char(10));
          this->interface->sendBLEstring( dataStr, sendTo);
          return true;
      }
      dataStr =  this->interface->DeviceTranslation("calc_ufid") + "..." + this->interface->BaseTranslation("wait_moment")  + "." +String(char(10));
      this->interface->sendBLEstring( dataStr, sendTo);
      
      this->interface->setMCUclockFrequency(this->interface->MAX_FREQUENCY);
      
      dataStr += "Unique Data Fingerprint ID:"+String(char(10));
      dataStr += CryptoICserialNumber(this->interface)+"-"+macChallengeDataAuthenticity(this->interface, String(this->interface->rtc.getDateTime(true)) + String(roundFloat(this->last_measured_probe_temp,2)) );
      dataStr += String(char(10) + String(char(10)) );
      
      this->interface->setMCUclockFrequency(this->interface->CURRENT_CLOCK_FREQUENCY);
      this->interface->sendBLEstring( dataStr, sendTo);
      return true;
    }

    if($BLE_CMD == "$mi"){
      dataStr= this->interface->DeviceTranslation("curr_measure_interval") + " " + String(roundFloat(this->config.MEASUREMENT_INTERVAL/(60*1000) ,2)) + String(" min") + String(char(10));
      this->interface->sendBLEstring( dataStr, sendTo);
      return true;
    }

    if( $BLE_CMD == "$me status"){
      if( this->Measurments_EN == false){
          dataStr = this->interface->DeviceTranslation("measure_not_started") +  String("\n\n");
      } else{
          dataStr = this->interface->DeviceTranslation("measure_already_started") + String("\n");
          dataStr += this->interface->DeviceTranslation("measure_num_records") + " " + String(this->DATASET_NUM_SAMPLES) + "\n\n";
      }

      this->interface->sendBLEstring( dataStr, sendTo); 
      return true;
    }
    if( $BLE_CMD == "$me new"){
        this->Measurments_NEW=true;
        this->Measurments_EN=false;
        this->DATASET_NUM_SAMPLES=0;
        dataStr= this->interface->DeviceTranslation("new_started") +  String("\n\n");
        this->interface->sendBLEstring( dataStr, sendTo); 
        return true;
    }
    if($BLE_CMD == "$me start"){
        if (this->Measurments_EN){
            dataStr = this->interface->DeviceTranslation("measure_already_started_on") +  " " + String(this->measurement_Start_Time) + String("\n\n");
            this->interface->sendBLEstring( dataStr, sendTo); 
        }else{
            this->DATASET_NUM_SAMPLES=0;
            this->Measurments_NEW=true;
            this->Measurments_EN=true;
            this->measurement_Start_Time = this->interface->rtc.getDateTime(true);

            dataStr = this->interface->DeviceTranslation("measure_started_on") +  " " + String(this->measurement_Start_Time) + String("\n");
            this->interface->sendBLEstring( dataStr, sendTo); 
            
            this->gbrl_summary_measurement_config(sendTo);
        }
        return true;

    }
    if( $BLE_CMD=="$me end"){
        if(this->Measurments_EN==false){
            dataStr= this->interface->DeviceTranslation("measure_already_ended")  + String( char(10));
        }else{
            this->Measurments_EN=false;
            dataStr= this->interface->DeviceTranslation("measure_ended_on") +  " " + String(this->interface->rtc.getDateTime(true)) + String( char(10));
        }
        this->interface->sendBLEstring( dataStr, sendTo); 
        return true;
    }
    if($BLE_CMD=="$ns"){
        dataStr = this->interface->DeviceTranslation("num_data_measure") +  ": " + String(this->DATASET_NUM_SAMPLES+1) + String(char(10));
        this->interface->sendBLEstring( dataStr, sendTo);
        return true;
    }
    bool result =false;
    result = this->helpCommands( $BLE_CMD,  sendTo);
    
    bool result2 =false;
    result2 = this->history($BLE_CMD,  sendTo);
    
    bool result3 =false;
    result3 = this->cfg_commands($BLE_CMD,  sendTo);
    
    bool result4 =false;
    result4 = this->measurementInterval($BLE_CMD,  sendTo);    
    
    //this->gbrl_menu_selection();
   
   return ( result || result2 || result3 || result4 );

}

// ****************************************************
bool MEASUREMENTS::sw_commands(String $BLE_CMD, uint8_t sendTo){
    String dataStr="";

    if($BLE_CMD.indexOf("$set sw off")>-1){
      this->config.channel_1_switch_en=false;
      this->config.channel_1_switch_on_pos=0;
      uint8_t SELECTED_ADC_REF_RESISTANCE=0;      
      dataStr =" All channel 1 switches set to OFF postion\n";
    }

    if($BLE_CMD.indexOf("$set sw1 on")>-1){
      this->config.channel_1_switch_en=true;
      this->config.channel_1_switch_on_pos=1;
      dataStr =" position 1 on channel 1 switch set to ON postion. All other are set to OFF\n";
    }
      
    if($BLE_CMD.indexOf("$set sw2 on")>-1){
      this->config.channel_1_switch_en=true;
      this->config.channel_1_switch_on_pos=2;
      this->SELECTED_ADC_REF_RESISTANCE = 0;

      dataStr =" position 2 on on channel 1 switch set to ON postion ("+ addThousandSeparators( std::string( String(this->config.ADC_REF_RESISTANCE[this->SELECTED_ADC_REF_RESISTANCE]).c_str() ) ) +" Ohm). All other are set to OFF\n";
    }

    if($BLE_CMD.indexOf("$set sw3 on")>-1){
      this->config.channel_1_switch_en=true;
      this->config.channel_1_switch_on_pos=3;
      this->SELECTED_ADC_REF_RESISTANCE = 1;
      dataStr =" position 3 on on channel 1 switch set to ON postion ("+ addThousandSeparators( std::string( String(this->config.ADC_REF_RESISTANCE[this->SELECTED_ADC_REF_RESISTANCE] ).c_str()) ) +" Ohm). All other are set to OFF\n";
    }

    if($BLE_CMD.indexOf("$set sw4 on")>-1){
      this->config.channel_1_switch_en=true;
      this->config.channel_1_switch_on_pos=4;
      this->SELECTED_ADC_REF_RESISTANCE = 2;
      dataStr =" position 4 on on channel 1 switch set to ON postion ("+ addThousandSeparators( std::string( String(this->config.ADC_REF_RESISTANCE[this->SELECTED_ADC_REF_RESISTANCE] ).c_str() )) +" Ohm). All other are set to OFF\n";
    }

   if($BLE_CMD.indexOf("$set sw5 on")>-1){
      this->config.channel_1_switch_en=true;
      this->config.channel_1_switch_on_pos=5;
      this->SELECTED_ADC_REF_RESISTANCE = 3;
      dataStr =" position 5 on on channel 1 switch set to ON postion ("+ addThousandSeparators( std::string( String(this->config.ADC_REF_RESISTANCE[this->SELECTED_ADC_REF_RESISTANCE] ).c_str() )) +" Ohm). All other are set to OFF\n";
    }
    
    this->saveSettings();

    this->interface->sendBLEstring( dataStr + "\n" , sendTo); 
    return true;
  
}

// ********************************************************
bool MEASUREMENTS:: gbrl_summary_measurement_config( uint8_t sendTo){
    String dataStr = this->interface->DeviceTranslation("config_summary") +  ":\n";
    dataStr += this->interface->DeviceTranslation("mi_interval") +  ": " + String(this->config.MEASUREMENT_INTERVAL/1000) + " sec.\n\n";
    this->interface->sendBLEstring( dataStr , sendTo); 
    return true;
}


// *******************************************************
bool MEASUREMENTS::cfg_commands(String $BLE_CMD, uint8_t sendTo){
    String dataStr="";
    long int hourT; 
    long int minT; 
    long int secT; 
    long int daysT;
    long int $timedif;

    if($BLE_CMD.indexOf("$cfg mi ")>-1){
        String value= $BLE_CMD.substring(11, $BLE_CMD.length());
        if (isNumeric(value)){
            long int val= (long int) value.toInt();
            if(val>0){
                this->config.MEASUREMENT_INTERVAL=val*1000; // mili seconds 
                this->saveSettings(LittleFS);

                hourT = (long int) ( this->config.MEASUREMENT_INTERVAL/(3600*1000) );
                minT  = (long int) ( this->config.MEASUREMENT_INTERVAL/(60*1000) - (hourT*60));
                secT  = (long int) ( this->config.MEASUREMENT_INTERVAL/1000 - (hourT*3600) - (minT*60));
                daysT = (long int) (hourT/24);
                hourT = (long int) ( (this->config.MEASUREMENT_INTERVAL/(3600*1000) ) - (daysT*24));
                
                dataStr = this->interface->DeviceTranslation("new_mi_accepted") +  "\r\n\n";        
                dataStr += " ["+String(daysT)+"d "+ String(hourT)+"h "+ String(minT)+"m "+ String(secT)+"s "+ String("]\n\n");
                this->interface->sendBLEstring( dataStr, sendTo);
            }else{
                dataStr= this->interface->BaseTranslation("invalid_input") +  "\r\n";
                this->interface->sendBLEstring( dataStr, sendTo);
            }
            return true;
        }
    }
    return false;
}

// *******************************************************
bool MEASUREMENTS::measurementInterval(String $BLE_CMD, uint8_t sendTo){
    if($BLE_CMD !="$MEASURE INTERVAL" && $BLE_CMD !="$measure interval")
        return false;

    long int hourT; 
    long int minT; 
    long int secT; 
    long int daysT;
    long int $timedif;
    String dataStr="";

    hourT = (long int) (this->config.MEASUREMENT_INTERVAL/(3600*1000) );
    minT = (long int) (this->config.MEASUREMENT_INTERVAL/(60*1000) - (hourT*60));
    secT =  (long int) (this->config.MEASUREMENT_INTERVAL/1000 - (hourT*3600) - (minT*60));
    daysT = (long int) (hourT/24);
    hourT = (long int) ((this->config.MEASUREMENT_INTERVAL/(3600*1000) ) - (daysT*24));

    dataStr= this->interface->DeviceTranslation("mi_interval")  + String(char(10)) + String(daysT)+"d "+ String(hourT)+"h "+ String(minT)+"m "+ String(secT)+"s "+ String(char(10));
    this->interface->sendBLEstring( dataStr, sendTo);
    return true;
}


// ********************************************************
bool MEASUREMENTS::history(String $BLE_CMD, uint8_t sendTo){
    if($BLE_CMD != "$history"  )
        return false;

    long int hourT; 
    long int minT; 
    long int secT; 
    long int daysT;
    String dataStr="";
    long int $timedif;
    time_t timeNow;
    time(&timeNow);

    dataStr = this->interface->DeviceTranslation("calc_mi_st_val") +  "..."+ this->interface->BaseTranslation("wait_moment") +"." +String(char(10));
    this->interface->sendBLEstring( dataStr, sendTo);

    this->interface->setMCUclockFrequency(this->interface->MAX_FREQUENCY);

    dataStr = "\n"+ this->interface->DeviceTranslation("data_history") +String(char(10));

    File file = LittleFS.open("/" + this->interface->config.SENSOR_DATA_FILENAME, "r");
    int counter=0; 

    long int sumTimeDelta=0;

    while (file.available()) {
        this->interface->sendBLEstring( "#", sendTo);

        if (counter == 0 ) {
            // raed the header
            String bin2 = file.readStringUntil( char(10) );
            bin2 = file.readStringUntil( char(10) );
        }

        String bin = file.readStringUntil( ';' ); // RTC Date & Time

        long int timeStart = atol(file.readStringUntil( (char) ';' ).c_str() ); //start time of measure 

        long int timeDelta = atol(file.readStringUntil( (char) ';' ).c_str() ); // delta time since last measure
        sumTimeDelta+=timeDelta; //elapsed time since start time of measure

        float temp = (file.readStringUntil( (char) ';' ).toFloat()); // probe temp.

        String rest = file.readStringUntil( char(10) ); // remaider of data string line

        $timedif = (timeStart+sumTimeDelta) - timeStart;
        hourT = (long int) ($timedif / (3600*1000));
        minT = (long int) ($timedif/ (60*1000) - (hourT*60));
        secT =  (long int) ($timedif/1000 - (hourT*3600) - (minT*60));
        daysT = (long int) (hourT/24);
        hourT = (long int) (($timedif/(3600*1000) ) - (daysT*24));

        dataStr +=  ": ["+String(daysT)+"d "+ String(hourT)+"h "+ String(minT)+"m "+ String(secT)+"s "+ String("]  ");    
        dataStr +=  this->interface->DeviceTranslation("probe_temp") + ": ";
        dataStr += String(roundFloat(temp,2))+String(char(176))+String("C  ");

        counter++; 
    }

    file.close();
  
    dataStr += "--------------- \n";
    this->interface->sendBLEstring( dataStr, sendTo);
    this->interface->setMCUclockFrequency( this->interface->CURRENT_CLOCK_FREQUENCY);
  return true;
}

// *********************************************************