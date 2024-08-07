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

#define uS_TO_S_FACTOR 1000000

//----------------------------------------------------------------------------------------
// Components Testing  **************************************
bool SCAN_I2C_BUS = true;
bool TEST_FINGERPRINT_ID_IC = true;

//----------------------------------------------------------------------------------
#include <math.h>
#include <cmath>
#include "SPI.h"
#include <semphr.h>

#include "esp32-hal-psram.h"
// #include "rom/cache.h"
extern "C"
{
#include <esp_himem.h>
#include <esp_spiram.h>
}

// custom includes **********************************

#include "nvs_flash.h"  //preferences lib

// External sensor moeasurements
#include "measurements.h"
MEASUREMENTS* measurements = new MEASUREMENTS();

// custom functions
#include "m_file_functions.h"

// Interface class ******************************
#include "interface_class.h"
INTERFACE_CLASS* interface = new INTERFACE_CLASS();
#define DEVICE_NAME "Smart 12-Bit Data Acquisition Device"

// GBRL commands  ***************************
#include "gbrl.h"
GBRL gbrl = GBRL();

// DATAVERSE **********************************
#include "dataverse.h"
DATAVERSE_CLASS* dataverse = new DATAVERSE_CLASS();

// Onboard sensors  *******************************
#include "onboard_sensors.h"
ONBOARD_SENSORS* onBoardSensors = new ONBOARD_SENSORS();

// unique figerprint data ID
#include "m_atsha204.h"
#include <Wire.h>

// serial comm
#include <HardwareSerial.h>
HardwareSerial UARTserial(0);

#include "mserial.h"
mSerial* mserial = new mSerial(true, &UARTserial);

// File class
#include <esp_partition.h>
#include "FS.h"
#include <LittleFS.h>
#include "m_file_class.h"

FILE_CLASS* drive = new FILE_CLASS(mserial);

// WIFI Class
#include <ESP32Ping.h>
#include "m_wifi.h"
M_WIFI_CLASS* mWifi = new M_WIFI_CLASS();


// Certificates
#include "cert/github_cert.h"

// LCD display
#include "m_display_lcd.h"
DISPLAY_LCD_CLASS* display = new DISPLAY_LCD_CLASS();

/********************************************************************/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLECharacteristic *pCharacteristicTX, *pCharacteristicRX;
BLEServer *pServer;
BLEService *pService;


bool BLE_advertise_Started = false;

bool newValueToSend = false;
String $BLE_CMD = "";
bool newBLESerialCommandArrived = false;
SemaphoreHandle_t MemLockSemaphoreBLE_RX = xSemaphoreCreateMutex();

float txValue = 0;
String valueReceived = "";


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      mWifi->setBLEconnectivityStatus (true);
      mserial->printStr("BLE connection init ", mserial->DEBUG_BOTH_USB_UART);

      interface->onBoardLED->led[0] = interface->onBoardLED->LED_BLUE;
      interface->onBoardLED->statusLED(100, 1);

      String dataStr = "Connected to the Smart Concrete Maturity device (" + String(interface->firmware_version) + ")" + String(char(10)) + String(char(13)) + "Type $? or $help to see a list of available commands" + String(char(10));
      dataStr += String(interface->rtc.getDateTime(true)) + String(char(10)) + String(char(10));

      if (mWifi->getNumberWIFIconfigured() == 0 ) {
        dataStr += "no WiFi Networks Configured" + String(char(10)) + String(char(10));
      }
      //interface->sendBLEstring(dataStr, mserial->DEBUG_TO_BLE);
    }

    void onDisconnect(BLEServer* pServer) {
      mWifi->setBLEconnectivityStatus (false);

      interface->onBoardLED->led[0] = interface->onBoardLED->LED_BLUE;
      interface->onBoardLED->statusLED(100, 0.5);
      interface->onBoardLED->led[0] = interface->onBoardLED->LED_RED;
      interface->onBoardLED->statusLED(100, 0.5);
      interface->onBoardLED->led[0] = interface->onBoardLED->LED_BLUE;
      interface->onBoardLED->statusLED(100, 0.5);

      pServer->getAdvertising()->start();
    }
};

class pCharacteristicTX_Callbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String txValue = String(pCharacteristic->getValue().c_str());
      txValue.trim();
      mserial->printStrln("Transmitted TX Value: " + String(txValue.c_str()) , mserial->DEBUG_BOTH_USB_UART);

      if (txValue.length() == 0) {
        mserial->printStr("Transmitted TX Value: empty ", mserial->DEBUG_BOTH_USB_UART);
      }
    }

    void onRead(BLECharacteristic *pCharacteristic) {
      mserial->printStr("TX onRead...", mserial->DEBUG_BOTH_USB_UART);
      //pCharacteristic->setValue("OK");
    }
};

class pCharacteristicRX_Callbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      delay(10);
      String rxValue;
      int len = strlen(pCharacteristic->getValue().c_str() );
      const char* chrArr;
      chrArr = pCharacteristic->getValue().c_str();

      for (int i=0; i<len; i++ ){
        if(isdigit(chrArr[i])==false && isalpha(chrArr[i])==false ){
          mserial->printStrln("Error receiving Serial BLE command");
          return;      
        }
      }
      rxValue = String(pCharacteristic->getValue().c_str());      
      rxValue.trim();
      mserial->printStrln("Received RX Value: " + String(rxValue.c_str()), mserial->DEBUG_BOTH_USB_UART );

      if (rxValue.length() == 0) {
        mserial->printStr("Received RX Value: empty " , mserial->DEBUG_BOTH_USB_UART);
      }

      $BLE_CMD = rxValue;
      mWifi->setBLEconnectivityStatus(true);

      xSemaphoreTake(MemLockSemaphoreBLE_RX, portMAX_DELAY);
      newBLESerialCommandArrived = true; // this needs to be the last line
      xSemaphoreGive(MemLockSemaphoreBLE_RX);

      delay(50);
    }

    void onRead(BLECharacteristic *pCharacteristic) {
      mserial->printStr("RX onRead..." , mserial->DEBUG_BOTH_USB_UART);
      //pCharacteristic->setValue("OK");
    }

};

// ********************************************************
// *************************  == SETUP == *****************
// ********************************************************
//variaveis que indicam o núcleo
static uint8_t taskCoreZero = 0;
static uint8_t taskCoreOne  = 1;

long int prevMeasurementMillis;

void setup() {
  // ......................................................................................................
  // .......................... START OF IO & PIN CONFIGURATION..............................................
  // ......................................................................................................
  
  // I2C IOs  __________________________
  interface->I2C_SDA_IO_PIN = 8;
  interface->I2C_SCL_IO_PIN = 9;
/*
  i2c_config_t conf;

  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = interface->I2C_SDA_IO_PIN;
  conf.scl_io_num = interface->I2C_SCL_IO_PIN;
  conf.sda_pullup_en = true;
  conf.scl_pullup_en = true;
  conf.master.clk_speed = 400000;
  conf.clk_flags = 0;

  i2c_param_config(I2C_NUM_0, &conf);

  i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    */
    
    
  // Firmware Build Version / revision ______________________________
  interface->firmware_version = "1.0.0";

  // Serial Communication Init ______________________________
  interface->UARTserial = &UARTserial;
  mserial->DEBUG_TO = mserial->DEBUG_TO_UART;
  mserial->DEBUG_EN = true;
  mserial->DEBUG_TYPE = mserial->DEBUG_TYPE_VERBOSE; // DEBUG_TYPE_INFO;

  // Power Saving ____________________________________
  interface->LIGHT_SLEEP_EN = false;

  // External Ports IO Pin assignment ________________________________
  interface-> EXT_PLUG_PWR_EN = 33;
  
  measurements->ENABLE_3v3_PWR_PIN = 2;
  measurements->EXT_IO_ANALOG_PIN = 6;
  measurements->VOLTAGE_REF_PIN = 7;

  pinMode(interface->EXT_PLUG_PWR_EN, OUTPUT);
  digitalWrite(interface->EXT_PLUG_PWR_EN , HIGH);

  // Battery Power Monitor ___________________________________
  interface->BATTERY_ADC_IO = 8;
  pinMode(interface->BATTERY_ADC_IO, INPUT);

  // ________________ Onboard LED  _____________
  interface->onBoardLED = new ONBOARD_LED_CLASS();
  interface->onBoardLED->LED_RED = 24;
  interface->onBoardLED->LED_BLUE = 13;
  interface->onBoardLED->LED_GREEN = 14;

  interface->onBoardLED->LED_RED_CH = 7;
  interface->onBoardLED->LED_BLUE_CH = 2;
  interface->onBoardLED->LED_GREEN_CH = 3;

  // ___________ MCU freq ____________________
  interface-> SAMPLING_FREQUENCY = 240;
  interface-> WIFI_FREQUENCY = 80; // min WIFI MCU Freq is 80-240
  interface->MIN_MCU_FREQUENCY = 10;
  interface-> SERIAL_DEFAULT_SPEED = 115200;

  // ___________  LCD Display ____________________
  display->LCD_BACKLIT_LED = 9;
  display->LCD_DISABLED = true;

  // ......................................................................................................
  // .......................... END OF IO & PIN CONFIGURATION..............................................
  // ......................................................................................................
  bool result = Wire.begin( interface->I2C_SDA_IO_PIN , interface->I2C_SCL_IO_PIN, 100000 );
  
  ESP_ERROR_CHECK(nvs_flash_erase());
  nvs_flash_init();

  mserial->start(115200);
  if ( result ){
    mserial->printStrln("I2C Bus started at SDA IO " + String(interface->I2C_SDA_IO_PIN) + " SCL IO " + String(interface->I2C_SCL_IO_PIN) );
  }else{
    mserial->printStrln("Error starting I2C Bus");  
  }
  delay(3000);
  
  interface->onBoardLED->init();

  interface->onBoardLED->led[0] = interface->onBoardLED->LED_RED;
  interface->onBoardLED->statusLED(100, 0);

  //init storage drive ___________________________
  drive->partition_info();
  if (drive->init(LittleFS, "storage", 2, mserial,   interface->onBoardLED ) == false)
    while (1);

  //init interface ___________________________
  interface->init(mserial, true); // debug EN ON
  interface->settings_defaults();

  if ( !interface->loadSettings() ) {
    interface->onBoardLED->led[0] = interface->onBoardLED->LED_RED;
    interface->onBoardLED->led[0] = interface->onBoardLED->LED_GREEN;
    interface->onBoardLED->statusLED(100, 2);
  }
  
  // init onboard sensors ___________________________
  onBoardSensors->init(interface, mserial);
  if (SCAN_I2C_BUS) {
    onBoardSensors->I2Cscanner();
  }
  onBoardSensors->initOnboardSensors();
  
  //init display ___________________________
  display->init(interface, mWifi);

  if (TEST_FINGERPRINT_ID_IC) {
    delay(500);
    runFingerPrintIDtests(interface);
  }

  mserial->printStrln("\nMicrocontroller specifications:");
  interface->CURRENT_CLOCK_FREQUENCY = getCpuFrequencyMhz();
  mserial->printStr("Internal Clock Freq = ");
  mserial->printStr(String(interface->CURRENT_CLOCK_FREQUENCY));
  mserial->printStrln(" MHz");

  interface->Freq = getXtalFrequencyMhz();
  mserial->printStr("XTAL Freq = ");
  mserial->printStr(String(interface->Freq));
  mserial->printStrln(" MHz");

  interface->Freq = getApbFrequency();
  mserial->printStr("APB Freq = ");
  mserial->printStr(String(interface->Freq / 1000000));
  mserial->printStrln(" MHz\n");

  interface->setMCUclockFrequency( interface->WIFI_FREQUENCY);
  mserial->printStrln("setting Boot MCU Freq to " + String(getCpuFrequencyMhz()) +"MHz");
  mserial->printStrln("");

  // init BLE
  BLE_init();

  //init wifi
  mWifi->init(interface, drive, interface->onBoardLED);
  mWifi->OTA_FIRMWARE_SERVER_URL = "https://github.com/aeonSolutions/openScience-Smart-DAQ-to-Upload-Live-Experimental-Data-to-a-Data-Repository/releases/download/openFirmware/firmware.bin";
  
  mWifi->add_wifi_network("TheScientist","angelaalmeidasantossilva");
  mWifi->ALWAYS_ON_WIFI = true;

  mWifi->WIFIscanNetworks();

  // check for firmwate update
  mWifi->startFirmwareUpdate();
  
  mserial->printStrln( "\nRequesting Geo Location FingerPrint  =====================================" );
  mWifi->get_ip_geo_location_data("", true);
  
  ESP32Time rtc(0);
  rtc.setTime( mWifi->requestGeoLocationDateTime );

  mserial->printStrln("Internet IP       : " + mWifi->InternetIPaddress );
  mserial->printStrln("Geo Location Time : " + String( rtc.getDateTime(true) ) );
  if ( mWifi->geoLocationInfoJson.isNull() == false ){
    float lat =0.0f;
    float lon = 0.0f;

    if( mWifi->geoLocationInfoJson.containsKey("lat")){
      lat = mWifi->geoLocationInfoJson["lat"];
      mserial->printStr( "Latitude : " + String(lat,4) );
    }else{
      mserial->printStr( "Latitude : - -" );
    }

    if( mWifi->geoLocationInfoJson.containsKey("lon")){
      lon = mWifi->geoLocationInfoJson["lon"];
      mserial->printStr( "  Longitude: " + String(lon,4) );
    }else{
      mserial->printStr( "  Longitude: - -" );
    }
    
    if( mWifi->geoLocationInfoJson.containsKey("regionName")){
      String regionName = mWifi->geoLocationInfoJson["regionName"];
      mserial->printStr( "\nLocation: " + regionName );
    }else{
      mserial->printStr( "\nLocation: - -" );
    }

    if( mWifi->geoLocationInfoJson.containsKey("country")){
      String country = mWifi->geoLocationInfoJson["country"];
      mserial->printStrln( ", " + country );
    }else{
      mserial->printStrln( " , - -" );
    }

    mserial->printStrln( "\nGEO Location FingerPrint is : " );
    String geoFingerprint = String( interface->rtc.getEpoch()  ) + "-" + String(mWifi->requestGeoLocationDateTime) + "-" + mWifi->InternetIPaddress + "-" + String(lat) + "-" + String(lon);
    mserial->printStrln( geoFingerprint + "\nGEO Location FingerPrint is ID: " );
    geoFingerprint = macChallengeDataAuthenticity( interface, geoFingerprint );
    mserial->printStrln( geoFingerprint );
    mserial->printStrln("Offline response is:\n" + macChallengeDataAuthenticityOffLine(interface, (char*) geoFingerprint.c_str() ) );

  }
  mserial->printStrln( "\n ====================== done =======================" );
  
  interface->onBoardLED->led[0] = interface->onBoardLED->LED_RED;
  interface->onBoardLED->statusLED(100, 0);
  
  //init measurements
  measurements->init(interface, display, mWifi, onBoardSensors);
  
  // init dataverse
  dataverse->init(interface, mWifi, measurements);

  //Init GBRL
  gbrl.init(interface, mWifi);

  // calibrate motion sensor to detect shaking
  onBoardSensors->initRollTheshold();
 
  interface->onBoardLED->led[0] = interface->onBoardLED->LED_BLUE;
  interface->onBoardLED->statusLED(100, 0);

  interface->$espunixtimePrev = millis();
  interface->$espunixtimeStartMeasure = millis();

  mWifi->$espunixtimeDeviceDisconnected = millis();

  prevMeasurementMillis = millis();
  
  mserial->printStr("Starting MCU cores... ");
/*
  xTaskCreatePinnedToCore (
    loop2,     // Function to implement the task
    "loop2",   // Name of the task
    1000,      // Stack size in bytes
    NULL,      // Task input parameter
    0,         // Priority of the task
    NULL,      // Task handle.
    0          // Core where the task should run
  );
*/
  MemLockSemaphoreBLE_RX = xSemaphoreCreateMutex();
  mserial->printStrln("done. ");

  mserial->printStrln("Free memory: " + addThousandSeparators( std::string( String(esp_get_free_heap_size() ).c_str() ) ) + " bytes");
  
  mserial->printStrln("============================================================================");
  mserial->printStrln("Setup is completed. You may start using the " + String(DEVICE_NAME) );
  mserial->printStrln("Type $? for a List of commands.");
  mserial->printStrln("============================================================================\n");

  interface->onBoardLED->led[0] = interface->onBoardLED->LED_GREEN;
  interface->onBoardLED->statusLED(100, 1);

}
// ********END SETUP *********************************************************

void GBRLcommands(String command, uint8_t sendTo) {
  if (gbrl.commands(command, sendTo) == false) {
    if ( onBoardSensors->gbrl_commands(command, sendTo ) == false) {
      if (mWifi->gbrl_commands(command, sendTo ) == false) {
        if ( measurements->gbrl_commands(command, sendTo ) == false) {
          if ( dataverse->gbrl_commands(command, sendTo ) == false) {
            if ( command.indexOf("$") > -1) {
              interface->sendBLEstring("$ CMD ERROR \r\n", sendTo);
            } else {
              // interface->sendBLEstring("$ CMD UNK \r\n", sendTo);
            }
          }
        }
      }
    }
  }
}
// ************************************************************

void BLE_init() {
  // Create the BLE Device
  BLEDevice::init(String("LDAD " + interface->config.DEVICE_BLE_NAME).c_str());  // max 29 chars

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic

  pCharacteristicTX = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );


  pCharacteristicTX->addDescriptor(new BLE2902());

  pCharacteristicRX = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_RX,
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_WRITE  |
                        BLECharacteristic::PROPERTY_NOTIFY |
                        BLECharacteristic::PROPERTY_INDICATE
                      );

  pCharacteristicTX->setCallbacks(new pCharacteristicTX_Callbacks());
  pCharacteristicRX->setCallbacks(new pCharacteristicRX_Callbacks());

  interface->init_BLE(pCharacteristicTX);

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}
// *******************************************************************************************

//******************************* ==  LOOP == ******************************************************

unsigned long lastMillisWIFI = 0;
int waitTimeWIFI = 0;

String dataStr = "";
long int eTime;
long int statusTime = millis();
long int beacon = millis();
// adc_power_release()

unsigned int cycle = 0;
uint8_t updateCycle = 0;


// ********************* == Core 1 : Data Measurements Acquisition == ******************************
void loop2 (void* pvParameters) {
  //measurements->runExternalMeasurements();
  delay(2000);
}


//************************** == Core 2: Connectivity WIFI & BLE == ***********************************************************
void loop(){  
  if (millis() - beacon > 60000) {    
    beacon = millis();
    mserial->printStrln("(" + String(beacon) + ") Free memory: " + addThousandSeparators( std::string( String(esp_get_free_heap_size() ).c_str() ) ) + " bytes\n", mSerial::DEBUG_TYPE_VERBOSE, mSerial::DEBUG_ALL_USB_UART_BLE);
  }

  if ( (millis() - statusTime > 10000)) { //10 sec
    statusTime = millis();
    interface->onBoardLED->led[1] = interface->onBoardLED->LED_GREEN;
    interface->onBoardLED->statusLED(100, 0.04);
  } else if  (millis() - statusTime > 10000) {
    statusTime = millis();
    interface->onBoardLED->led[1] = interface->onBoardLED->LED_RED;
    interface->onBoardLED->statusLED(100, 0.04);
  }
  
  // .............................................................................
  // disconnected for at least 3min
  // change MCU freq to min
  if (  mWifi->getBLEconnectivityStatus() == false && ( millis() - mWifi->$espunixtimeDeviceDisconnected > 180000) && interface->CURRENT_CLOCK_FREQUENCY >= interface->WIFI_FREQUENCY) {
    mserial->printStrln("setting min MCU freq.");
    btStop();
    //BLEDevice::deinit(); // crashes the device

    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_MODE_NULL);

    interface->setMCUclockFrequency(interface->MIN_MCU_FREQUENCY);
    mWifi->setBLEconnectivityStatus(false);

    interface->onBoardLED->led[0] = interface->onBoardLED->LED_RED;
    interface->onBoardLED->led[1] = interface->onBoardLED->LED_GREEN;
    interface->onBoardLED->statusLED(100, 2);
  }

  // ................................................................................
  // Ligth Sleeep
  eTime = millis() - prevMeasurementMillis;
  if ( mWifi->getBLEconnectivityStatus() == false && interface->LIGHT_SLEEP_EN) {
    mserial->printStr("Entering light sleep....");
    interface->onBoardLED->turnOffAllStatusLED();

    esp_sleep_enable_timer_wakeup( ( (measurements->config.MEASUREMENT_INTERVAL - eTime) / 1000)  * uS_TO_S_FACTOR);
    delay(100);
    esp_light_sleep_start();
    mserial->printStrln("wake up done.");
  }
  prevMeasurementMillis = millis();


  // ....................................
  measurements->runExternalMeasurements();
  
  // ....................................................................................
  // Dataverse
  dataverse->syncronizeToDataverse();

// ................................................................................    

  if (mserial->readSerialData()){
    GBRLcommands(mserial->serialDataReceived, mserial->DEBUG_TO_USB);
    mserial->printStrln("<");
  }
 // ................................................................................    

  if (mserial->readUARTserialData()){
    GBRLcommands(mserial->serialUartDataReceived, mserial->DEBUG_TO_UART);
    mserial->printStrln("<");
  }
// ................................................................................

  if (newBLESerialCommandArrived){
    xSemaphoreTake(MemLockSemaphoreBLE_RX, portMAX_DELAY); 
      newBLESerialCommandArrived=false; // this needs to be the last line       
    xSemaphoreGive(MemLockSemaphoreBLE_RX);

    GBRLcommands($BLE_CMD, mserial->DEBUG_TO_BLE);
    mserial->printStrln("<");
  }

  // ....................................................................................
  // OTA Firmware
  if ( mWifi->forceFirmwareUpdate == true )
    mWifi->startFirmwareUpdate();

// ---------------------------------------------------------------------------
  if (millis() - lastMillisWIFI > 60000) {
    xSemaphoreTake(interface->MemLockSemaphoreCore2, portMAX_DELAY);
    waitTimeWIFI++;
    lastMillisWIFI = millis();
    xSemaphoreGive(interface->MemLockSemaphoreCore2);
  }

}
// -----------------------------------------------------------------
