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

#include "aht20.h"
#include "Arduino.h"

AHT20_SENSOR::AHT20_SENSOR() {
  this->AHT20_ADDRESS = 0x38;
  this->numSensors=2;
  this->measurement = new float[numSensors];
  this->measurement_label = new String[2] {"Temperature", "Humidity"};
}

void AHT20_SENSOR::init(INTERFACE_CLASS* interface, uint8_t AHT20_ADDRESS){
  this->interface=interface;
  this->interface->mserial->printStr("\ninit AHT20 sensor ...");
  this->AHT20_ADDRESS = AHT20_ADDRESS;

  this->aht20= new AHT20(this->AHT20_ADDRESS);
  this->startAHT();

  this->interface->mserial->printStrln("done.");
}

// ********************************************************
void AHT20_SENSOR::startAHT() {
  
    Wire.begin();

    bool result = this->aht20->begin();  
    if (result){
        this->sensorAvailable = true;
        this->interface->mserial->printStr("AHT sensor status code: " + String(this->aht20->getStatus()));
        this->interface->mserial->printStrln(" <<>> calibrated: " + String( this->aht20->isCalibrated() == 1 ? "True" : "False" ) );
    }else{
        this->sensorAvailable = false;
        this->interface->mserial->printStrln("AHT sensor not found at specified address (0x"+String(this->AHT20_ADDRESS, HEX)+")");
        this->interface->onBoardLED->led[0] = interface->onBoardLED->LED_RED;
        this->interface->onBoardLED->statusLED(100,2); 
    }
}

// *************************************************
 bool AHT20_SENSOR::requestMeasurements(){
    if (this->sensorAvailable == false) {
      startAHT(); 
    }  

    float aht_temp = aht20->getTemperature();
    float aht_humidity = aht20->getHumidity();

    if (isnan(aht_temp)) { // check if 'is not a number'
      this->interface->mserial->printStrln("Failed to read temperature");
      this->measurement[0] = -500;
    }else{
      this->measurement[0] = aht_temp;
    }

    if (isnan(aht_humidity)) { // check if 'is not a number'
      this->interface->mserial->printStrln("Failed to read humidity");
      this->measurement[1] = -500;
    }else{
      this->measurement[1] = aht_humidity;
    }

    return true;
 }


// *********************************************************
 bool AHT20_SENSOR::helpCommands(uint8_t sendTo ){
    String dataStr="GBRL commands:\n" \
                    "$help $?                           - View available GBRL commands\n" \
                    "$lang set [country code]           - Change the smart device language\n\n";

    this->interface->sendBLEstring( dataStr,  sendTo ); 
    return false; 
 }
// ******************************************************************************************

bool AHT20_SENSOR::commands(String $BLE_CMD, uint8_t sendTo ){
  String dataStr="";
  if($BLE_CMD.indexOf("$lang dw ")>-1){

  }

  return false;
}




