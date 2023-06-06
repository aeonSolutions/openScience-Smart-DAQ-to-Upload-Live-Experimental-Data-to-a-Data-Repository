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

#include "sht3x.h"
#include "Arduino.h"

SHT3X_SENSOR::SHT3X_SENSOR() {
  this->SHT3X_ADDRESS = 0x38;
  this->numSensors=2;
  this->measurement = new float[numSensors];
  this->measurement_label = new String[2] {"Temperature", "Humidity"};
}

void SHT3X_SENSOR::init(INTERFACE_CLASS* interface, uint8_t SHT3X_ADDRESS){
  this->interface=interface;
  this->interface->mserial->printStr("\ninit AHT20 sensor ...");
  this->SHT3X_ADDRESS = SHT3X_ADDRESS;

  this->sht3x= new SHT31();
  this->startSHT3X();

  this->interface->mserial->printStrln("done.");
}

// ********************************************************
void SHT3X_SENSOR::startSHT3X() {
    Wire.begin();

    bool result = this->sht3x->begin();  
    if (result){
        this->sensorAvailable = true;
        this->interface->mserial->printStr("SHT3x sensor status code: " + String(this->sht3x->readStatus()));
    }else{
        this->sensorAvailable = false;
        this->interface->mserial->printStrln("SHT3x sensor not found at specified address (0x"+String(this->SHT3X_ADDRESS, HEX)+")");
        this->interface->onBoardLED->led[0] = interface->onBoardLED->LED_RED;
        this->interface->onBoardLED->statusLED(100,2); 
    }
}

// *************************************************
 bool SHT3X_SENSOR::requestMeasurements(){
    if (this->sensorAvailable == false) {
      startSHT3X(); 
    }  

    //this->sht3x.read();

    float sht_temp = sht3x->getTemperature();
    float sht_humidity = sht3x->getHumidity();

    if (isnan(sht_temp)) { // check if 'is not a number'
      this->interface->mserial->printStrln("Failed to read temperature");
      this->measurement[0] = -500;
    }else{
      this->measurement[0] = sht_temp;
    }

    if (isnan(sht_humidity)) { // check if 'is not a number'
      this->interface->mserial->printStrln("Failed to read humidity");
      this->measurement[1] = -500;
    }else{
      this->measurement[1] = sht_humidity;
    }

    return true;
 }

// *********************************************************
 bool SHT3X_SENSOR::helpCommands(uint8_t sendTo ){
    String dataStr="GBRL commands:\n" \
                    "$help $?                           - View available GBRL commands\n" \
                    "$lang set [country code]           - Change the smart device language\n\n";

    this->interface->sendBLEstring( dataStr,  sendTo ); 
    return false; 
 }
// ******************************************************************************************

bool SHT3X_SENSOR::commands(String $BLE_CMD, uint8_t sendTo ){
  String dataStr="";
  if($BLE_CMD.indexOf("$lang dw ")>-1){

  }

  return false;
}