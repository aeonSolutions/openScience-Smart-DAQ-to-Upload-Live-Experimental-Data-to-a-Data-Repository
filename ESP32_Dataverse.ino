#include "Wire.h"
#include "SparkFunLSM6DS3.h"
#include "SPI.h"
#include <SHT31.h>
#include "FS.h"
#include "FFat.h"
#include "WiFiClientSecure.h"
#include "sha204_i2c.h"
#include "nvs_flash.h"
#include <semphr.h>
#include "time.h"
#include <ESP32Ping.h>
#include "ESP32Time.h"

// WIFI SETUP ********************************************
const char* WIFI_SSID = "ssid";
const char* WIFI_PASSWORD = "password";
bool WIFIconnected=false;

// RTC SETUP *******************
//ESP32Time rtc;
ESP32Time rtc(3600);  // offset in seconds GMT+1

// RTC: NTP server ***********************
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;
long NTP_request_interval=64000;// 64 sec.
long NTP_last_request=0;
  
// DATAVERSE **********************************
String API_TOKEN = "xxxxXXXXxxxxxxx";
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


// File management *********************************
File EXPERIMENTAL_DATA_FILE;
String EXPERIMENTAL_DATA_FILENAME = "ER_measurements.csv";

#define FORMAT_FATFS_IF_FAILED true


// Measurements: Data ********************************
// array size is the number of sensors to do data collection
#define NUM_SAMPLES 5
#define NUM_DATA_READINGS 16
#define SAMPLING_INTERVAL 1000
#define MEASUREMENTS_BUFFER_SIZE 10
String measurements[MEASUREMENTS_BUFFER_SIZE][NUM_SAMPLES][NUM_DATA_READINGS];
byte measurements_current_pos=0;

/* Attention: is needed to set BUFFER_LENGTH to at least 64 in Wire.h and twi.h*/
atsha204Class sha204;
const char *DATA_VALIDATION_KEY = "A9CD7F1B6688159B54BBE862F638FF9D29E0FA5F87C69D27BFCD007814BA69C9";


// Measurements: Planning / Schedule
unsigned long UPLOAD_DATASET_DELTA_TIME= NUM_SAMPLES*SAMPLING_INTERVAL + 120000; // 2 min
unsigned long DO_DATA_MEASURMENTS_DELTA_TIME= NUM_SAMPLES*SAMPLING_INTERVAL + 60000; // 1 min
unsigned long LAST_DATASET_UPLOAD = 0;
unsigned long LAST_DATA_MEASUREMENTS = 0;
unsigned long MAX_LATENCY_ALLOWED = (unsigned long)(DO_DATA_MEASURMENTS_DELTA_TIME/2);

// Setup IO pins and Addresses **********************************************

//I2C Bus Pins
#define SDA 8
#define SCL 9

//SHT31 sensor
SHT31 sht31;
#define SHT31_ADDRESS 0x44
#define SHTFREQ 100000ul

// LSM6DS3 motion sensor
#define LSMADDR 0x6B
LSM6DS3 LSM6DS3sensor( I2C_MODE, LSMADDR);

// External PWM / Digital IO Pin
#define EXT_IO_ANALOG_PIN 6
#define Vin 3.33
#define R1 1032



// Components to test **************************************
bool scanI2C = true;
bool testSHTsensor = true;
bool testMotionSensor = true;
bool testRgbLed = true;
bool SHTavail = false;


//*******************************************************************
const byte ledRed = 36;
const byte ledBlue = 34;
const byte ledGreen = 35;


//**********************************************************************

void StatusLED(int led, int pauseTime) {
  if (testRgbLed) {
    // pausetime in seconds
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay (pauseTime * 1000);
    digitalWrite(led, LOW);   // turn the LED on (HIGH is the voltage level)
    delay (1000);
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


// **************************** == Serial Class == ************************
class mSerial {
  private:
    boolean debugModeOn;
    SemaphoreHandle_t MemLockSemaphoreSerial = xSemaphoreCreateMutex();
  public:
    mSerial(boolean DebugMode) {
      this->debugModeOn = DebugMode;
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
    }

    void printStr(String str) {
      if (this->debugModeOn) {
        xSemaphoreTake(MemLockSemaphoreSerial, portMAX_DELAY); // enter critical section
        Serial.print(str);
        xSemaphoreGive(MemLockSemaphoreSerial); // exit critical section    
      }
    }
};

mSerial mserial = mSerial(true);

// ********************************************************
// *************************  == SETUP == *****************
// ********************************************************
void setup1() {

}



// ********************************************************
void setup() {

  xTaskCreatePinnedToCore(
    Core2Loop,               /* Task function. */
    "Core2Loop",                /* name of task. */
    10000,                  /* Stack size of task */
    NULL,                   /* parameter of the task */
    1,                      /* priority of the task */
    &task_Core2_Loop,            /* Task handle to keep track of created task */
    !ARDUINO_RUNNING_CORE); /* pin task to core */

  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);

  
  // setup LED for output
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);

  delay(2000);

  mserial.printStr("Starting serial...");
  mserial.start(115200);            // USB communication with mserial Monitor

  Wire.begin();
  Wire.setClock(100000);
  mserial.printStrln("OK");
  mserial.printStrln("Initializing...");
  delay(5000);
  mserial.printStr("set RTC clock to firmware Date & Time ...");  
  rtc.setTimeStruct(CompileDateTime(__DATE__, __TIME__)); 
  mserial.printStrln(rtc.getDateTime(true));
            
  mserial.printStr("Testing RGB LED OFF...");
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(ledBlue, HIGH);
  delay(5000);
  mserial.printStrln("OK");


  mserial.printStr("Testing LED Red...");
  digitalWrite(ledRed, LOW);
  delay(5000);
  digitalWrite(ledRed, HIGH);
  mserial.printStrln("OK");
  delay(1000);


  mserial.printStr("Testing LED Green...");
  digitalWrite(ledGreen, LOW);
  delay(5000);
  digitalWrite(ledGreen, HIGH);
  mserial.printStrln("OK");
  delay(1000);

  mserial.printStr("Testing LED Blue...");
  digitalWrite(ledBlue, LOW);
  delay(5000);
  digitalWrite(ledBlue, HIGH);
  mserial.printStrln("OK");
  delay(1000);

  mserial.printStrln("LED test completed.");
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(ledBlue, HIGH);
  delay(1000);

  mserial.printStrln("");

  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);  
  mserial.printStr("Mounting FAT FS Partition..."); 

  if (FFat.begin()){
      digitalWrite(ledRed, HIGH);
      digitalWrite(ledGreen, LOW);  
      mserial.printStrln(F("done."));
  }else{
    if(FFat.format()){
      digitalWrite(ledRed, HIGH);
      digitalWrite(ledGreen, LOW);  
      mserial.printStrln("Formated sucessfully!");
    }else{
      mserial.printStrln(F("fail."));
    }
  }
  delay(2000);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);  
  mserial.printStrln("");
  
  // Get all information of your FFat

  unsigned int totalBytes = FFat.totalBytes();
  unsigned int usedBytes = FFat.usedBytes();
  unsigned int freeBytes  = FFat.freeBytes();

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

  mserial.printStrln("Listing Files and Directories: ");
  // Open dir folder
  File dir = FFat.open("/");
  // Cycle all the content
  printDirectory(dir,1);
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, LOW);  
  delay(2000);
  mserial.printStrln("");

  File testFile;    
  testFile = FFat.open(F("/testCreate.txt"), "w"); 
  if (testFile){
     mserial.printStr("Writing content to a file...");
     testFile.print("Here the test text!!");
     testFile.close();
     mserial.printStrln("DONE.");
   }else{
     mserial.printStrln("Error creating file(1) !");
   }
     
  testFile = FFat.open(F("/testCreate.txt"), "r");
  if (testFile){
    mserial.printStrln("-- Reading file content (below) --");
    /**
     * File derivate from Stream so you can use all Stream method
     * readBytes, findUntil, parseInt, println etc
     */
    mserial.printStrln(testFile.readString());
    testFile.close();
    mserial.printStrln("-- Reading File completed --");
  }else{
    mserial.printStrln("Error reading file !");
  }
  mserial.printStrln("");
       
  if (scanI2C) {
    // scan I2C devices
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
    I2Cscanner();

    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
  }

  if (testSHTsensor) {
    mserial.printStr("Starting SHT31 sensor...");
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
    delay(1000);

    startSHT();
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
    delay(2000);
  }

  if (testMotionSensor) {
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
    mserial.printStr("Starting LSM6DS3 sensor...");
    delay(1000);

    if ( LSM6DS3sensor.begin() != 0 ) {
      mserial.printStrln("FAIL!");
      mserial.printStrln("Error starting the sensor at specified address");
    } else {
      mserial.printStrln("started.");
    }

    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
    delay(2000);
    mserial.printStrln("");
  }

  WiFi.mode(WIFI_STA); //  Station Mode 
  digitalWrite(ledBlue, LOW);
  WIFIscanNetworks();
  digitalWrite(ledBlue, HIGH);
  digitalWrite(ledGreen, LOW);
  delay(1000);
  
  digitalWrite(ledBlue, LOW);      
  connect2WIFInetowrk();
  digitalWrite(ledBlue, HIGH);
  digitalWrite(ledGreen, LOW);
  delay(1000);

  mserial.printStr("Get Data Validation IC Serial Number: ");
  mserial.printStrln(CryptoICserialNumber());
  delay(2000);

  mserial.printStrln("Test Random Gen: " + CryptoGetRandom());
  mserial.printStrln("");
  delay(1000);

  mserial.printStrln("Testing Experimental Data Validation HASH");
  mserial.printStrln("TEST IC => " + macChallengeDataAuthenticity("TEST IC"));
  mserial.printStrln("");
  delay(2000);

  LAST_DATASET_UPLOAD = 0;
  LAST_DATA_MEASUREMENTS = 0;

  mserial.printStrln("Time interval for dataset upload to a dataverse repository hosted by Harvard University: ");
  mserial.printStr(String(UPLOAD_DATASET_DELTA_TIME/60000));
  mserial.printStrln(" min");
  
  mserial.printStrln("Time interval for data measurements collection on each connected sensor:");
  mserial.printStr(String(DO_DATA_MEASURMENTS_DELTA_TIME/60000));
  mserial.printStrln(" min");

  LAST_DATA_MEASUREMENTS=millis();

  ESP_ERROR_CHECK(nvs_flash_erase());
  nvs_flash_init();

  mserial.printStrln("Setup completed! Running loop mode now...");
}


 
//******************************* ==  LOOP == *******************************************************

unsigned long lastMillisSensors=0;
unsigned long lastMillisWIFI=0;
bool scheduleWait = false;
int waitTimeSensors=0;
int waitTimeWIFI=0;

//************************** == Core 2 == ***********************************************************
void loop1() {
  // WIFI **********************  
  digitalWrite(ledBlue, LOW);      
  connect2WIFInetowrk();
  digitalWrite(ledBlue, HIGH);
  digitalWrite(ledGreen, LOW);
  delay(1000);

  if (UPLOAD_DATASET_DELTA_TIME < ( millis() -  LAST_DATASET_UPLOAD )){
    LAST_DATASET_UPLOAD= millis();
    digitalWrite(ledBlue, HIGH);
    digitalWrite(ledGreen, LOW);
    delay(1000);
    mserial.printStrln("");
    mserial.printStrln("Upload Data to the Dataverse...");
    while (datasetFileIsBusySaveData){
      mserial.printStr("busy S ");
      delay(500);
    }

    xSemaphoreTake(MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      datasetFileIsBusyUploadData=true;
    xSemaphoreGive(MemLockSemaphoreDatasetFileAccess); // exit critical section 
    UploadToDataverse();
    xSemaphoreTake(MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      datasetFileIsBusyUploadData=false;
    xSemaphoreGive(MemLockSemaphoreDatasetFileAccess); // exit critical section 

    digitalWrite(ledBlue, HIGH);
    digitalWrite(ledGreen, LOW);
    delay(1000);
    xSemaphoreTake(MemLockSemaphoreCore2, portMAX_DELAY); // enter critical section
      waitTimeWIFI=0;
    xSemaphoreGive(MemLockSemaphoreCore2); // exit critical section    
  }
  
  if (millis()-lastMillisWIFI > 60000){
    xSemaphoreTake(MemLockSemaphoreCore2, portMAX_DELAY); // enter critical section
      waitTimeWIFI++;
      lastMillisWIFI=millis();
    xSemaphoreGive(MemLockSemaphoreCore2); // exit critical section    
 
  }
}

// ********************* == Core 1 == ******************************  
void loop() {
  if (DO_DATA_MEASURMENTS_DELTA_TIME < ( millis() -  LAST_DATA_MEASUREMENTS ) ){
    LAST_DATA_MEASUREMENTS=millis(); 
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
    delay(1000);
    mserial.printStrln("");
    mserial.printStrln("Reading Specimen electrical response...");
    updateInternetTime();
    ReadExternalAnalogData();
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
    delay(1000);   
    
    mserial.printStrln("Saving collected data....");
    if(datasetFileIsBusyUploadData){
      mserial.printStr("file is busy....");  
    }

    if(measurements_current_pos+1 > MEASUREMENTS_BUFFER_SIZE){
      mserial.printStr("[mandatory wait]");  
      while (datasetFileIsBusyUploadData){
      }
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
 void initSaveDataset(){
    mserial.printStrln("starting");
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);    
    // SAVE DATA MEASUREMENTS ****
    xSemaphoreTake(MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      datasetFileIsBusySaveData=true;
    xSemaphoreGive(MemLockSemaphoreDatasetFileAccess); // exit critical section 
    delay(1000);
    saveDataMeasurements();
    xSemaphoreTake(MemLockSemaphoreDatasetFileAccess, portMAX_DELAY); // enter critical section
      datasetFileIsBusySaveData=false;
    xSemaphoreGive(MemLockSemaphoreDatasetFileAccess); // exit critical section 

    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);    
 }

// *********************************************************
//            Upload Dataset to Harvard's Dataverse
// *********************************************************
void UploadToDataverse() {
  //Check WiFi connection status
  if(WiFi.status() != WL_CONNECTED){
    mserial.printStrln("WiFi Disconnected");
    if (connect2WIFInetowrk()){
      UploadToDataverse();
    }
  }
  // Start sending dataset file
  File datasetFile = FFat.open("/"+EXPERIMENTAL_DATA_FILENAME, FILE_READ);
  if (!datasetFile){
    mserial.printStrln("Dataset file not found");
    return;
  }
    
  String boundary = "7MA4YWxkTrZu0gW";
  String contentType = "text/csv";
  // DATASET_REPOSITORY_URL = "/api/files/:persistentId/replace?persistentId=" +PERSISTENT_ID;
  DATASET_REPOSITORY_URL =  "/api/datasets/:persistentId/add?persistentId="+PERSISTENT_ID;
  
  String datasetFileName = datasetFile.name();
  String datasetFileSize = String(datasetFile.size());
  mserial.printStrln("Dataset File Details:");
  mserial.printStrln("Filename:" + datasetFileName);
  mserial.printStrln("size (bytes): "+ datasetFileSize);
  mserial.printStrln("");
    
  WiFiClientSecure client;

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
  // We now create a URI for the request
  mserial.printStr("Requesting URL: ");
    mserial.printStrln(DATASET_REPOSITORY_URL);

  // Make a HTTP request and add HTTP headers    
  // post header
  String postHeader = "POST " + DATASET_REPOSITORY_URL + " HTTP/1.1\r\n";
  postHeader += "Host: " + SERVER_URL + ":" + String(SERVER_PORT) + "\r\n";
  postHeader += "X-Dataverse-key: " + API_TOKEN + "\r\n";
  postHeader += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
  postHeader += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
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



  // Read all the lines of the reply from server and print them to mserial
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
    String DataHeader[NUM_DATA_READINGS];
    
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
    for (int j = 0; j < NUM_DATA_READINGS; j++) {
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
  EXPERIMENTAL_DATA_FILE = FFat.open("/" + EXPERIMENTAL_DATA_FILENAME,"a+");
  if (EXPERIMENTAL_DATA_FILE){
    mserial.printStrln("Saving data measurements to the dataset file ...");
    for (int k = 0; k <= measurements_current_pos; k++) {
      for (int i = 0; i < NUM_SAMPLES; i++) {
        String lineRowOfData="";
        for (int j = 0; j < NUM_DATA_READINGS; j++) {
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
    return true;
  }else{
    mserial.printStrln("Error creating CSV dataset file(3): " + EXPERIMENTAL_DATA_FILENAME);
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
    if (SHTavail) {  
      mserial.printStr("SHT31 data: ");
      sht31.read();
  
      float t = sht31.getTemperature();
      float h = sht31.getHumidity();
  
      if (! isnan(t)) { // check if 'is not a number'
        mserial.printStr("Temp *C = ");
        mserial.printStr(String(t));
        measurements[measurements_current_pos][i][5]=String(t);
      } else {
        mserial.printStrln("Failed to read temperature");
      }
  
      if (! isnan(h)) { // check if 'is not a number'
        mserial.printStr("   Hum. % = ");
        mserial.printStrln(String(h));
        measurements[measurements_current_pos][i][6]=String(h);
      } else {
        mserial.printStrln("Failed to read humidity");
      }
    }
 
    getLSM6DS3sensorData(i);
}


//***************************************************
void ReadExternalAnalogData() {
  int raw = 0;
  float Vout = 0;

  float VoutSum = 0;
  float R2Sum = 0;

  float R2 = 0; // to be calculated
  int counter = 0;

  mserial.printStr("Requesting measurement values and sampling ");
  mserial.printStrln(String(NUM_SAMPLES));

  byte count = 0;
  for (byte i = 0; i < NUM_SAMPLES; i++) {
    mserial.printStrln("");
    mserial.printStr(String(i));
    mserial.printStr(": ");

    raw = analogRead(EXT_IO_ANALOG_PIN);
    if (raw) {
      counter = counter + 1;

      Vout = (raw / 4094.0) * Vin;
      VoutSum = VoutSum + Vout;

      R2 = (R1 * raw) / (4094 - raw);
      R2Sum = R2Sum + R2;

      mserial.printStr("Vout: ");
      mserial.printStr(String(Vout));
      mserial.printStr(" Volt");
      mserial.printStr(" <><>  R: ");
      mserial.printStr(String(R2));
      mserial.printStrln("  Ohm");
        
      measurements [measurements_current_pos][i][0]=String(rtc.getDateTime(true));
      measurements [measurements_current_pos][i][1]=String(raw);
      measurements [measurements_current_pos][i][2]=String(Vin);
      measurements [measurements_current_pos][i][3]=String(Vout);
      measurements [measurements_current_pos][i][4]=String(R2);
      
      onBoardSensorMeasurements(i);
      
      delay(SAMPLING_INTERVAL);
    }
  }
  float AVG_Vout = VoutSum / counter;
  float AVG_R2 = R2Sum / counter;

  mserial.printStrln("");
  mserial.printStr("Total samples requested: ");
  mserial.printStrln(String(counter));

  mserial.printStr("Average Vout: ");
  mserial.printStr(String(AVG_Vout));
  mserial.printStrln(" Volt")
  ;
  mserial.printStr("Average R: ");
  mserial.printStr(String(AVG_R2));
  mserial.printStrln(" Ohm");
}


// ********************************************************
void startSHT() {
  sht31.begin(SHT31_ADDRESS);
  mserial.printStr("DONE.");
  mserial.printStr("status code: " + String(sht31.readStatus()));
  mserial.printStrln("");
  SHTavail = true;

}


// ********************************************************
void getLSM6DS3sensorData(int i) {
  //Get all parameters
  mserial.printStr("LSM6DS3 Accelerometer Data: ");
  mserial.printStr(" X1 = ");
  mserial.printStr(String(LSM6DS3sensor.readFloatAccelX()));
  measurements[measurements_current_pos][i][7]=String(LSM6DS3sensor.readFloatAccelX());
  
  mserial.printStr("  Y1 = ");
  mserial.printStr(String(LSM6DS3sensor.readFloatAccelY()));
  measurements[measurements_current_pos][i][8]=String(LSM6DS3sensor.readFloatAccelY());

  mserial.printStr("  Z1 = ");
  mserial.printStrln(String(LSM6DS3sensor.readFloatAccelZ()));
  measurements[measurements_current_pos][i][9]=String(LSM6DS3sensor.readFloatAccelZ());

  mserial.printStr("LSM6DS3 Gyroscope data:");
  mserial.printStr(" X1 = ");
  mserial.printStr(String(LSM6DS3sensor.readFloatGyroX()));
  measurements[measurements_current_pos][i][10]=String(LSM6DS3sensor.readFloatGyroX());
  
  mserial.printStr("  Y1 = ");
  mserial.printStr(String(LSM6DS3sensor.readFloatGyroY()));
  measurements[measurements_current_pos][i][11]=String(LSM6DS3sensor.readFloatGyroY());
  
  mserial.printStr("  Z1 = ");
  mserial.printStr(String(LSM6DS3sensor.readFloatGyroZ()));
  measurements[measurements_current_pos][i][12]=String(LSM6DS3sensor.readFloatGyroZ());
  
  mserial.printStr("\nLSM6DS3 Thermometer Data:");
  mserial.printStr(" Degrees C1 = ");
  mserial.printStr(String(LSM6DS3sensor.readTempC()));
  measurements[measurements_current_pos][i][13]=String(LSM6DS3sensor.readTempC());
  
  mserial.printStr("  Degrees F1 = ");
  mserial.printStrln(String(LSM6DS3sensor.readTempF()));

  mserial.printStr("LSM6DS3 SensorOne Bus Errors Reported:");
  mserial.printStr(" All '1's = ");
  mserial.printStr(String(LSM6DS3sensor.allOnesCounter));
  measurements[measurements_current_pos][i][14]=String(LSM6DS3sensor.allOnesCounter);
  
  mserial.printStr("   Non-success = ");
  mserial.printStrln(String(LSM6DS3sensor.nonSuccessCounter));
  measurements[measurements_current_pos][i][15]=String(LSM6DS3sensor.nonSuccessCounter);
  delay(1000);
}


// ********************************************************
void I2Cscanner() {
  mserial.printStrln ("I2C scanner. Scanning ...");
  byte count = 0;
  for (byte i = 8; i < 120; i++){
    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    byte error = Wire.endTransmission();
    if (error == 0) { // Receive 0 = success (ACK response)
      mserial.printStr ("Found address: ");
      mserial.printStr (String(i, DEC));
      mserial.printStr (" (0x");
      mserial.printStr (String(i, HEX));     // PCF8574 7 bit address
      mserial.printStrln (")");
      count++;
    } else if (error == 4) {
      mserial.printStr("Unknown error at address 0x");
      if (i < 16)
      mserial.printStr("0");
      mserial.printStrln(String(i, HEX));
    }
  }
  mserial.printStr ("Found ");
  mserial.printStr (String(count));        // numbers of devices
  mserial.printStrln (" device(s).");
}


// *********************************************************
//            WIFI Connectivity & Management 
// *********************************************************
bool connect2WIFInetowrk(){
  if (WiFi.status() == WL_CONNECTED)
    return true;
    
  mserial.printStrln("Connecting to the WIFI Network: \"" + String(WIFI_SSID)+"\"");
  WiFi.disconnect(true);
  
  int WiFi_prev_state=-10;
  int cnt = 0;        
  uint8_t statusWIFI=WL_DISCONNECTED;
  
  while (statusWIFI != WL_CONNECTED) {
    if (cnt == 0) {
          mserial.printStrln("WIFI Begin...");
          WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
          delay(1000);
      }
    statusWIFI = WiFi.waitForConnectResult();
    
    if(WiFi_prev_state != statusWIFI){
        WiFi_prev_state = statusWIFI;
        mserial.printStrln("("+String(statusWIFI)+"): "+get_wifi_status(statusWIFI));
    }

    mserial.printStr(".");
    if (++cnt == 10){
      mserial.printStrln("");
      mserial.printStrln("Restarting WiFi! ");
       
      WiFi.disconnect(true);
      WIFIscanNetworks();
        
      cnt = 0;
    }
  }   
  return true;
}

// ********************************************************
String get_wifi_status(int status){
    switch(status){
        case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS(0): WiFi is in process of changing between statuses";
        case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED(2): Successful connection is established";
        case WL_NO_SSID_AVAIL:
        return "WL_NO_SSID_AVAIL(1): SSID cannot be reached";
        case WL_CONNECT_FAILED:
        return "WL_CONNECT_FAILED (4): Password is incorrect";
        case WL_CONNECTION_LOST:
        return "WL_CONNECTION_LOST (5)";
        case WL_CONNECTED:
        return "WL_CONNECTED (3)";
        case WL_DISCONNECTED:
        return "WL_DISCONNECTED (6): Module is not configured in station mode";
    }
}

// *********************************************************
void WIFIscanNetworks(){
  if (WiFi.status() == WL_CONNECTED)
    return;
    
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0) {
    mserial.printStrln("no networks found");
  } else {
    mserial.printStr(String(n));
    mserial.printStrln(" WiFi Networks found:");
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
  }  
}

// *************************************************************
void WiFiEvent(WiFiEvent_t event) {  
  switch (event) {
    case SYSTEM_EVENT_WIFI_READY: 
      mserial.printStrln("WiFi interface ready");
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      mserial.printStrln("Completed scan for access points");
      break;
    case SYSTEM_EVENT_STA_START:
      mserial.printStrln("WiFi client started");
      break;
    case SYSTEM_EVENT_STA_STOP:
      mserial.printStrln("WiFi clients stopped");
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      mserial.printStrln("Connected to access point");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      xSemaphoreTake(MemLockSemaphoreWIFI, portMAX_DELAY); // enter critical section
          WIFIconnected=false;
      xSemaphoreGive(MemLockSemaphoreWIFI); // exit critical section
      mserial.printStrln("Disconnected from WiFi access point");
      //WiFi.begin(ssid, password);
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      mserial.printStrln("Authentication mode of access point has changed");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      mserial.printStrln("Connection Details");
      mserial.printStrln("     IP     : "+WiFi.localIP().toString());
      mserial.printStrln("     Gateway: "+WiFi.gatewayIP().toString());

      if(!Ping.ping("www.google.com", 3)){
        mserial.printStrln("no Internet connectivity found.");
      }else{
        mserial.printStrln("Connection has Internet connectivity.");
        //init time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        updateInternetTime();
      }
      xSemaphoreTake(MemLockSemaphoreWIFI, portMAX_DELAY); // enter critical section
        WIFIconnected=true;
      xSemaphoreGive(MemLockSemaphoreWIFI); // exit critical section  
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
}}

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
