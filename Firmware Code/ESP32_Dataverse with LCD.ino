/* Firmware version 0.5
 *  ToDo:
 *    - Calc MD5 hash of the dataset file yo compare with json result on upload 
 *    - OTA firmware update
 *    - Remove lock on a dataset (by an admin)
 *    - Validate Json received on a new dataset upload
 *    - Load dataset repository metadata
 *    - Output board startup diagnostics serial stream to a Bluetooth or WIFI data strean
 *    - accecpt GBRL like setup and config $ commands.
 *    - SQLite dataset 
*/

#include "Wire.h"
#include "SparkFunLSM6DS3.h"
#include "SPI.h"
#include <SHT31.h>

#ifndef _FFAT_H_
  #include <FFat.h>
#endif
#ifndef FS_H
  #include "FS.h"
#endif

#include "nvs_flash.h"  //preferences lib

#include "WiFiClientSecure.h"
#include <ESP32Ping.h>

#include "sha204_i2c.h"

#include <semphr.h>

#include "time.h"
#include "ESP32Time.h"

#include <ArduinoJson.h>


#include <TFT_eSPI.h>                                            
// AeonLabs Logo 
#include "aeonlabs_science_img.h"
#include "wifi_icon.h"
TFT_eSPI tft = TFT_eSPI();    

#include <Arduino.h>
#include <Adafruit_I2CDevice.h>

#include "esp32-hal-psram.h"
// #include "rom/cache.h"
extern "C" 
{
#include <esp_himem.h>
#include <esp_spiram.h>
}


// WIFI SETUP ********************************************
const char* WIFI_SSID = "TheScientist";
const char* WIFI_PASSWORD = "angelaalmeidasantossilva";
bool WIFIconnected=false;
WiFiClientSecure client;
#define HTTP_TTL 20000 // 20 sec TTL
bool WIFI_IS_ENABLED = false;


// RTC SETUP *******************
//ESP32Time rtc;
ESP32Time rtc(3600);  // offset in seconds GMT+1


// RTC: NTP server ***********************
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0; // 3600 for 1h difference
const int   daylightOffset_sec = 3600;
struct tm timeinfo;
long NTP_request_interval=64000;// 64 sec.
long NTP_last_request=0;
  
// DATAVERSE **********************************
String API_TOKEN = "a85f5973-34dc-4133-a32b-c70dfef9d001";
String SERVER_URL = "dataverse.harvard.edu";
int SERVER_PORT = 443;
String PERSISTENT_ID = "doi:10.7910/DVN/ZWOLNI"; 
String DATASET_ID = "6552890";
String DATASET_REPOSITORY_URL;
  
const static char* HARVARD_ROOT_CA_RSA_SHA1 PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb\n" \
"MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\n" \
"GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj\n" \
"YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL\n" \
"MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n" \
"BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM\n" \
"GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP\n" \
"ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua\n" \
"BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe\n" \
"3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4\n" \
"YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR\n" \
"rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm\n" \
"ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU\n" \
"oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v\n" \
"QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t\n" \
"b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF\n" \
"AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q\n" \
"GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz\n" \
"Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2\n" \
"G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi\n" \
"l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3\n" \
"smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==\n" \
"-----END CERTIFICATE-----\n";

// JSON 
struct SpiRamAllocator {
  void* allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void* pointer) {
    heap_caps_free(pointer);
  }

  void* reallocate(void* ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;
SpiRamJsonDocument datasetInfoJson(2*2463+ 14*JSON_ARRAY_SIZE(1) + 122*JSON_OBJECT_SIZE(1));


// Measurements: Live Data Acquisition  ******************************
union Data {
   int i;
   float f;
   char chr[20];
};  
 
// Measurements: RAM Storage of Live Data  ******************************
// array size is the number of sensors to do data collection
#define NUM_SAMPLE_SAMPLING_READINGS 16
#define SAMPLING_INTERVAL 0
#define MEASUREMENTS_BUFFER_SIZE 10
uint8_t NUMBER_OF_SENSORS_DATA_VALUES =5; // time + 4 ADC

char ****measurements = NULL; //pointer to pointer
int measurements_current_pos=0;

// Measurements: Planning / Schedule  **********************************
unsigned long UPLOAD_DATASET_DELTA_TIME= NUM_SAMPLE_SAMPLING_READINGS*SAMPLING_INTERVAL + 120000; // 10 min
unsigned long DO_DATA_MEASURMENTS_DELTA_TIME= NUM_SAMPLE_SAMPLING_READINGS*SAMPLING_INTERVAL + 00000; // 1 min
unsigned long LAST_DATASET_UPLOAD = 0;
unsigned long LAST_DATA_MEASUREMENTS = 0;
unsigned long MAX_LATENCY_ALLOWED = (unsigned long)(DO_DATA_MEASURMENTS_DELTA_TIME/2);


// **************************** == PSRAM Memory Class == ************************
class PSRAMallocClass {
  private:

  public:
    bool result=false;
    PSRAMallocClass() {

    }

  char**** initializeDynamicVar( int size1D, int size2D, int size3D){    
    int i1D = 0; //Variable for looping Row
    int i2D = 0; //Variable for looping column
    int i3D=0; 

    //Init memory allocation
    measurements = (char ****)heap_caps_malloc(size1D * sizeof(char***), MALLOC_CAP_SPIRAM);
    
    //Check memory validity
    if(measurements == NULL){
       Serial.println("FAIL");
       this->result=false;
      return NULL;
    }
    //Allocate memory for 1D (rows)
    for (i1D =0 ; i1D < size1D ; i1D++){
        measurements[i1D] = (char ***)heap_caps_malloc(size2D * sizeof(char**), MALLOC_CAP_SPIRAM);
        //Check memory validity
        if(measurements[i1D] == NULL){
         // this->freeAllocatedMemory(measurements,i1D);
          Serial.println("FAIL 1D alloc");
          this->result=false;
          return NULL;
        }
    }

    //Allocate memory for 2D (column)
    for (i1D =0 ; i1D < size1D ; i1D++){
      for (i2D =0 ; i2D < size2D ; i2D++){
        measurements[i1D][i2D] = (char **)heap_caps_malloc(size3D * sizeof(char*), MALLOC_CAP_SPIRAM);
        //Check memory validity
        if(measurements[i1D][i2D] == NULL){
          //this->freeAllocatedMemory(measurements,i1D);
          Serial.println("FAIL 2D alloc");
          this->result=false;
          return NULL;
        }
      }
    }

    //Allocate memory for 3D (floor)
    for (i1D =0 ; i1D < size1D ; i1D++) {
        for (i2D =0 ; i2D < size2D ; i2D++){
          for (i3D =0 ; i3D < size3D ; i3D++){
            measurements[i1D][i2D][i3D] =  (char *)heap_caps_malloc(20 * sizeof(char), MALLOC_CAP_SPIRAM);
            //Check memory validity
            if(measurements[i1D][i2D][i3D] == NULL){
              //this->freeAllocatedMemory(measurements,i1D);
              Serial.println("FAIL 3D alloc");
              this->result=false;
              return NULL;
            }
          }
        }
    }
    this->result=true;   
    return measurements;
  } // initializeDynamicVar

  //Free Allocated memory
  void freeAllocatedMemory(int ****measurements, int nRow, int nColumn, int dim3){
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

}; // END class PSRAMallocClass   

// class initialization
PSRAMallocClass PSRAMalloc= PSRAMallocClass();



// File management *********************************
File EXPERIMENTAL_DATA_FILE;
String EXPERIMENTAL_DATA_FILENAME = "ER_measurements.csv";

#define FORMAT_FATFS_IF_FAILED true


// MCU Clock Management ******************************

//function takes the following frequencies as valid values:
//  240, 160, 80    <<< For all XTAL types
//  40, 20, 10      <<< For 40MHz XTAL
//  26, 13          <<< For 26MHz XTAL
//  24, 12          <<< For 24MHz XTAL
// For More Info Visit: https://deepbluembedded.com/esp32-change-cpu-speed-clock-frequency/

#define SAMPLING_FREQUENCY 240 
#define WIFI_FREQUENCY 240 // min WIFI MCU Freq is 80-240
#define MIN_MCU_FREQUENCY 240

int CURRENT_CLOCK_FREQUENCY;
const int SERIAL_DEFAULT_SPEED = 115200;
int MCU_FREQUENCY_SERIAL_SPEED;


// UNIQUE FingerPrint ID for Live data Authenticity and Authentication ******************************
atsha204Class sha204;
/* WARNING: is required to set BUFFER_LENGTH to at least 64 in Wire.h and twi.h  */
const char *DATA_VALIDATION_KEY = "A9CD7F1B6688159B54BBE862F638FF9D29E0FA5F87C69D27BFCD007814BA69C9";




// Setup IO pins and Addresses **********************************************

// external 3V3 power
#define ENABLE_3v3_PWR 2
// Voltage reference
#define VOLTAGE_REF_PIN 7
// External PWM / Digital IO Pin
#define EXT_IO_ANALOG_PIN 6

// Reference resistances : loaded from config file
float ADC_R1 = 1032;
float ADC_R20 = 19910;
float ADC_R200 = 198000;
float ADC_R2M = 2000000;
uint8_t SELECTED_ADC_REF_RESISTANCE=0;
float ADC_REF_RESISTANCE[4];

// configuration: PCB specific
const float MCU_VDD = 3.33;
const float MCU_ADC_DIVIDER = 4095.0;

//I2C Bus Pins
#define SDA 8
#define SCL 9

//SHT31 sensor
SHT31 sht31;
#define SHT31_ADDRESS 0x44
#define SHTFREQ 100000ul

// LSM6DS3 motion sensor
#define LSMADDR 0x64
LSM6DS3 LSM6DS3sensor( I2C_MODE, LSMADDR);
// IMU
#define IMU_CS 38

// LCD  ***************************************************** 
// ST7789 TFT module connections
#define TFT_MOSI 8  // SDA Pin on ESP32
#define TFT_SCLK 9  // SCL Pin on ESP32
#define TFT_CS   13  // Chip select control pin
#define TFT_DC   11  // Data Command control pin
#define TFT_RST  33  // Reset pin (could connect to RST pin)
#define LCD_BACKLIT_LED 10

// TFT screen resolution
#define TFT_WIDTH 240
#define TFT_HEIGHT 280 

// TFT orientation modes
#define TFT_LANDSCAPE 2
#define TFT_LANDSCAPE_FLIPED 3
#define TFT_PORTRAIT 1
#define TFT_PORTRAIT_FLIPED 4

// onboard LED
const byte LED_RED = 36;
const byte LED_BLUE = 34;
const byte LED_GREEN = 35;

const byte LED_RED_CH = 1;
const byte LED_BLUE_CH = 0;
const byte LED_GREEN_CH = 7;

// Components Testing  **************************************
bool SCAN_I2C_BUS = false;
bool TEST_RGB_LED = false;
bool TEST_FLASH_DRIVE=false;
bool TEST_WIFI_SCAN =false;
bool TEST_WIFI_NETWORK_CONNECTION=true;
bool TEST_SECURITY_IC=false;
bool TEST_LCD=false;

// COMPONENTS STATUS AND AVAILABILITY
bool SHTsensorAvail = false;
bool MotionSensorAvail=false;
uint8_t NUMBER_OF_SENSORS =0; 

//*******************************************************************
//*******************************************************************
void turnOffLedAll(){
    // TURN OFF ALL LED
  ledcWrite(LED_RED_CH, 4096);// Green
  ledcWrite(LED_BLUE_CH, 4096);//RED
  ledcWrite(LED_GREEN_CH, 4096);//Blue

  //digitalWrite(LED_RED,HIGH);
  //digitalWrite(LED_GREEN,HIGH);
  //digitalWrite(LED_BLUE,HIGH);
}


void statusLED(byte led[], byte brightness, float time=0) {
  turnOffLedAll();
  // turn ON LED
  int bright = floor(4096-(brightness/100*3596));
  bool redPresent=false;
  bool greenPresent=false;
  bool bluePresent=false;
  byte ch;

  for(int i=0; i<(sizeof(led)/sizeof(led[0])); i++){
    if(led[i]==LED_RED){
      redPresent=true;
    }
    if(led[i]==LED_GREEN){
      greenPresent=true;
    }
    if(led[i]==LED_BLUE){
      bluePresent=true;
    }
  } 
  for(int i=0; i<(sizeof(led)/sizeof(led[0])); i++){
    if(redPresent && led[i] != LED_RED){
      bright=floor(bright*0.9);
    }
    if(led[i]==LED_RED)
      ch=LED_RED_CH;

    if(led[i]==LED_GREEN)
      ch=LED_GREEN_CH;

    if(led[i]==LED_BLUE)
      ch=LED_BLUE_CH;
    ledcWrite(ch, bright);

  }
  if(!greenPresent){
    ledcWrite(LED_GREEN, 4096);
    //digitalWrite(LED_GREEN,HIGH);
  }
  if(!redPresent){
    ledcWrite(LED_RED, 4096);
    //digitalWrite(LED_RED,HIGH);
  }
  if(!bluePresent){
    ledcWrite(LED_BLUE, 4096);
    //digitalWrite(LED_BLUE,HIGH);
  }
  if (time>0){
    delay(time*1000);
    turnOffLedAll();
  }
}  

void blinkStatusLED(byte led[], byte brightness, float time=0, byte numberBlinks=1){
  for(int i=0; i<numberBlinks; i++){
    statusLED(led, brightness, time);
    turnOffLedAll();
    delay(time*1000);
  }
}
// *********************** == Dual Core Setup == ************************
TaskHandle_t task_Core2_Loop;
void Core2Loop(void* pvParameters) {
  setup1();

  for (;;)
    loop1();
}


// ************************** == Lock semaphore == **********************
SemaphoreHandle_t MemLockSemaphoreCore1 = xSemaphoreCreateMutex();
SemaphoreHandle_t MemLockSemaphoreCore2 = xSemaphoreCreateMutex();
SemaphoreHandle_t MemLockSemaphoreDatasetFileAccess = xSemaphoreCreateMutex();
bool datasetFileIsBusySaveData = false;
bool datasetFileIsBusyUploadData = false;
SemaphoreHandle_t MemLockSemaphoreWIFI = xSemaphoreCreateMutex();

SemaphoreHandle_t McuFreqSemaphore = xSemaphoreCreateMutex();
bool McuFrequencyBusy =false;

// **************************** == Serial Class == ************************
class mSerial {
  private:
    int tftLine;
    boolean outputToLCD;
    boolean debugModeOn;
    SemaphoreHandle_t MemLockSemaphoreSerial = xSemaphoreCreateMutex();
  public:
    mSerial(boolean DebugMode, boolean ouputToLCD) {
      this->outputToLCD = ouputToLCD;
      this->debugModeOn = DebugMode;
      this->tftLine=0;
    }

    void clearLCDscreen(){
          this->tftLine=1;
          tft.setCursor(0, 0);
          tft.fillScreen(TFT_BLACK);
    }

    void setOutput2LCD(boolean stat){
      this->outputToLCD=stat;
      this->tftLine=1;
    }

    void setDebugMode(boolean stat){
      this->debugModeOn=stat;
    }

    void start(int baud) {
      if (this->debugModeOn) {
        Serial.begin(baud);
      }
    }

    void printStrln(String str) {
      if (this->debugModeOn) {
        xSemaphoreTake(MemLockSemaphoreSerial, portMAX_DELAY); // enter critical section
        Serial.println(str);
        xSemaphoreGive(MemLockSemaphoreSerial); // exit critical section    
      }

      if (outputToLCD){
        if(this->tftLine>15){
          this->tftLine=1;
          tft.setCursor(0, 20);
          tft.fillScreen(TFT_BLACK);
        }
        tft.println(str);
        this->tftLine=this->tftLine+1;
      }
    }

    void printStr(String str) {
      if (this->debugModeOn) {
        xSemaphoreTake(MemLockSemaphoreSerial, portMAX_DELAY); // enter critical section
        Serial.print(str);
        xSemaphoreGive(MemLockSemaphoreSerial); // exit critical section    
      }
      if (outputToLCD){
        if(this->tftLine>15){
          this->tftLine=1;
          tft.setCursor(0, 20);
          tft.fillScreen(TFT_BLACK);
        }
        tft.print(str);
        this->tftLine=this->tftLine+1;
      }
    }

  void changeSerialBaudRate(uint32_t Freq){
    if (Freq < 80) {
      MCU_FREQUENCY_SERIAL_SPEED = 80 / Freq * SERIAL_DEFAULT_SPEED;
    }
    else {
      MCU_FREQUENCY_SERIAL_SPEED = SERIAL_DEFAULT_SPEED;
    }
    Serial.end();
    Serial.begin(MCU_FREQUENCY_SERIAL_SPEED);
    Serial.print("\nSerial Baud Rate Speed is ");
    Serial.println(MCU_FREQUENCY_SERIAL_SPEED);
  }

};

mSerial mserial = mSerial(true, false);
uint32_t Freq = 0;

// *************************** MCU frequency **********************************************************************
void changeMcuFreq(int Freq){
  delay(1000);
  setCpuFrequencyMhz(Freq);
  CURRENT_CLOCK_FREQUENCY=Freq;
  mserial.changeSerialBaudRate(Freq);
} 

// **************** LCD ****************************

void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(TFT_BLACK);
  for (int16_t y=0; y < tft.height(); y+=5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x=0; x < tft.width(); x+=5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}

String oldText="";
void loadStatus(String text){
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(80, 120);
  tft.println(oldText);
  tft.setTextColor(TFT_BLACK);
  tft.println(text); 
  oldText=text;
}

String oldTFTtext="";
int oldPosX=0;
int oldPosY=0;
uint8_t oldTextsize=2;
int TFT_CURRENT_X_RES;
int TFT_CURRENT_Y_RES;

void tftPrintText(int x, int y, char text[], uint8_t textSize =2 , char align[] = "custom", uint16_t color=TFT_WHITE, bool deletePrevText=false){
    /*
    align: left, center, right
    */
    int posX=x;
    int posY=y;
    uint8_t pad=3; // pixel padding from the border
    uint8_t TEXT_CHAR_WIDTH=6;
    

    if (align=="center"){
      posX= (TFT_CURRENT_X_RES/2) - strlen(text)*TEXT_CHAR_WIDTH*textSize/2;
    }else if (align=="left"){
      posX=pad;
    } else if (align=="right"){
      posX=TFT_CURRENT_X_RES - strlen(text)*6*textSize-pad;
    }
    
    if (strlen(text)*6*textSize>TFT_CURRENT_X_RES)
      posX=0;

    if (posX<0){
      posX=0;
      text= (char*)String("err PosX:"+String(text)).c_str();
    }
    
    if (posY<0){
      posY=0;
      text=(char*)String("err PosY:"+String(text)).c_str();;
    }

    if (deletePrevText){
      tft.setTextColor(TFT_BLACK);
      tft.setTextSize(oldTextsize);
      tft.setCursor(oldPosX, oldPosY);
      tft.println(oldTFTtext);
    }

    tft.setTextSize(textSize);
    tft.setTextColor(color);
    tft.setCursor(posX, posY);
    tft.println(text);

    oldTFTtext=text;
    oldPosX=posX;
    oldPosY=posY;
    oldTextsize=textSize; 
  }
//**********************************************
void TFT_setScreenRotation(uint8_t rotation){
  tft.setRotation(rotation);
  switch(rotation){
      case TFT_LANDSCAPE:
        TFT_CURRENT_X_RES=TFT_HEIGHT;
        TFT_CURRENT_Y_RES= TFT_WIDTH;
        break;

      case TFT_LANDSCAPE_FLIPED:
        TFT_CURRENT_X_RES=TFT_HEIGHT;
        TFT_CURRENT_Y_RES= TFT_WIDTH;
        break;

      case TFT_PORTRAIT:
        TFT_CURRENT_X_RES=TFT_HEIGHT;
        TFT_CURRENT_Y_RES= TFT_HEIGHT;
        break;
        
      case TFT_PORTRAIT_FLIPED:
        TFT_CURRENT_X_RES=TFT_HEIGHT;
        TFT_CURRENT_Y_RES= TFT_HEIGHT;
        break;
  }
}

// ********************************************************
// *************************  == SETUP == *****************
// ********************************************************
void setup1() {
  xTaskCreatePinnedToCore(
    Core2Loop,               /* Task function. */
    "Core2Loop",                /* name of task. */
    10000,                  /* Stack size of task */
    NULL,                   /* parameter of the task */
    1,                      /* priority of the task */
    &task_Core2_Loop,            /* Task handle to keep track of created task */
    !ARDUINO_RUNNING_CORE); /* pin task to core */

}



// ********************************************************
void setup() {
  String units="";

  //ToDo: load from config file
  SELECTED_ADC_REF_RESISTANCE=0;
  ADC_REF_RESISTANCE[0]=ADC_R1;
  ADC_REF_RESISTANCE[1]=ADC_R20;
  ADC_REF_RESISTANCE[2]=ADC_R200;
  ADC_REF_RESISTANCE[3]=ADC_R2M;

  // ******** Setup *******************
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
 
  // setup LED for output
  //pinMode(LED_RED, OUTPUT);
  //pinMode(LED_BLUE, OUTPUT);
  //pinMode(LED_GREEN, OUTPUT);
  
  ledcAttachPin(LED_RED, LED_RED_CH);
  ledcAttachPin(LED_BLUE, LED_BLUE_CH);
  ledcAttachPin(LED_GREEN, LED_GREEN_CH);

  ledcSetup(LED_RED_CH, 4000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(LED_BLUE_CH, 4000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(LED_GREEN_CH, 4000, 8); // 12 kHz PWM, 8-bit resolution
  turnOffLedAll();

  // LCD backligth
  pinMode(LCD_BACKLIT_LED, OUTPUT);
  
  // ADC Power
  pinMode(ENABLE_3v3_PWR, OUTPUT);
  digitalWrite(ENABLE_3v3_PWR,LOW); // disabled


  ESP_ERROR_CHECK(nvs_flash_erase());
  nvs_flash_init();

  // ************** Configuration & testing*********************
  mserial.printStrln("Initializing...");
  mserial.start(SERIAL_DEFAULT_SPEED);// USB communication with mserial Monitor
  mserial.printStrln("OK");
  
  bool SHT_begin_result=sht31.begin(SHT31_ADDRESS);
  delay(10);
  tft.begin ();                                 // initialize a ST7789 chip
  tft.setSwapBytes (true);                      // swap the byte order for pushImage() - corrects endianness
  oldText="";
  
/*
TextSize(1) The space occupied by a character using the standard font is 6 pixels wide by 8 pixels high.
A two characters string sent with this command occup a space that is 12 pixels wide by 8 pixels high.
Other sizes are multiples of this size.
Thus, the space occupied by a character whith TextSize(3) is 18 pixels wide by 24 pixels high. The command only excepts integers are arguments
See more http://henrysbench.capnfatz.com/henrys-bench/arduino-adafruit-gfx-library-user-guide/arduino-adafruit-gfx-printing-to-the-tft-screen/
*/
  TFT_setScreenRotation(TFT_LANDSCAPE_FLIPED);
  tft.fillScreen (TFT_BLACK);
  tft.pushImage (10,65,250,87,AEONLABS_16BIT_BITMAP_LOGO);

  // PosX, PoxY, text, text size, text align, text color, delete previous text true/false
  tftPrintText(0,160,"Loading...",2,"center", TFT_WHITE, false);

  delay(5000);

  if(TEST_LCD){
    mserial.printStrln("LCD optimized lines red blue...");
    // optimized lines
    digitalWrite(LCD_BACKLIT_LED, HIGH); // Enable LCD backlight 
    testfastlines(TFT_RED, TFT_BLUE);
    delay(500);
    mserial.printStr("Hello! ST77xx TFT Test...");
    mserial.printStr("Black screen fill: ");
    uint16_t time = millis();
    tft.fillScreen(TFT_BLACK);
    time = millis() - time;
    mserial.printStrln(String(time/1000)+" sec. \n");
    delay(500);
  }

  mserial.setOutput2LCD(false);

  if(TEST_RGB_LED){
    tftPrintText(0,160,"Testing RGB LED",2,"center", TFT_WHITE, true);
    mserial.printStr("Testing RGB LED OFF...");
    statusLED((byte*)(const byte[]){LED_RED}, HIGH); // ALL led ARE OFF
    delay(2000);
    mserial.printStrln("OK");

    tftPrintText(0,160,"Test LED RED",2,"center", TFT_WHITE, true);
    mserial.printStr("Testing LED Red...");
    statusLED( (byte*)(const byte[]){LED_RED}, 100, 5);
    mserial.printStrln("OK");

    tftPrintText(0,160,"",2,"center", TFT_WHITE, true);
    mserial.printStr("Testing RGB LED OFF...");
    turnOffLedAll();
    delay(2000);
    mserial.printStrln("OK");
    
    tftPrintText(0,160,"Test LED GREEN",2,"center", TFT_WHITE, true);
    mserial.printStr("Testing LED Green...");
    statusLED( (byte*)(const byte[]){LED_GREEN}, 100, 5);
    mserial.printStrln("OK");

    mserial.printStr("Testing RGB LED OFF...");
    turnOffLedAll();
    tftPrintText(0,160,"",2,"center", TFT_WHITE, true);
    delay(2000);
    mserial.printStrln("OK");

    tftPrintText(0,160,"Test LED BLUE",2,"center", TFT_WHITE, true);
    mserial.printStr("Testing LED Blue...");
    statusLED( (byte*)(const byte[]){LED_BLUE}, 100, 5);
    
    turnOffLedAll();
    mserial.printStrln("OK");
    mserial.printStrln("LED test completed.");
    mserial.printStrln("");

    delay(2000);
  }
  
  tftPrintText(0,160, (char*)String("PSRAM "+String(ESP.getPsramSize()/1024/1024)+" Mb").c_str(),2,"center", TFT_WHITE, true);
  delay(2000);
  
  mserial.printStrln("Total heap: "+String(ESP.getHeapSize()/1024/1024)+"Mb");
  mserial.printStrln("Free heap: "+String(ESP.getFreeHeap()/1024/1024)+"Mb");
  mserial.printStrln("Total PSRAM: "+String(ESP.getPsramSize()/1024/1024)+"Mb");
  mserial.printStrln("Free PSRAM: "+String(ESP.getFreePsram()/1024/1024)+"Mb");

  mserial.printStrln("set RTC clock to firmware Date & Time ...");  
  rtc.setTimeStruct(CompileDateTime(__DATE__, __TIME__)); 
  mserial.printStrln(rtc.getDateTime(true));
  mserial.printStrln("");
  tftPrintText(0,160, (char*)rtc.getDateTime(true).c_str(),2,"center", TFT_WHITE, true); 
  delay(5000);

  CURRENT_CLOCK_FREQUENCY=getCpuFrequencyMhz();
  mserial.printStr("MCU Freq = ");
  mserial.printStr(String(CURRENT_CLOCK_FREQUENCY));
  mserial.printStrln(" MHz");
  tftPrintText(0,160,(char*) String("MCU clock: "+ String(CURRENT_CLOCK_FREQUENCY) +"Mhz").c_str(),2,"center", TFT_WHITE, true); 

  Freq = getXtalFrequencyMhz();
  mserial.printStr("XTAL Freq = ");
  mserial.printStr(String(Freq));
  mserial.printStrln(" MHz");
  
  Freq = getApbFrequency();
  mserial.printStr("APB Freq = ");
  mserial.printStr(String(Freq/1000000));
  mserial.printStrln(" MHz");

  //changeMcuFreq(10);
  //mserial.printStrln("setting to Boot min CPU Freq = " +String(getCpuFrequencyMhz()));
  //mserial.printStrln("");
  
  delay(3000);
  if(TEST_FLASH_DRIVE){
    mserial.printStr("Mounting FAT FS Partition..."); 
    tftPrintText(0,160,"Mounting F.S. ... ",1,"center", TFT_WHITE, true); 
    
    if (FFat.begin()){
        tftPrintText(0,160,"File System Ready",2,"center", TFT_WHITE, true); 
        statusLED( (byte*)(const byte[]){LED_GREEN}, 100,1); 
        mserial.printStrln(F("done."));
    }else{
      if(FFat.format()){
        tftPrintText(0,160,"File System formated successfully.",2,"center", TFT_WHITE, true); 
        mserial.printStrln("Formated sucessfully!");
        statusLED( (byte*)(const byte[]){LED_RED,LED_GREEN}, 100,5); 
      }else{
        tftPrintText(0,160,"Mounting File System.Fail.",2,"center", TFT_WHITE, true); 
        statusLED( (byte*)(const byte[]){LED_RED}, 100,5); 
        mserial.printStrln(F("fail."));
      }
    }
    mserial.printStrln("");

    // Get all information of your FFat

    unsigned int totalBytes = FFat.totalBytes();
    unsigned int usedBytes = FFat.usedBytes();
    unsigned int freeBytes  = FFat.freeBytes();
    
    delay(3000);

    mserial.printStrln("-- File System info --");

    mserial.printStr("Total space:      ");
    mserial.printStr(String(totalBytes));
    mserial.printStrln(" bytes");

    mserial.printStr("Total space used: ");
    mserial.printStr(String(usedBytes));
    mserial.printStrln(" bytes");

    mserial.printStr("Total space free: ");
    mserial.printStr(String(freeBytes));
    mserial.printStrln(" bytes");

    mserial.printStrln("");
    tftPrintText(0,160,(char*) String(String(roundf(totalBytes/1024/1024))+" / "+ String(roundf(freeBytes/1024/1024))+" Mb free").c_str(),2,"center", TFT_WHITE, true); 
    delay(1000);

    mserial.printStrln("Listing Files and Directories: ");
    // Open dir folder
    File dir = FFat.open("/");
    // Cycle all the content
    statusLED( (byte*)(const byte[]){LED_RED}, 100,0); 
    printDirectory(dir,1);
    statusLED( (byte*)(const byte[]){LED_RED}, 0,0); 
    statusLED( (byte*)(const byte[]){LED_GREEN}, 100,2); 
    mserial.printStrln("");

    File testFile;    
    testFile = FFat.open(F("/testCreate.txt"), "w"); 
    if (testFile){
      mserial.printStr("Writing content to a file...");
      testFile.print("Here the test text!!");
      testFile.close();
      mserial.printStrln("DONE.");
    }else{
      tftPrintText(0,160,"Error on open test save file.",2,"center", TFT_WHITE, true); 
      mserial.printStrln("Error openning file(1) !");
      statusLED( (byte*)(const byte[]){LED_RED}, 100,5); 
    }
      
    testFile = FFat.open(F("/testCreate.txt"), "r");
    if (testFile){
      mserial.printStrln("-- Reading file content (below) --");
      mserial.printStrln(testFile.readString());
      testFile.close();
      mserial.printStrln("DONE.");
      mserial.printStrln("-- Reading File completed --");
    }else{
      tftPrintText(0,160,"Error on open test read file",2,"center", TFT_WHITE, true); 
      mserial.printStrln("Error reading file !");
      statusLED( (byte*)(const byte[]){LED_RED}, 100,5); 
    }
    mserial.printStrln("");
  } //END TEST FLASH DRIVE

  if(TEST_SECURITY_IC){
    mserial.printStr("Get Data Validation IC Serial Number: ");
    mserial.printStrln(CryptoICserialNumber());
    tftPrintText(0,160,(char*) CryptoICserialNumber().c_str(),2,"center", TFT_WHITE, true); 

    mserial.printStrln("Test Random Gen: " + CryptoGetRandom());
    mserial.printStrln("");

    mserial.printStrln("Testing Experimental Data Validation HASH");
    mserial.printStrln("TEST IC => " + macChallengeDataAuthenticity("TEST IC"));
    mserial.printStrln("");
    delay(2000);
  }
  
  // ***************
  tft.pushImage (TFT_CURRENT_X_RES-50,0,25,18,WIFI_GREY_ICON_16BIT_BITMAP);
  tft.pushImage (TFT_CURRENT_X_RES-75,0,16,18,MEASURE_GREY_ICON_16BIT_BITMAP);

  if (SCAN_I2C_BUS) {
    // scan I2C devices
    I2Cscanner();
    delay(1000);
  }
  

  mserial.printStr("Starting SHT31 sensor...");
  startSHT(SHT_begin_result);
  delay(1000);

  mserial.printStr("Starting Motion Sensor...");
  if ( LSM6DS3sensor.begin() != 0 ) {
    mserial.printStrln("FAIL!");
    mserial.printStrln("Error starting the sensor at specified address");
    tftPrintText(0,160,"Motion Sensor Error",2,"center", TFT_RED, true); 
    statusLED( (byte*)(const byte[]){LED_RED}, 100,2); 
    MotionSensorAvail=false;
  } else {
    mserial.printStrln("started.");
    tftPrintText(0,160,"Motion sensor started",2,"center", TFT_GREEN, true); 
    statusLED( (byte*)(const byte[]){LED_GREEN}, 100,2);
    NUMBER_OF_SENSORS=NUMBER_OF_SENSORS+1;
    MotionSensorAvail = true; 
    NUMBER_OF_SENSORS_DATA_VALUES = NUMBER_OF_SENSORS_DATA_VALUES + 9;
  }
  mserial.printStrln("");
  delay(1000);
  
  // Add ADC External plug as a sensor
  NUMBER_OF_SENSORS=NUMBER_OF_SENSORS+1;  

  mserial.printStrln("Sensor detection completed. Initializing Dynamic memory with:");
  mserial.printStrln("Number of SAMPLING READINGS:"+ String(NUM_SAMPLE_SAMPLING_READINGS));
  mserial.printStrln("Number of Sensor Data values per reading:" + String(NUMBER_OF_SENSORS_DATA_VALUES));
  mserial.printStrln("PSRAM buffer size:" + String(MEASUREMENTS_BUFFER_SIZE));
  tftPrintText(0,160,(char*) String("Buff. size:"+ String(MEASUREMENTS_BUFFER_SIZE)).c_str(),2,"center", TFT_WHITE, true); 
  delay(2000);

  units="";
  // 1D number of sample readings ; 2D number of sensor data measuremtns; 3D RAM buffer size
  measurements=PSRAMalloc.initializeDynamicVar(NUM_SAMPLE_SAMPLING_READINGS, (int) NUMBER_OF_SENSORS_DATA_VALUES , MEASUREMENTS_BUFFER_SIZE);
  if (measurements == NULL){
      mserial.printStrln("Error Initializing Measruments Buffer.");
      tftPrintText(0,160,"ERR Exp. data Buffer",2,"center", TFT_WHITE, true); 
      statusLED( (byte*)(const byte[]){LED_RED}, 100,5); 
      // TODO : what to do when memeory alloc is NULL

  }else{
      mserial.printStrln("Measurements Buffer Initialized successfully.");
      float bufSize= sizeof(char)*(NUM_SAMPLE_SAMPLING_READINGS*NUMBER_OF_SENSORS_DATA_VALUES*MEASUREMENTS_BUFFER_SIZE); // bytes
      units=" B";
      if (bufSize>1024){
        bufSize=bufSize/1024;
        units=" Kb";
      }
      mserial.printStrln("Buffer size:" + String(bufSize));
      tftPrintText(0,160,(char*) String("Buf size:"+String(bufSize)+ units).c_str(),2,"center", TFT_WHITE, true); 
      delay(1000);
      tftPrintText(0,160,"Exp. Data RAM ready",2,"center", TFT_WHITE, true); 
      statusLED( (byte*)(const byte[]){LED_GREEN}, 100,2);
  }

  changeMcuFreq(WIFI_FREQUENCY);
  mserial.printStrln("setting to WIFI EN CPU Freq = " +String(getCpuFrequencyMhz())); 
  
  if (TEST_WIFI_SCAN){
    WIFIscanNetworks();
    if(TEST_WIFI_NETWORK_CONNECTION)
      connect2WIFInetowrk();
  }

  LAST_DATASET_UPLOAD = 0;
  LAST_DATA_MEASUREMENTS = 0;

  float refRes;
  units=" Ohm";
  refRes=ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE];
  if(ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE]>10000){
     refRes=ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE]/1000;
    units="k Ohm";     
  }else if(ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE]>1000000){
     refRes=ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE]/1000000;
     units="M Ohm";
  }
  
  tftPrintText(0,160,(char*)String("Sel. Rref: "+String(refRes)+ units).c_str(),2,"center", TFT_WHITE, true); 
  mserial.printStrln("ADC_REF_RESISTANCE="+String(ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE])+" Ohm");
  delay(2000);


  mserial.printStrln("Time interval for dataset upload to a dataverse repository hosted by Harvard University: ");
  mserial.printStr(String(UPLOAD_DATASET_DELTA_TIME/60000));
  mserial.printStrln(" min");
  tftPrintText(0,160,(char*)String("Exp. data upload:\n"+String(UPLOAD_DATASET_DELTA_TIME/60000)+" min").c_str(),2,"center", TFT_WHITE, true); 
  delay(2000);

  mserial.printStrln("Time interval for data measurements collection on each connected sensor:");
  mserial.printStr(String(DO_DATA_MEASURMENTS_DELTA_TIME/60000));
  mserial.printStrln(" min");
  tftPrintText(0,160,(char*)String("Exp. Data delta time:\n"+String(DO_DATA_MEASURMENTS_DELTA_TIME/60000)+" min").c_str(),2,"center", TFT_WHITE, true); 
  LAST_DATA_MEASUREMENTS=millis();
  delay(2000);
  
  mserial.printStrln("Setup completed! Running loop mode now...");
}

void loop(){
  ReadExternalAnalogData();
}

//******************************* ==  LOOP == *******************************************************

unsigned long lastMillisSensors=0;
unsigned long lastMillisWIFI=0;
bool scheduleWait = false;
int waitTimeSensors=0;
int waitTimeWIFI=0;

//************************** == Core 2: Connectivity WIFI & BLE == ***********************************************************
void loop1() {
  if ((WIFI_IS_ENABLED) && (UPLOAD_DATASET_DELTA_TIME < ( millis() -  LAST_DATASET_UPLOAD ))){
    LAST_DATASET_UPLOAD= millis();
    statusLED( (byte*)(const byte[]){LED_BLUE}, 0,0);

    mserial.printStrln("");
    mserial.printStrln("Upload Data to the Dataverse...");

    while (datasetFileIsBusySaveData){
      mserial.printStr("busy Saving data... ");
      delay(500);
    }

    xSemaphoreTake(McuFreqSemaphore, portMAX_DELAY); // enter critical section
      McuFrequencyBusy=true;
      changeMcuFreq(WIFI_FREQUENCY);
      mserial.printStrln("setting to WIFI EN CPU Freq = " +String(getCpuFrequencyMhz()));
    xSemaphoreGive(McuFreqSemaphore); // exit critical section 
    
    delay(500);
    
    connect2WIFInetowrk();
    statusLED( (byte*)(const byte[]){LED_BLUE}, 0,0);
    xSemaphoreTake(MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      datasetFileIsBusyUploadData=true;
    xSemaphoreGive(MemLockSemaphoreDatasetFileAccess); // exit critical section 
    
    mserial.printStrln(" get dataset metadata...");
    getDatasetMetadata();   
    
    statusLED( (byte*)(const byte[]){LED_GREEN}, 100,2);
    statusLED( (byte*)(const byte[]){LED_BLUE}, 0,0);
    
    UploadToDataverse(0);
    
    statusLED( (byte*)(const byte[]){LED_GREEN}, 100,2);

    xSemaphoreTake(MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      datasetFileIsBusyUploadData=false;
    xSemaphoreGive(MemLockSemaphoreDatasetFileAccess); // exit critical section 

    xSemaphoreTake(MemLockSemaphoreCore2, portMAX_DELAY); // enter critical section
      waitTimeWIFI=0;
    xSemaphoreGive(MemLockSemaphoreCore2); // exit critical section 

    xSemaphoreTake(McuFreqSemaphore, portMAX_DELAY); // enter critical section
      changeMcuFreq(MIN_MCU_FREQUENCY);
      mserial.printStrln("Setting to min CPU Freq = " +String(getCpuFrequencyMhz()));
      McuFrequencyBusy=false;
    xSemaphoreGive(McuFreqSemaphore); // exit critical section    
  }
  
  if (millis()-lastMillisWIFI > 60000){
    xSemaphoreTake(MemLockSemaphoreCore2, portMAX_DELAY); // enter critical section
      waitTimeWIFI++;
      lastMillisWIFI=millis();
    xSemaphoreGive(MemLockSemaphoreCore2); // exit critical section    
  }
}

// ********************* == Core 1 : Data Measurements Acquisition == ******************************  
void loop_p() {
  if (DO_DATA_MEASURMENTS_DELTA_TIME < ( millis() -  LAST_DATA_MEASUREMENTS ) ){
    LAST_DATA_MEASUREMENTS=millis(); 
    mserial.printStrln("");
    mserial.printStrln("Reading Specimen electrical response...");

    xSemaphoreTake(McuFreqSemaphore, portMAX_DELAY); // enter critical section
      McuFrequencyBusy=true;
      changeMcuFreq(SAMPLING_FREQUENCY);
      mserial.printStrln("Setting to ADC read CPU Freq = " +String(getCpuFrequencyMhz()));
    xSemaphoreGive(McuFreqSemaphore); // exit critical section    

    updateInternetTime();
    ReadExternalAnalogData();
    
    mserial.printStrln("Saving collected data....");
    if(datasetFileIsBusyUploadData){
      mserial.printStr("file is busy....");
      statusLED( (byte*)(const byte[]){LED_RED}, 100,2);  
    }
    if(measurements_current_pos+1 > MEASUREMENTS_BUFFER_SIZE){
      mserial.printStr("[mandatory wait]");  
      statusLED( (byte*)(const byte[]){LED_RED}, 0,0);
      while (datasetFileIsBusyUploadData){
      }
      statusLED( (byte*)(const byte[]){LED_GREEN}, 100,1);
      initSaveDataset();
    }else{ // measurements buffer is not full
      long latency=millis();
      long delta= millis()-latency;
      while (datasetFileIsBusyUploadData || (delta < MAX_LATENCY_ALLOWED)){
        delta= millis()-latency;
        delay(500);
      }
      if(datasetFileIsBusyUploadData){
        measurements_current_pos++;
        mserial.printStr("skipping file save.");  
      }else{
        initSaveDataset();
      }       
    }

    xSemaphoreTake(McuFreqSemaphore, portMAX_DELAY); // enter critical section
      McuFrequencyBusy=true;
      changeMcuFreq(MIN_MCU_FREQUENCY);
      mserial.printStrln("Setting to min CPU Freq. = " +String(getCpuFrequencyMhz()));
    xSemaphoreGive(McuFreqSemaphore); // exit critical section    

    scheduleWait=false;
    xSemaphoreTake(MemLockSemaphoreCore1, portMAX_DELAY); // enter critical section
      waitTimeSensors=0;
    xSemaphoreGive(MemLockSemaphoreCore1); // exit critical section
  }

  if (scheduleWait){
    mserial.printStrln("");
    mserial.printStrln("Waiting (schedule) ..");
  }

  if (millis()-lastMillisSensors > 60000){    
    xSemaphoreTake(MemLockSemaphoreCore1, portMAX_DELAY); // enter critical section
      lastMillisSensors=millis();
      mserial.printStrln("Sensor Acq. in " + String((DO_DATA_MEASURMENTS_DELTA_TIME/60000)-waitTimeSensors) + " min </\> Upload Experimental Data to a Dataverse Repository in " + String((UPLOAD_DATASET_DELTA_TIME/60000)-waitTimeWIFI) + " min");
      waitTimeSensors++;
    xSemaphoreGive(MemLockSemaphoreCore1); // exit critical section
  }
}

// ******************************************************************************
 void initSaveDataset(){
    mserial.printStrln("starting"); 
    // SAVE DATA MEASUREMENTS ****
    xSemaphoreTake(MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      datasetFileIsBusySaveData=true;
    xSemaphoreGive(MemLockSemaphoreDatasetFileAccess); // exit critical section 
    delay(1000);
    saveDataMeasurements();
    xSemaphoreTake(MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      datasetFileIsBusySaveData=false;
    xSemaphoreGive(MemLockSemaphoreDatasetFileAccess); // exit critical section 
  
 }

// *********************************************************
//            Upload Dataset to Harvard's Dataverse
// *********************************************************
void UploadToDataverse(byte counter) {
  if(counter>5)
    return;
    
  //Check WiFi connection status
  if(WiFi.status() != WL_CONNECTED){
    mserial.printStrln("WiFi Disconnected");
    if (connect2WIFInetowrk()){
      UploadToDataverse(counter+1);
    }
  }
  String tmp;
  char* tmpChr;
  JsonObject datasetObject = datasetInfoJson[0];
  boolean uploadStatusNotOK;

  if(datasetObject["data"].containsKey("id")){
    if(datasetObject["data"]["id"] != ""){
     tmp = String(datasetObject["data"]["id"].as<char*>());
      String rawResponse = GetInfoFromDataverse("/api/datasets/"+ tmp +"/locks");
      const size_t capacity =2*rawResponse.length() + JSON_ARRAY_SIZE(1) + 7*JSON_OBJECT_SIZE(1);
      DynamicJsonDocument datasetLocksJson(capacity);  
      // Parse JSON object
      DeserializationError error = deserializeJson(datasetLocksJson, rawResponse);
      if (error) {
        mserial.printStr("unable to retrive dataset lock status. Upload not possible. ERR: "+ String(error.f_str()));
        //mserial.printStrln(rawResponse);
        return;
      }else{
         String stat = datasetObject["status"];
         if(datasetObject.containsKey("lockType")){

           String locktype = String(datasetObject["data"]["lockType"].as<char*>());           
           mserial.printStrln("There is a Lock on the dataset: "+ locktype); 
           mserial.printStrln("Upload of most recent data is not possible without removal of the lock.");  
           // Do unlocking 
                
         }else{
            mserial.printStrln("The dataset is unlocked. Upload possible.");
         return;
        }
      }
    }else{
      mserial.printStrln("dataset ID is empty. Upload not possible. "); 
    return;
    }
  }else{
    mserial.printStrln("dataset metadata not loaded. Upload not possible. "); 
    return;
  }

  // Open the dataset file and prepare for binary upload
  File datasetFile = FFat.open("/"+EXPERIMENTAL_DATA_FILENAME, FILE_READ);
  if (!datasetFile){
    mserial.printStrln("Dataset file not found");
    return;
  }
    
  String boundary = "7MA4YWxkTrZu0gW";
  String contentType = "text/csv";
  DATASET_REPOSITORY_URL =  "/api/datasets/:persistentId/add?persistentId="+PERSISTENT_ID;
  
  String datasetFileName = datasetFile.name();
  String datasetFileSize = String(datasetFile.size());
  mserial.printStrln("Dataset File Details:");
  mserial.printStrln("Filename:" + datasetFileName);
  mserial.printStrln("size (bytes): "+ datasetFileSize);
  mserial.printStrln("");
    
  int str_len = SERVER_URL.length() + 1; // Length (with one extra character for the null terminator)
  char SERVER_URL_char [str_len];    // Prepare the character array (the buffer) 
  SERVER_URL.toCharArray(SERVER_URL_char, str_len);    // Copy it over 
    
  client.stop();
  client.setCACert(HARVARD_ROOT_CA_RSA_SHA1);
  if (!client.connect(SERVER_URL_char, SERVER_PORT)) {
      mserial.printStrln("Cloud server URL connection FAILED!");
      mserial.printStrln(SERVER_URL_char);
      int server_status = client.connected();
      mserial.printStrln("Server status code: " + String(server_status));
      return;
  }
  mserial.printStrln("Connected to the dataverse of Harvard University"); 
  mserial.printStrln("");

  mserial.printStr("Requesting URL: " + DATASET_REPOSITORY_URL);

  // Make a HTTP request and add HTTP headers    
  String postHeader = "POST " + DATASET_REPOSITORY_URL + " HTTP/1.1\r\n";
  postHeader += "Host: " + SERVER_URL + ":" + String(SERVER_PORT) + "\r\n";
  postHeader += "X-Dataverse-key: " + API_TOKEN + "\r\n";
  postHeader += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
  postHeader += "Accept: text/html,application/xhtml+xml,application/xml,application/json;q=0.9,*/*;q=0.8\r\n";
  postHeader += "Accept-Encoding: gzip,deflate\r\n";
  postHeader += "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n";
  postHeader += "User-Agent: AeonLabs LDAD Smart DAQ device\r\n";
  postHeader += "Keep-Alive: 300\r\n";
  postHeader += "Connection: keep-alive\r\n";
  postHeader += "Accept-Language: en-us\r\n";

  // jsonData header
  String jsonData = "{\"description\":\"LIVE Experimental data upload from LDAD Smart 12bit DAQ \",\"categories\":[\"Data\"], \"restrict\":\"false\", \"tabIngest\":\"false\"}";     
  String jsonDataHeader = "--" + boundary + "\r\n";
  jsonDataHeader += "Content-Disposition: form-data; name=\"jsonData\"\r\n\r\n";
  jsonDataHeader += jsonData+"\r\n";

  // dataset header
  String datasetHead = "--" + boundary + "\r\n";
  datasetHead += "Content-Disposition: form-data; name=\"file\"; filename=\"" + datasetFileName + "\"\r\n";
  datasetHead += "Content-Type: " + contentType + "\r\n\r\n";

  // request tail
  String tail = "\r\n--" + boundary + "--\r\n\r\n";

  // content length
  int contentLength = jsonDataHeader.length() + datasetHead.length() + datasetFile.size() + tail.length();
  postHeader += "Content-Length: " + String(contentLength, DEC) + "\n\n";
  
  // send post header
  int postHeader_len=postHeader.length() + 1; 
  char charBuf0[postHeader_len];
  postHeader.toCharArray(charBuf0, postHeader_len);
  client.print(charBuf0);
  mserial.printStr(charBuf0);

  // send key header
  char charBufKey[jsonDataHeader.length() + 1];
  jsonDataHeader.toCharArray(charBufKey, jsonDataHeader.length() + 1);
  client.print(charBufKey);
  mserial.printStr(charBufKey);

  // send request buffer
  char charBuf1[datasetHead.length() + 1];
  datasetHead.toCharArray(charBuf1, datasetHead.length() + 1);
  client.print(charBuf1);
  mserial.printStr(charBuf1);

  // create buffer
  const int bufSize = 2048;
  byte clientBuf[bufSize];
  int clientCount = 0;

  while (datasetFile.available()) {
    clientBuf[clientCount] = datasetFile.read();
    clientCount++;
    if (clientCount > (bufSize - 1)) {
        client.write((const uint8_t *)clientBuf, bufSize);
        clientCount = 0;
    }
  }
  datasetFile.close();
  if (clientCount > 0) {
      client.write((const uint8_t *)clientBuf, clientCount);
      mserial.printStrln("[binary data]");
  }

  // send tail
  char charBuf3[tail.length() + 1];
  tail.toCharArray(charBuf3, tail.length() + 1);
  client.print(charBuf3);
  mserial.printStr(charBuf3);

  // Read all the lines on reply back from server and print them to mserial
  mserial.printStrln("");
  mserial.printStrln("Response Headers:");
  String responseHeaders = "";

  while (client.connected()) {
    // mserial.printStrln("while client connected");
    responseHeaders = client.readStringUntil('\n');
    mserial.printStrln(responseHeaders);
    if (responseHeaders == "\r") {
      mserial.printStrln("======   end of headers ======");
      break;
    }
  }

  String responseContent = client.readStringUntil('\n');
  mserial.printStrln("Harvard University's Dataverse reply was:");
  mserial.printStrln("==========");
  mserial.printStrln(responseContent);
  mserial.printStrln("==========");
  mserial.printStrln("closing connection");
  client.stop();
}

// *********************************************************
//            Make data request to Dataverse
// *********************************************************
String GetInfoFromDataverse(String url) {
  //Check WiFi connection status
  if(WiFi.status() != WL_CONNECTED){
    mserial.printStrln("WiFi Disconnected");
    if (connect2WIFInetowrk()){
      GetInfoFromDataverse(url);
    }
  }
            
  int str_len = SERVER_URL.length() + 1; // Length (with one extra character for the null terminator)
  char SERVER_URL_char [str_len];    // Prepare the character array (the buffer) 
  SERVER_URL.toCharArray(SERVER_URL_char, str_len);    // Copy it over 
    
  client.stop();
  client.setCACert(HARVARD_ROOT_CA_RSA_SHA1);
    
  if (!client.connect(SERVER_URL_char, SERVER_PORT)) {
      mserial.printStrln("Cloud server URL connection FAILED!");
      mserial.printStrln(SERVER_URL_char);
      int server_status = client.connected();
      mserial.printStrln("Server status code: " + String(server_status));
      return "";
  }
  
  mserial.printStrln("Connected to the dataverse of Harvard University"); 
  mserial.printStrln("");
  
  // We now create a URI for the request
  mserial.printStr("Requesting URL: ");
  mserial.printStrln(url);

  // Make a HTTP request and add HTTP headers    
  // post header
  String postHeader = "GET " + url + " HTTP/1.1\r\n";
  postHeader += "Host: " + SERVER_URL + ":" + String(SERVER_PORT) + "\r\n";
  //postHeader += "X-Dataverse-key: " + API_TOKEN + "\r\n";
  postHeader += "Content-Type: text/json\r\n";
  postHeader += "Accept: text/html,application/xhtml+xml,application/xml,application/json,text/json;q=0.9,*/*;q=0.8\r\n";
  postHeader += "Accept-Encoding: gzip,deflate\r\n";
  postHeader += "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n";
  postHeader += "User-Agent: AeonLabs LDAD Smart DAQ device\r\n";
  //postHeader += "Keep-Alive: 300\r\n";
  postHeader += "Accept-Language: en-us\r\n";
  postHeader += "Connection: close\r\n\r\n";

  // request tail
  String boundary = "7MA4YWxkTrZu0gW";
  String tail = "\r\n--" + boundary + "--\r\n\r\n";

  // content length
  int contentLength = tail.length();
  //postHeader += "Content-Length: " + String(contentLength, DEC) + "\n\n";
  
  // send post header
  int postHeader_len=postHeader.length() + 1; 
  char charBuf0[postHeader_len];
  postHeader.toCharArray(charBuf0, postHeader_len);
  client.print(charBuf0);
  
  mserial.printStrln("======= Headers =======");
  mserial.printStrln(charBuf0);
  mserial.printStrln("======= End of Headers =======");
  
  // Read all the lines of the reply from server and print them to mserial
  String responseHeaders = "";

  mserial.printStr("Waiting for server response...");
  long timeout= millis();
  long ttl=millis()-timeout;
  while (client.connected() && abs(ttl) < HTTP_TTL) {
    ttl=millis()-timeout;      
    responseHeaders = client.readStringUntil('\n');
    if (responseHeaders == "\r") {
      break;
    }
  }
  if (abs(ttl) < 10000){
    mserial.printStrln("OK");
  }else{
    mserial.printStrln("timed out.");  
  }
  
  String responseContent = client.readStringUntil('\n');
  client.stop();

  return responseContent;
  }

// *********************************************************
//            Get dataset metadata
// *********************************************************

void getDatasetMetadata(){
  String rawResponse = GetInfoFromDataverse("/api/datasets/6443747/locks");
  mserial.printStr("getDatasetMetadata json ini");
  // Parse JSON object
  DeserializationError error = deserializeJson(datasetInfoJson, rawResponse);
  if (error) {
    mserial.printStr("Get Info From Dataverse deserializeJson() failed: ");
    mserial.printStrln(error.f_str());
    mserial.printStrln("==== raw response ====");
    mserial.printStr(rawResponse);  
    mserial.printStr("========================");
    return;
  }else{
     String stat = datasetInfoJson["status"];
     mserial.printStrln("Status: "+ stat);   
     if(stat=="ERROR"){
       String code = datasetInfoJson["code"];
       String msg = datasetInfoJson["message"];
       mserial.printStrln("code : " + code);
       mserial.printStrln("message : " + msg);            
     }else{
        mserial.printStrln("The dataset is unlocked");     
     }
     
    String id = datasetInfoJson["data"]["id"];
    mserial.printStrln("DATASET ID: " + id);
  }
}

// *********************************************************
//            FS FAT File Management
// *********************************************************


// *********************************************************
//  create new CSV ; delimeted dataset file 
bool initializeDataMeasurementsFile(){
  if (FFat.exists("/" +  EXPERIMENTAL_DATA_FILENAME)){
    return true;
  }
  // create new CSV ; delimeted dataset file 
  
  EXPERIMENTAL_DATA_FILE = FFat.open("/" + EXPERIMENTAL_DATA_FILENAME,"w");
  if (EXPERIMENTAL_DATA_FILE){
    mserial.printStr("Creating a new dataset CSV file and adding header ...");
    String DataHeader[NUM_SAMPLE_SAMPLING_READINGS];
    
    DataHeader[0]="Date&Time";
    DataHeader[1]="ANALOG RAW (0-4095)";
    DataHeader[2]="Vref (Volt)";
    DataHeader[3]="V (Volt)";
    DataHeader[4]="R (Ohm)";
    DataHeader[5]="SHT TEMP (*C)";
    DataHeader[6]="SHT Humidity (%)";
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
    for (int j = 0; j < NUM_SAMPLE_SAMPLING_READINGS; j++) {
      lineRowOfData= lineRowOfData + DataHeader[j] +";";
    }
    EXPERIMENTAL_DATA_FILE.println(lineRowOfData);        
    EXPERIMENTAL_DATA_FILE.close();
    mserial.printStrln("header added to the dataset file.");
  }else{
    mserial.printStrln("Error creating file(2): " + EXPERIMENTAL_DATA_FILENAME);
    return false;
  }
  mserial.printStrln("");  
  return true;
}

// *********************************************************
bool saveDataMeasurements(){
  statusLED( (byte*)(const byte[]){LED_RED, LED_GREEN}, 0,0);
  EXPERIMENTAL_DATA_FILE = FFat.open("/" + EXPERIMENTAL_DATA_FILENAME,"a+");
  if (EXPERIMENTAL_DATA_FILE){
    mserial.printStrln("Saving data measurements to the dataset file ...");
    for (int k = 0; k <= measurements_current_pos; k++) {
      for (int i = 0; i < NUM_SAMPLE_SAMPLING_READINGS; i++) {
        String lineRowOfData="";
        for (int j = 0; j < NUM_SAMPLE_SAMPLING_READINGS; j++) {
            lineRowOfData= lineRowOfData + measurements[k][i][j] +";";
        }
        lineRowOfData+= macChallengeDataAuthenticity(lineRowOfData) + ";" + CryptoICserialNumber();
        mserial.printStrln(lineRowOfData);
        EXPERIMENTAL_DATA_FILE.println(lineRowOfData);        
      }
    }
    EXPERIMENTAL_DATA_FILE.close();
    delay(500);
    mserial.printStrln("collected data measurements stored in the dataset CSV file.("+ EXPERIMENTAL_DATA_FILENAME +")");
    measurements_current_pos=0;
    statusLED( (byte*)(const byte[]){LED_GREEN}, 0,0);
    return true;
  }else{
    mserial.printStrln("Error creating CSV dataset file(3): " + EXPERIMENTAL_DATA_FILENAME);
    statusLED( (byte*)(const byte[]){LED_RED}, 100,2);
    return false;
  }
}


//***********************************************************
void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      mserial.printStrln("");
      mserial.printStrln("no more files or directories found.");
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      mserial.printStr("     ");
    }
    mserial.printStr(entry.name());
    if (entry.isDirectory()) {
      mserial.printStrln("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      mserial.printStr("          ");
      mserial.printStrln(String(entry.size()));
    }
    entry.close();
  }
}
// *********************************************************
//            Sensor Data Measurements 
// *********************************************************
// onBoard Sensors
void onBoardSensorMeasurements(int i){
    union Data t; 
    union Data h;

    if (SHTsensorAvail) {  
      mserial.printStr("SHT31 data: ");
      sht31.read();
  
      t.f = sht31.getTemperature();
      h.f = sht31.getHumidity();
  
      if (! isnan(t.f)) { // check if 'is not a number'
        mserial.printStr("Temp *C = ");
        mserial.printStr(String(t.chr));
        int MAX_DATA__MEASUREMENT_SIZE=20;

        measurements[i][5][measurements_current_pos]= (char*) String(t.f).c_str();

      } else {
        mserial.printStrln("Failed to read temperature");
      }
  
      if (! isnan(h.f)) { // check if 'is not a number'
        mserial.printStr("   Hum. % = ");
        mserial.printStrln(String(h.f));
        
        measurements[i][6][measurements_current_pos]= (char*) String(h.f).c_str();
      } else {
        mserial.printStrln("Failed to read humidity");
      }
    }
 
    getLSM6DS3sensorData(i);
}

// **************************************************
// ********************************************************
void getLSM6DS3sensorData(int i) {
  if (MotionSensorAvail==false)
      return;

  union Data motion;
  //Get all parameters
  mserial.printStr("LSM6DS3 Accelerometer Data: ");
  mserial.printStr(" X1 = ");
  motion.f=LSM6DS3sensor.readFloatAccelX();
  mserial.printStr(String(motion.chr));
  measurements[i][7][measurements_current_pos]= (char*) String(motion.f).c_str();
  
  mserial.printStr("  Y1 = ");
  motion.f=LSM6DS3sensor.readFloatAccelY();
  mserial.printStr(String(motion.chr));
  measurements[i][8][measurements_current_pos]= (char*) String(motion.f).c_str();

  mserial.printStr("  Z1 = ");
  motion.f=LSM6DS3sensor.readFloatAccelZ();
  mserial.printStr(String(motion.chr));
  measurements[i][9][measurements_current_pos]= (char*) String(motion.f).c_str();
  
  mserial.printStr("LSM6DS3 Gyroscope data:");
  mserial.printStr(" X1 = ");  
  motion.f=LSM6DS3sensor.readFloatGyroX();
  mserial.printStr(String(motion.chr));
  measurements[i][10][measurements_current_pos]= (char*) String(motion.f).c_str();

  mserial.printStr("  Y1 = ");
  motion.f=LSM6DS3sensor.readFloatGyroY();
  mserial.printStr(String(motion.chr));
  measurements[i][11][measurements_current_pos]= (char*) String(motion.f).c_str();
  
  mserial.printStr("  Z1 = ");
  motion.f=LSM6DS3sensor.readFloatGyroZ();
  mserial.printStr(String(motion.chr));
  measurements[i][12][measurements_current_pos]= (char*) String(motion.f).c_str();
  
  mserial.printStr("\nLSM6DS3 Thermometer Data:");
  mserial.printStr(" Degrees C1 = ");
  motion.f=LSM6DS3sensor.readTempC();
  mserial.printStr(String(motion.chr));
  measurements[i][13][measurements_current_pos]= (char*) String(motion.f).c_str();
  
  //mserial.printStrln(String(LSM6DS3sensor.readTempF()));

  mserial.printStr("LSM6DS3 SensorOne Bus Errors Reported:");
  mserial.printStr("success = ");
  motion.f=LSM6DS3sensor.allOnesCounter;
  mserial.printStr(String(motion.chr));
  measurements[i][14][measurements_current_pos]= (char*) String(motion.f).c_str();
  
  mserial.printStr("no success = ");
  motion.f=LSM6DS3sensor.nonSuccessCounter;
  mserial.printStr(String(motion.chr));
  measurements[i][15][measurements_current_pos]= (char*) String(motion.f).c_str();
}


//***************************************************
void ReadExternalAnalogData() {

  union Data adc_ch_analogRead_raw; 
  union Data ADC_CH_REF_VOLTAGE; 
  union Data adc_ch_measured_voltage; 
  union Data adc_ch_calcukated_e_resistance; 

  float adc_ch_measured_voltage_Sum = 0;
  float adc_ch_calcukated_e_resistance_sum = 0;
  int num_valid_sample_measurements_made = 0;

  mserial.setOutput2LCD(false);
    
  //tft.pushImage (10,65,250,87,AEONLABS_16BIT_BITMAP_LOGO);

  tft.fillRect(10, 65 , 250, 87, TFT_BLACK);
  tft.pushImage (TFT_CURRENT_X_RES-75,0,16,18,MEASURE_ICON_16BIT_BITMAP);
  
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
  
  ADC_CH_REF_VOLTAGE.f= analogRead(VOLTAGE_REF_PIN)/MCU_ADC_DIVIDER * MCU_VDD;

  tftPrintText(0,25,(char*) String(String(NUM_SAMPLE_SAMPLING_READINGS)+ " measurements\nsamples requested\n\nSampling interval (ms)\n"+String(SAMPLING_INTERVAL)+"\n\nADC CH OUT: "+String(ADC_CH_REF_VOLTAGE.f)+" Volt").c_str(),2,"left", TFT_WHITE, true); 
  delay(2000);

  int zerosCount=0;
  for (byte i = 0; i < NUM_SAMPLE_SAMPLING_READINGS; i++) {
    mserial.printStrln("");
    mserial.printStr(String(i));
    mserial.printStr(": ");

    adc_ch_analogRead_raw.f = analogRead(EXT_IO_ANALOG_PIN);
    num_valid_sample_measurements_made = num_valid_sample_measurements_made + 1;
    // Vref
    ADC_CH_REF_VOLTAGE.f= analogRead(VOLTAGE_REF_PIN)/MCU_ADC_DIVIDER * MCU_VDD;
    
    // ADC Vin
    adc_ch_measured_voltage.f = adc_ch_analogRead_raw.f  * ADC_CH_REF_VOLTAGE.f / MCU_ADC_DIVIDER;
    
    //R
    adc_ch_calcukated_e_resistance.f = (ADC_REF_RESISTANCE[SELECTED_ADC_REF_RESISTANCE] * adc_ch_analogRead_raw.f ) / (MCU_ADC_DIVIDER- adc_ch_analogRead_raw.f);


    mserial.printStr("ADC Vref raw: ");
    mserial.printStrln(String(adc_ch_analogRead_raw.f));

    mserial.printStr("ADC Vref: ");
    mserial.printStr(String(ADC_CH_REF_VOLTAGE.f));
    mserial.printStrln(" Volt");

    mserial.printStr("ADC CH IN raw: ");
    mserial.printStrln(String(adc_ch_analogRead_raw.f));

    mserial.printStr("ADC CH READ: ");
    mserial.printStr(String(adc_ch_measured_voltage.f));
    mserial.printStrln(" Volt");
    mserial.printStr("Calc. E.R.: ");
    mserial.printStr(String(adc_ch_calcukated_e_resistance.f));
    mserial.printStrln("  Ohm");

    // SAMPLING_READING POS,  SENSOR POS, MEASUREMENTS_BUFFER_ POS
    measurements [i][0][measurements_current_pos]= (char*) rtc.getDateTime(true).c_str();
    measurements [i][1][measurements_current_pos]= (char*) String(adc_ch_analogRead_raw.f).c_str();
    measurements [i][2][measurements_current_pos]= (char*) String(ADC_CH_REF_VOLTAGE.f).c_str();
    measurements [i][3][measurements_current_pos]= (char*) String(adc_ch_measured_voltage.f).c_str();
    measurements [i][4][measurements_current_pos]= (char*) String(adc_ch_calcukated_e_resistance.f).c_str();
    
    onBoardSensorMeasurements(i);

    delay(SAMPLING_INTERVAL);

    if (adc_ch_analogRead_raw.f==0) {
      zerosCount++;
    }else{
      adc_ch_calcukated_e_resistance_sum = adc_ch_calcukated_e_resistance_sum + adc_ch_calcukated_e_resistance.f;
      adc_ch_measured_voltage_Sum = adc_ch_measured_voltage_Sum + adc_ch_measured_voltage.f;
    }
  }

  tft.pushImage (TFT_CURRENT_X_RES-75,0,16,18,MEASURE_GREY_ICON_16BIT_BITMAP);

  if (zerosCount>0) {
    tftPrintText(0,25,(char*) String(String(zerosCount)+" zero(s) found\nconsider chg ref. R\nswitch on the DAQ" ).c_str(),2,"left", TFT_WHITE, true); 
    mserial.printStrln("Zero value measur. founda: "+String(adc_ch_analogRead_raw.f));
    statusLED( (byte*)(const byte[]){LED_RED}, 100,2); 
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
  tftPrintText(0,25,(char*) String("Total data samples: \n"+String(num_valid_sample_measurements_made)+"/"+String(NUM_SAMPLE_SAMPLING_READINGS)+"\n\n"+"Avg ADC CH volt.:\n"+String(adc_ch_measured_voltage_avg)+" Volt\n\nAverage ADC CH ER:\n"+String(adc_ch_calcukated_e_resistance_avg)+" "+units  ).c_str(),2,"left", TFT_WHITE, true); 

  delay(5000);
}


// ********************************************************
void startSHT(bool result) {
  if (result){
    mserial.printStr("DONE.");
    mserial.printStrln("status code: " + String(sht31.readStatus()));

    tftPrintText(0,160,(char*) String("Init. SHT3x sensor (stat. code): "+String(sht31.readStatus())).c_str(),2,"center", TFT_WHITE, true); 
    if (sht31.readStatus()==0){
      NUMBER_OF_SENSORS=NUMBER_OF_SENSORS+1;
      SHTsensorAvail = true;
      statusLED( (byte*)(const byte[]){LED_GREEN}, 100,1); 
      NUMBER_OF_SENSORS_DATA_VALUES = NUMBER_OF_SENSORS_DATA_VALUES + 2;
    }else{
      SHTsensorAvail = false;
      statusLED( (byte*)(const byte[]){LED_RED}, 100,1); 
    }
    mserial.printStrln("");
  }else{
    mserial.printStr("SHT sensor not found ");
    tftPrintText(0,160,(char*) String("SHT sensor not found").c_str(),2,"center", TFT_RED, true);
    statusLED( (byte*)(const byte[]){LED_RED}, 100,1); 
  }
}


// ********************************************************
// see https://stackoverflow.com/questions/52221727/arduino-wire-library-returning-error-code-7-which-is-not-defined-in-the-library
const char*i2c_err_t[]={
    "I2C_ERROR_OK",
    "I2C_ERROR_DEV",      // 1
    "I2C_ERROR_ACK",     // 2
    "I2C_ERROR_TIMEOUT", // 3
    "I2C_ERROR_BUS",     // 4
    "I2C_ERROR_BUSY",    // 5
    "I2C_ERROR_MEMORY",  // 6
    "I2C_ERROR_CONTINUE",// 7
    "I2C_ERROR_NO_BEGIN" // 8
};

void I2Cscanner() {
  mserial.printStrln ("I2C scanner. \n Scanning ...");  
  byte count = 0;
  String addr;

  for (byte i = 8; i < 120; i++){
    tftPrintText(0,160, (char*) String("I2C scan: "+String(i)+"/"+String(120)).c_str(),2,"center", TFT_WHITE, true); 

    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    byte error = Wire.endTransmission();
    addr="";
    if (i < 16){
       mserial.printStr("0");
       addr="0";
    }
    addr=addr+String(i, HEX);

    if (error == 0) { // Receive 0 = success (ACK response)
      mserial.printStr ("Found address: ");
      mserial.printStr (String(i, DEC));
      mserial.printStr (" (0x");
      mserial.printStr (String(i, HEX));     // PCF8574 7 bit address
      mserial.printStrln (")");
      count++;
      tftPrintText(0,160, (char*) String("I2C found at 0x"+addr).c_str(),2,"center", TFT_GREEN, true);
    } else if (error == 4) {
      mserial.printStr("Unknown error at address 0x");
      mserial.printStrln(addr);
      tftPrintText(0,160, (char*) String(" 0x"+addr + " "+String(i2c_err_t[error])).c_str(),2,"center", TFT_WHITE, true); 
      statusLED( (byte*)(const byte[]){LED_RED}, 100, 1);
    } else{
        statusLED( (byte*)(const byte[]){LED_GREEN, LED_RED}, 100, 1);
    }
  }

  mserial.printStr ("Found ");
  mserial.printStr (String(count));        // numbers of devices
  mserial.printStrln (" device(s).");
  if (count == 0) {
      statusLED( (byte*)(const byte[]){LED_RED}, 100, 2);
      tftPrintText(0,160, (char*) String("Found I2C "+String(count)+" device(s)").c_str(),2,"center", TFT_RED, true); 
    }else if (count==2){
      statusLED( (byte*)(const byte[]){LED_GREEN, LED_RED}, 100, 2);
      tftPrintText(0,160, (char*) String("Found I2C "+String(count)+" device(s)").c_str(),2,"center", TFT_YELLOW, true); 
  }else{
      statusLED( (byte*)(const byte[]){LED_GREEN}, 100, 2);
      tftPrintText(0,160, (char*) String("Found I2C "+String(count)+" device(s)").c_str(),2,"center", TFT_GREEN, true);     
  }
}


// *********************************************************
//            WIFI Connectivity & Management 
// *********************************************************
bool connect2WIFInetowrk(){
  if(CURRENT_CLOCK_FREQUENCY < WIFI_FREQUENCY)
    return false;

  if (WiFi.status() == WL_CONNECTED)
    return true;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA); //  Station Mode   
  mserial.printStrln("Connecting to the WIFI Network: \"" + String(WIFI_SSID)+"\"");
  
  int WiFi_prev_state=-10;
  int cnt = 0;        
  uint8_t statusWIFI=WL_DISCONNECTED;
  
  while (statusWIFI != WL_CONNECTED) {
      if(CURRENT_CLOCK_FREQUENCY < WIFI_FREQUENCY)
        mserial.printStrln("MCU clock frequency below min for WIFI connectivity");
        break;

    if (cnt == 0) {
          mserial.printStrln("WIFI Begin...");
          tftPrintText(0,160,"Starting WiFi...",2,"center", TFT_WHITE, true); 
          WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
          delay(300);
      }
    statusLED( (byte*)(const byte[]){LED_BLUE}, 100,0.300);
    turnOffLedAll();
    delay(0.300);
    statusWIFI = WiFi.waitForConnectResult();
    
    if(WiFi_prev_state != statusWIFI){
        WiFi_prev_state = statusWIFI;
        mserial.printStrln("("+String(statusWIFI)+"): "+get_wifi_status(statusWIFI));
        tftPrintText(0,160, (char*) String(get_wifi_status(statusWIFI)).c_str(),2,"center", TFT_WHITE, true); 
        statusLED( (byte*)(const byte[]){LED_RED, LED_GREEN}, 100,1); 
    }

    mserial.printStr(".");
    if (++cnt == 10){
      statusLED( (byte*)(const byte[]){LED_RED}, 100,2); 
      mserial.printStrln("");
      mserial.printStrln("Restarting WiFi! ");
       
      WIFIscanNetworks();
        
      cnt = 0;
    }
  }
  if (statusWIFI != WL_CONNECTED){
    statusLED( (byte*)(const byte[]){LED_GREEN}, 100,2); 
    tftPrintText(0,160,"Connected to WiFi",2,"center", TFT_WHITE, true); 
    return true;
  }else{
    statusLED( (byte*)(const byte[]){LED_RED}, 100,2); 
    tftPrintText(0,160,"WiFi unavailable",2,"center", TFT_WHITE, true); 
    return false;
  }
}

// ********************************************************
String get_wifi_status(int status){
    switch(status){
        case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS(0): WiFi is in process of changing between statuses";
        case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED(2): Successful connection is established";
        case WL_NO_SSID_AVAIL:
          statusLED( (byte*)(const byte[]){LED_RED}, 100, 1);
          return "SSID cannot be reached";
        case WL_CONNECT_FAILED:
          statusLED( (byte*)(const byte[]){LED_RED}, 100, 1);
          return "WL_CONNECT_FAILED (4): Password is incorrect";
        case WL_CONNECTION_LOST:
          blinkStatusLED( (byte*)(const byte[]){LED_BLUE}, 100, 0.300, 5);
          return "WL_CONNECTION_LOST (5)";
        case WL_CONNECTED:
        return "WL_CONNECTED (3)";
        case WL_DISCONNECTED:
        return "WL_DISCONNECTED (6): Module is not configured in station mode";
    }
}

// *********************************************************
void WIFIscanNetworks(){
    if(CURRENT_CLOCK_FREQUENCY < WIFI_FREQUENCY)
    return;

  if (WiFi.status() == WL_CONNECTED)
    return;

  statusLED( (byte*)(const byte[]){LED_BLUE}, 0,0);  
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0) {
    tftPrintText(0,160,"No WiFi Networks found.",2,"center", TFT_WHITE, true); 
    mserial.printStrln("no networks found");
    statusLED( (byte*)(const byte[]){LED_RED}, 100,2); 
  } else {
    mserial.printStr(String(n));
    mserial.printStrln(" WiFi Networks found:");
    tftPrintText(0,160,(char*) String("Found "+String(n)+" WiFi Networks").c_str(),2,"center", TFT_WHITE, true);
    delay(1000); 
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      mserial.printStr(String(i + 1));
      mserial.printStr(": ");
      mserial.printStr(String(WiFi.SSID(i)));
      mserial.printStr(" (");
      mserial.printStr(String(WiFi.RSSI(i)));
      mserial.printStr(")");
      mserial.printStrln((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
    statusLED( (byte*)(const byte[]){LED_GREEN}, 100,2); 
  }  
}

// *************************************************************
void WiFiEvent(WiFiEvent_t event) {  
  switch (event) {
    case SYSTEM_EVENT_WIFI_READY: 
      mserial.printStrln("WiFi interface ready");
      tftPrintText(0,160,"WiFi interface ready",2,"center", TFT_WHITE, true); 
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      mserial.printStrln("Completed scan for access points");
      tftPrintText(0,160,"Completed AP scan",2,"center", TFT_WHITE, true); 
      break;
    case SYSTEM_EVENT_STA_START:
      tft.pushImage (TFT_CURRENT_X_RES-50,0,25,18,WIFI_ORANGE_ICON_16BIT_BITMAP);
      mserial.printStrln("WiFi client started");
      tftPrintText(0,160,"WiFi client started",1,"center", TFT_WHITE, true); 
      tftPrintText(0,160," ",2,"center", TFT_BLACK, true); 
      break;
    case SYSTEM_EVENT_STA_STOP:
      mserial.printStrln("WiFi clients stopped");
      tftPrintText(0,160,"WiFi clients stopped",2,"center", TFT_WHITE, true); 
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      mserial.printStrln("Connected to access point");
      tftPrintText(0,160,"Connected to access point",2,"center", TFT_WHITE, true); 
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      if (WIFIconnected==false)
        break;      
      xSemaphoreTake(MemLockSemaphoreWIFI, portMAX_DELAY); // enter critical section
          WIFIconnected=false;
      xSemaphoreGive(MemLockSemaphoreWIFI); // exit critical section
      mserial.printStrln("Disconnected from WiFi access point");
      tftPrintText(0,160,"Disconnected from WiFi AP",2,"center", TFT_WHITE, true); 
      tft.pushImage(TFT_CURRENT_X_RES-50,0,25,18,WIFI_GREY_ICON_16BIT_BITMAP);
      delay(100);
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      mserial.printStrln("Authentication mode of access point has changed");
      tftPrintText(0,160,"AP Auth mode has changed",2,"center", TFT_WHITE, true); 
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      mserial.printStrln("Connection Details");
      mserial.printStrln("     IP     : "+WiFi.localIP().toString());
      mserial.printStrln("     Gateway: "+WiFi.gatewayIP().toString());
      tftPrintText(0,160,(char*) String("I.P.:"+WiFi.localIP().toString()).c_str(),2,"center", TFT_WHITE, true); 
      delay(5000);
      if(!Ping.ping("www.google.com", 3)){
        tft.pushImage (TFT_CURRENT_X_RES-50,0,25,18,WIFI_RED_ICON_16BIT_BITMAP);
        mserial.printStrln("no Internet connectivity");
        tftPrintText(0,160,"no Internet conn.",2,"center", TFT_WHITE, true); 
      }else{
        tft.pushImage (TFT_CURRENT_X_RES-50,0,25,18,WIFI_BLUE_ICON_16BIT_BITMAP);
        mserial.printStrln("Internet connection OK");
        tftPrintText(0,160,"Internet conn. OK",2,"center", TFT_WHITE, true); 
        //init time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        updateInternetTime();
      }

      xSemaphoreTake(MemLockSemaphoreWIFI, portMAX_DELAY); // enter critical section
        WIFIconnected=true;
      xSemaphoreGive(MemLockSemaphoreWIFI); // exit critical section  
      delay(200);
      break;
    case SYSTEM_EVENT_STA_LOST_IP:
      mserial.printStrln("Lost IP address and IP address is reset to 0");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      mserial.printStrln("WiFi Protected Setup (WPS): succeeded in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      mserial.printStrln("WiFi Protected Setup (WPS): failed in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      mserial.printStrln("WiFi Protected Setup (WPS): timeout in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      mserial.printStrln("WiFi Protected Setup (WPS): pin code in enrollee mode");
      break;
    case SYSTEM_EVENT_AP_START:
      mserial.printStrln("WiFi access point started");
      break;
    case SYSTEM_EVENT_AP_STOP:
      mserial.printStrln("WiFi access point  stopped");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      mserial.printStrln("Client connected");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      mserial.printStrln("Client disconnected");
      break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
      mserial.printStrln("Assigned IP address to client");
      break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
      mserial.printStrln("Received probe request");
      break;
    case SYSTEM_EVENT_GOT_IP6:
      mserial.printStrln("IPv6 is preferred");
      break;
    case SYSTEM_EVENT_ETH_START:
      mserial.printStrln("Ethernet started");
      break;
    case SYSTEM_EVENT_ETH_STOP:
      mserial.printStrln("Ethernet stopped");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      mserial.printStrln("Ethernet connected");
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      mserial.printStrln("Ethernet disconnected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      mserial.printStrln("Obtained IP address");
      break;
    default: break;
  }
}


// *************************************************************
void updateInternetTime(){
  long diff= (millis()- NTP_last_request);
  if(abs(diff)< NTP_request_interval && NTP_last_request!=0){
    mserial.printStrln("Internet Time (NTP) is up to date. ");
    return;
  }
  NTP_last_request=millis();

  mserial.printStrln("Requesting Internet Time (NTP) to "+ String(ntpServer));
  if(!getLocalTime(&timeinfo)){
    tftPrintText(0,160,"Fialed to get Internet Time",2,"center", TFT_WHITE, true); 
    mserial.printStrln("Failed to obtain Internet Time. Current System Time is " + String(rtc.getDateTime(true)));
    return;
  }else{
    rtc.setTimeStruct(timeinfo); 
    mserial.printStrln("Internet Time is: " + String(rtc.getDateTime(true)));
  }
}


// *************************************************************
// *************************************************************

String CryptoGetRandom(){
  uint8_t response[32];
  uint8_t returnValue;
  sha204.simpleWakeup();
  returnValue = sha204.simpleGetRandom(response);
  sha204.simpleSleep();  
  return hexDump(response, sizeof(response));
}

String CryptoICserialNumber(){
  uint8_t serialNumber[6];
  uint8_t returnValue;

  sha204.simpleWakeup();
  returnValue = sha204.simpleGetSerialNumber(serialNumber);
  sha204.simpleSleep();
  
  return hexDump(serialNumber, sizeof(serialNumber));
}

String macChallengeDataAuthenticity(String text ){
  int str_len = text.length() + 1;
  char text_char [str_len];
  text.toCharArray(text_char, str_len);
        
  static uint32_t n = 0;
  uint8_t mac[32];
  uint8_t challenge[sizeof(text_char)] = {0};

  sprintf((char *)challenge, text_char, n++);

  sha204.simpleWakeup();
  uint8_t ret_code = sha204.simpleMac(challenge, mac);
  sha204.simpleSleep();  
  
  if (ret_code != SHA204_SUCCESS){
    mserial.printStrln("simpleMac failed.");
    return "Error";
  }

  return hexDump(mac, sizeof(mac));
}

String macChallengeDataAuthenticityOffLine(char dataRow[] ){
  static uint32_t n = 0;
  uint8_t mac[32];
  uint8_t challenge[sizeof(dataRow)] = {0}; // MAC_CHALLENGE_SIZE
  
  sprintf((char *)challenge, dataRow, n++);
  mserial.printStr("challenge: ");
  mserial.printStrln((char *)challenge);
  
  uint8_t key[32];
  //Change your key here.
  loadKeyFromHex(DATA_VALIDATION_KEY, key);
  uint8_t mac_offline[32];
  sha204.simpleWakeup();
  int ret_code = sha204.simpleMacOffline(challenge, mac_offline, key);
  sha204.simpleSleep();
  mserial.printStr("MAC Offline:\n");
  return hexDump(mac_offline, sizeof(mac_offline));
}

// ********************************************************
String hexDump(uint8_t *data, uint32_t length){
  char buffer[3];
  String hexStr="0x";
  for (uint32_t i = 0; i < length; i++){    
    snprintf(buffer, sizeof(buffer), "%02X", data[i]);
    hexStr += buffer;
  }
  return hexStr;
}

int char2int(char input){
  if (input >= '0' && input <= '9')
    return input - '0';
  if (input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if (input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return 0;
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
void hex2bin(const char *src, uint8_t *target){
  while (*src && src[1]){
    *(target++) = char2int(*src) * 16 + char2int(src[1]);
    src += 2;
  }
}

void loadKeyFromHex(const char *hex_key, uint8_t *key){
  hex2bin(hex_key, key);
}

//*********************************************************************

// Convert compile time to Time Struct
tm CompileDateTime(char const *dateStr, char const *timeStr){
    char s_month[5];
    int year;
    struct tm t;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(dateStr, "%s %d %d", s_month, &t.tm_mday, &year);
    sscanf(timeStr, "%2d %*c %2d %*c %2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
    // Find where is s_month in month_names. Deduce month value.
    t.tm_mon = (strstr(month_names, s_month) - month_names) / 3;    
    t.tm_year = year - 1900;    
    return t;
}
