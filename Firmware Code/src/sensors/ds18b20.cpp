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

#include "ds18b20.h"
#include "Arduino.h"

DS18B20_SENSOR::DS18B20_SENSOR() {
  this->EXT_IO_ANALOG_PIN=2;
  this->numSensors=1;
  this->measurement = new float[numSensors];
  this->measurement_label = new String[1] {"Temperature"};
}

void DS18B20_SENSOR::init(INTERFACE_CLASS* interface, uint8_t EXT_IO_ANALOG_PIN){
  this->interface=interface;
  this->interface->mserial->printStr("\ninit DS18B20 sensor ...");
  this->EXT_IO_ANALOG_PIN = EXT_IO_ANALOG_PIN;

  this->onewire = OneWire(this->EXT_IO_ANALOG_PIN); 
  this->sensors= DallasTemperature(&onewire);    
  this->sensors.begin();    // initialize the DS18B20 sensor

  this->interface->mserial->printStrln("done.");
}
// *************************************************
 bool DS18B20_SENSOR::requestMeasurements(){
    this->sensors.requestTemperatures(); // Send the command to get temperature readings         
    this->measurement[0] = this->sensors.getTempCByIndex(0);
    return true;
 }

// ************************************************
void DS18B20_SENSOR::ProbeSensorStatus(uint8_t sendTo){
    String dataStr="";
    this->sensorAvailable= true; 
    
    dataStr= this->interface->DeviceTranslation("num_temp_probes") + " " + String(this->sensors.getDS18Count(), DEC) +"\n";

    // report parasite power requirements
    dataStr += this->interface->DeviceTranslation("parasite_power") + ": "; 
    if (this->sensors.isParasitePowerMode()) dataStr +="ON\n";
    else dataStr += "OFF\n";

    this->onewire.reset_search();
    // assigns the first address found to insideThermometer
    if ( !this->onewire.search(this->insideThermometer) ){ 
      dataStr += this->interface->DeviceTranslation("unable_find_probe") + " ?\n";
      this->sensorAvailable= false; 
    }

    // show the addresses we found on the bus
    dataStr += this->interface->DeviceTranslation("probe_addr") + ": ";
    for (uint8_t i = 0; i < 8; i++)
    {
    if (this->insideThermometer [i] < 16)  dataStr += "0";
        dataStr += String(this->insideThermometer[i], HEX) ;
    }

    // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
    this->sensors.setResolution(this->insideThermometer, 9);


    dataStr += "\n " +  this->interface->DeviceTranslation("probe_resolution") + ": ";
    dataStr += String(this->sensors.getResolution(this->insideThermometer), DEC) + "\n\n"; 
    
    this->interface->sendBLEstring( dataStr, sendTo);  
}

// *********************************************************
 bool DS18B20_SENSOR::helpCommands(uint8_t sendTo ){
    String dataStr="GBRL commands:\n" \
                    "$help $?                           - View available GBRL commands\n" \
                    "$lang set [country code]           - Change the smart device language\n\n";

    this->interface->sendBLEstring( dataStr,  sendTo ); 
    return false; 
 }
// ******************************************************************************************

bool DS18B20_SENSOR::commands(String $BLE_CMD, uint8_t sendTo ){
  String dataStr="";
  if($BLE_CMD.indexOf("$lang dw ")>-1){

  }

  return false;
}




