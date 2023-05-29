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
#include "manage_mcu_freq.h"
#include "lcd_icons.h"

MEASUREMENTS::MEASUREMENTS() {
    // external 3V3 power
    this->ENABLE_3v3_PWR = 38;
    // Voltage reference
    this->VOLTAGE_REF_PIN = 7;
    // External PWM / Digital IO Pin
    this->EXT_IO_ANALOG_PIN = 6;
    this->SELECTED_ADC_REF_RESISTANCE = 0;
}


//****************************************************************
void MEASUREMENTS::init(INTERFACE_CLASS* interface, DISPLAY_LCD_CLASS* display,M_WIFI_CLASS* mWifi, ONBOARD_SENSORS* onBoardSensors ){
    this->interface=interface;
    this->interface->mserial->printStr("init MEASUREMENTS library ...");
    this->mWifi= mWifi;
    this->display = display;
    this->onBoardSensors =  onBoardSensors;
    
    // ADC Power
    pinMode(ENABLE_3v3_PWR, OUTPUT);

    this->settings_defaults();

    this->interface->mserial->printStrln("Sensor detection completed. Initializing Dynamic memory with:");
    this->interface->mserial->printStrln("Number of SAMPLING READINGS:"+ String(this->config.NUM_SAMPLE_SAMPLING_READINGS));
    this->interface->mserial->printStrln("Number of Sensor Data values per reading:" + String(NUMBER_OF_SENSORS_DATA_VALUES));
    this->interface->mserial->printStrln("PSRAM buffer size:" + String(this->config.MEASUREMENTS_BUFFER_SIZE));
    this->display->tftPrintText(0,160,(char*) String("Buff. size:"+ String(this->config.MEASUREMENTS_BUFFER_SIZE)).c_str(),2,"center", TFT_WHITE, true); 
    delay(2000);

    String units="";
    // 1D number of sample readings ; 2D number of sensor data measuremtns; 3D RAM buffer size
    if ( PSRAMalloc.initializeDynamicVar(this->config.NUM_SAMPLE_SAMPLING_READINGS, (int) NUMBER_OF_SENSORS_DATA_VALUES , this->config.MEASUREMENTS_BUFFER_SIZE) )
    {
      this->interface->mserial->printStrln("Error Initializing Measruments Buffer.");
      this->display->tftPrintText(0,160,"ERR Exp. data Buffer",2,"center", TFT_WHITE, true); 
      this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_RED;
      this->interface->onBoardLED->statusLED(100, 5);
      // TODO : what to do when memeory alloc is NULL

    }else{
        this->interface->mserial->printStrln("Measurements Buffer Initialized successfully.");
        float bufSize= sizeof(char)*(this->config.NUM_SAMPLE_SAMPLING_READINGS*NUMBER_OF_SENSORS_DATA_VALUES*this->config.MEASUREMENTS_BUFFER_SIZE); // bytes
        units=" B";
        if (bufSize>1024){
            bufSize=bufSize/1024;
            units=" Kb";
        }
        this->interface->mserial->printStrln("Buffer size:" + String(bufSize));
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

      // ADC Power
    pinMode(ENABLE_3v3_PWR, OUTPUT);
    digitalWrite(ENABLE_3v3_PWR,LOW); // disabled
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
      changeMcuFreq(interface, this->interface->SAMPLING_FREQUENCY);
      this->interface->mserial->printStrln("Setting to ADC read CPU Freq = " +String(getCpuFrequencyMhz()));
      this->interface->McuFrequencyBusy=false;
    xSemaphoreGive(this->interface->McuFreqSemaphore); // exit critical section    

    this->mWifi->updateInternetTime();
    this->ReadExternalAnalogData();
    
    this->interface->mserial->printStrln("Saving collected data....");
    if(datasetFileIsBusyUploadData){
      this->interface->mserial->printStr("file is busy....");
      this->interface->onBoardLED->led[0] = this->interface->onBoardLED->LED_RED;
      this->interface->onBoardLED->statusLED(100, 2);
    }
    if(measurements_current_pos+1 > this->config.MEASUREMENTS_BUFFER_SIZE){
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
        measurements_current_pos++;
        this->interface->mserial->printStr("skipping file save.");  
      }else{
        this->initSaveDataset();
      }       
    }

    xSemaphoreTake(this->interface->McuFreqSemaphore, portMAX_DELAY); // enter critical section
      this->interface->McuFrequencyBusy=true;
      changeMcuFreq(interface, this->interface->MIN_MCU_FREQUENCY);
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

// *****************************************************************************
void MEASUREMENTS::ReadExternalAnalogData() {

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
  digitalWrite(ENABLE_3v3_PWR,HIGH); //enable 3v3

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

    // SAMPLING_READING POS,  SENSOR POS, MEASUREMENTS_BUFFER_ POS
    measurements [i][0][measurements_current_pos]= (char*) this->interface->rtc.getDateTime(true).c_str();
    measurements [i][1][measurements_current_pos]= (char*) String(adc_ch_analogRead_raw).c_str();
    measurements [i][2][measurements_current_pos]= (char*) String(ADC_CH_REF_VOLTAGE).c_str();
    measurements [i][3][measurements_current_pos]= (char*) String(adc_ch_measured_voltage).c_str();
    measurements [i][4][measurements_current_pos]= (char*) String(adc_ch_calcukated_e_resistance).c_str();
    

    this->onBoardSensors->request_onBoard_Sensor_Measurements();

    measurements[i][5][measurements_current_pos]= (char*) String( this->onBoardSensors->aht_temp ).c_str();
    measurements[i][6][measurements_current_pos]= (char*) String( this->onBoardSensors->aht_humidity ).c_str();

    measurements[i][7][measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_Motion_X).c_str();  
    measurements[i][8][measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_Motion_Y).c_str();
    measurements[i][9][measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_Motion_Z).c_str();

    measurements[i][10][measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_GYRO_X).c_str();
    measurements[i][11][measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_GYRO_Y).c_str();  
    measurements[i][12][measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_GYRO_Z).c_str();

    measurements[i][13][measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_TEMP).c_str();
    measurements[i][14][measurements_current_pos]= (char*) String(this->onBoardSensors->LSM6DS3_errors).c_str();

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
  digitalWrite(ENABLE_3v3_PWR,LOW); 
  
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

// GBRL commands --------------------------------------------------------------
 bool MEASUREMENTS::helpCommands(uint8_t sendTo ){
    String dataStr="GBRL commands:\n" \
                    "$help $?                           - View available GBRL commands\n" \
                    "$dt                                - Device Time\n" \
                    "$lang set [country code]           - Change the smart device language\n\n";

    this->interface->sendBLEstring( dataStr,  sendTo ); 
    return false; 
 }
// ******************************************************************************************

bool MEASUREMENTS::commands(String $BLE_CMD, uint8_t sendTo ){
  String dataStr="";

  if($BLE_CMD.indexOf("$lang dw ")>-1){

  }
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
       this->interface->mserial->("Heap men Alloc. FAIL");
      return false;
    }

    //Allocate memory for 1D (rows)
    for (i1D =0 ; i1D < size1D ; i1D++){
        this->measurements[i1D] = (char ***)heap_caps_malloc(size2D * sizeof(char**), MALLOC_CAP_SPIRAM);
        //Check memory validity
        if(this->measurements[i1D] == NULL){
         // this->freeAllocatedMemory(measurements,i1D);
          this->interface->mserial->println("FAIL 1D alloc");
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
          this->interface->mserial->("FAIL 2D alloc");
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
              this->interface->mserial->("FAIL 3D alloc");
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

// END class PSRAMallocClass   ************************************************************
// *********************************************************
