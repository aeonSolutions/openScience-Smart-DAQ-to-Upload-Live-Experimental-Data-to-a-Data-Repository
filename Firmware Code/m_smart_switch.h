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


#include "mserial.h"
#include "interface_class.h"
#include <WebServer.h>
#include "m_file_class.h"
#include "onboard_sensors.h"
#include "Config.h"


#ifndef SMART_SWITCH_DEF
  #define SMART_SWITCH_DEF

#define SAVED_OR_DEFAULT_ROOM_NAME(string) (strlen(string) == 0 ? DEFAULT_ROOM_NAME : string)

class SMART_SWITCH_CLASS {
  private:

  public:
    INTERFACE_CLASS* interface=nullptr;
    FILE_CLASS* drive = nullptr;
    WebServer*  server = nullptr;
    ONBOARD_SENSORS* onBoardSensors = nullptr;

    // PCB board specific charateristics
    float ONBOARD_VOLTAGE = 3.3;
    int ANALOG_RESOLUTION = 4095;

    uint8_t POWER_CONSUMPTION_IO;
    uint8_t RELAY_SWITCH_IO;
    /*
    ACS712 Voltage Output
        1/2 VCC when no load or it is 2.5V since we use 5V as VCC.
        5A type generate 185mV every amp current
        20A type generate 100mV every amp current
        30A type generate 66mV every amp current
    */
    float ACS_712_AMP;
    float powerConsumption;

    bool LampSwitchIs; 
    char* weather;

    SMART_SWITCH_CLASS();

    void init(INTERFACE_CLASS* interface, FILE_CLASS* drive , WebServer* server, ONBOARD_SENSORS* onBoardSensors);
    
    void handleLamp();
    const char* getLampStatus();
    const char* getLampState();
    void updatePowerConsumptionData();
    void handleCommands();
};

#endif