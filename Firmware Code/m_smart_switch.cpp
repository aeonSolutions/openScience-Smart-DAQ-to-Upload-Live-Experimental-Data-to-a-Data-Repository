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

#include "m_smart_switch.h"
#include "interface_class.h"
#include "onboard_led.h"
#include "m_math.h"

SMART_SWITCH_CLASS::SMART_SWITCH_CLASS(){
  //Weather service
  this->weather = (char*) malloc(sizeof(char) * 64);
  strcpy_P(this->weather, PSTR("Unknown"));
}

void SMART_SWITCH_CLASS::init(INTERFACE_CLASS* interface, FILE_CLASS* drive , WebServer* server, ONBOARD_SENSORS* onBoardSensors){
    this->interface = interface;
    this->drive = drive;
    this->server = server;
    this->onBoardSensors = onBoardSensors;

    pinMode(this->RELAY_SWITCH_IO,OUTPUT);		//set pin RELAY_SWITCH_IO as OUTPUT
    digitalWrite(this->RELAY_SWITCH_IO, HIGH);    // set relay to OFF position

    pinMode(this->POWER_CONSUMPTION_IO, INPUT);

    this->ONBOARD_VOLTAGE = 3.3;
    this->ANALOG_RESOLUTION = 4095;
    this->ACS_712_AMP = (0.185*ONBOARD_VOLTAGE)/5.0 ; // 5A IC
}
// ***************************************************************


void SMART_SWITCH_CLASS::handleLamp(){
  this->interface->mserial->printStrln("");
  this->interface->mserial->printStr(" request to switch LAMP is... ");  
  this->interface->mserial->printStr(" request to switch LAMP ("+String(this->server->argName(0))+")("+String(this->server->arg(0))+")... ");

  String switchState;
  switchState = this->server->arg("input");
  this->interface->mserial->printStrln("SwitchSate:"+String(switchState));
  
  if(switchState=="1" || switchState=="0"){
    this->interface->mserial->printStr(" LAMP switch is : ");
    if(switchState=="1"){
      this->interface->mserial->printStrln(" ON");
      this->LampSwitchIs=true;
      digitalWrite(this->RELAY_SWITCH_IO, LOW);
    }else{
      this->LampSwitchIs=false;
      this->interface->mserial->printStrln(" OFF");
      digitalWrite(this->RELAY_SWITCH_IO, HIGH);
    }
  }
  this->server->send(
    200,
    F("application/json"),
    F(
      (String("{\"toast\":\"LAMP is ")+String(getLampStatus())+String("\"}")).c_str()
    )
  );

}
// ***************************************************************

const char* SMART_SWITCH_CLASS::getLampStatus(){

  if(this->LampSwitchIs){
      return "ON";
  }else{
    return "OFF";
  }
}
// ***************************************************************

const char* SMART_SWITCH_CLASS::getLampState(){

  if(this->LampSwitchIs){
      return "1";
  }else{
    return "0";
  }
}
// ***************************************************************

void SMART_SWITCH_CLASS::updatePowerConsumptionData() {
  unsigned int x=0;
  float AcsValue=0.0, Samples=0.0, AvgAcs=0.0;
  uint8_t numSamples=150;
  uint8_t counter=0;

    for (int x = 0; x < numSamples; x++){ //Get 150 samples
    AcsValue = analogRead(POWER_CONSUMPTION_IO);     //Read current sensor values   
    if (!isnan(AcsValue)){
      Samples = Samples + AcsValue;  //Add samples together
      counter=counter+1;
    }
    delay (3); // let ADC settle before next sample 3ms
  }
  AvgAcs=Samples/counter;//Taking Average of Samples

  //((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
  //2.5 is offset(I assumed that arduino is working on 5v so the viout at no current comes
  //out to be 2.5 which is out offset. If your arduino is working on different voltage than 
  //you must change the offset according to the input voltage)
  //0.185v(185mV) is rise in output voltage when 1A current flows at input
  this->powerConsumption = ((this->ONBOARD_VOLTAGE/2) - (AvgAcs * (this->ONBOARD_VOLTAGE / this->ANALOG_RESOLUTION)) )/ this->ACS_712_AMP;

  this->interface->mserial->printStr(String(this->powerConsumption)+" Amp. >> ");//Print the read current on Serial monitor
}
// ***************************************************************

void SMART_SWITCH_CLASS::handleCommands() {
  
  this->onBoardSensors->request_onBoard_Sensor_Measurements();
  this->updatePowerConsumptionData();

  char* roomName = (char*) this->drive->readFile("/room_name.txt").c_str();
  char* weatherDisplay = (char*) this->drive->readFile("/weather.txt").c_str();
  char* message = (char*) malloc(sizeof(char) * 512);
  sprintf_P(
    message,
    PSTR(
      "{"
        "\"commands\":{"
          "\"electricity\":{\"icon\": \"electricity\",\"title\":\"Power consumption\",\"summary\":\"Power Consumption is %s A\", \"mode\": \"none\"},"
          "\"lamp\":{\"icon\": \"lamp\",\"title\":\"Living room lighting\",\"summary\":\"Switch on the bedroom entrance\", \"mode\": \"switch\", \"data\":{\"input\": \" %s\"}},"
          "\"temperature\":{\"icon\": \"thermometer\",\"title\":\"%s °C\",\"summary\":\"Temperature in the %s\", \"mode\": \"none\"},"
          "\"humidity\":{\"icon\": \"hygrometer\",\"title\":\"%s %%\",\"summary\":\"Humidity in the  %s\", \"mode\": \"none\"}"
    ),
    String(this->powerConsumption,2).c_str(),
    this->getLampState(),
    String(this->onBoardSensors->aht_temp,2).c_str(),
    SAVED_OR_DEFAULT_ROOM_NAME(roomName),
    String(this->onBoardSensors->aht_humidity,2).c_str(),
    SAVED_OR_DEFAULT_ROOM_NAME(roomName)
  );
  if (strcmp(weatherDisplay, "1") == 0) {
    sprintf_P(
      message + strlen(message),
      PSTR(
        ","
        "\"weather\":{\"icon\": \"gauge\",\"title\":\"Weather\",\"summary\":\"%s\", \"mode\": \"none\"}"
      ),
      this->weather
    );
  }
  strcat_P(message, PSTR("}}"));

  //server.keepAlive(false);
  this->server->send(200, F("application/json"), message);
  //free(roomName);
  //free(weatherDisplay);
  //free(message);
}
