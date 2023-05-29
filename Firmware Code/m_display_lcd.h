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
#include "Arduino.h"
#include "interface_class.h"
#include <TFT_eSPI.h>                                            
#include "m_wifi.h"

#ifndef DISPLAY_LCD  
  #define DISPLAY_LCD

  // LCD  ***************************************************** 
    // ST7789 TFT module connections : every pcb needs to edit pins on the file UserSetup.h (see library)
    /*
    #define TFT_MOSI 8  // SDA Pin on ESP32
    #define TFT_SCLK 9  // SCL Pin on ESP32
    #define TFT_CS   37  // Chip select control pin
    #define TFT_DC   36  // Data Command control pin
    #define TFT_RST  38  // Reset pin (could connect to RST pin)
    #define TFT_BUSY 1  // busy pin
    */

    #define TFT_PORTRAIT_FLIPED 4

  class DISPLAY_LCD_CLASS {
    private:
        INTERFACE_CLASS* interface=nullptr;
        M_WIFI_CLASS* mWifi=nullptr;
      
        String oldText="";
        String oldTFTtext="";
        int oldPosX=0;
        int oldPosY=0;
        uint8_t oldTextsize=2;

    public:
      bool TEST_LCD;

      int8_t LCD_BACKLIT_LED;

      // TFT screen resolution
      static constexpr int LCD_TFT_WIDTH = 240;
      static constexpr int LCD_TFT_HEIGHT = 280; 

      // TFT orientation modes
      static constexpr uint8_t LCD_TFT_LANDSCAPE = 2;
      static constexpr uint8_t LCD_TFT_LANDSCAPE_FLIPED = 3;
      static constexpr uint8_t LCD_TFT_PORTRAIT = 1;
      static constexpr uint8_t LCD_TFT_PORTRAIT_FLIPED = 0;

      int TFT_CURRENT_X_RES;
      int TFT_CURRENT_Y_RES;
      
      TFT_eSPI tft = TFT_eSPI();    

      DISPLAY_LCD_CLASS();
      void init(INTERFACE_CLASS* interface,  M_WIFI_CLASS* mWifi);

      void testfastlines(uint16_t color1, uint16_t color2);
      void loadStatus(String text);
      void tftPrintText(int x, int y, char text[], uint8_t textSize =2 , char align[] = "custom", uint16_t color=TFT_WHITE, bool deletePrevText=false);
      void TFT_setScreenRotation(uint8_t rotation);
};

#endif
