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
#include "m_display_lcd.h"
#include "Arduino.h"
// AeonLabs Logo 
#include "lcd_aeonlabs_science_logo.h"

DISPLAY_LCD_CLASS::DISPLAY_LCD_CLASS() {
  this->TEST_LCD=false;
  this->LCD_BACKLIT_LED=35;
}

// *************************************************************************************
void DISPLAY_LCD_CLASS::init(INTERFACE_CLASS* interface, M_WIFI_CLASS* mWifi){
  this->interface=interface;
  this->interface->mserial->printStr("\nInit LCD Display Library ...");
  this->mWifi= mWifi;

  // LCD backligth
  pinMode(this->LCD_BACKLIT_LED, OUTPUT);


  this->tft.begin ();                                          // initialize a ST7789 chip
  this->tft.setSwapBytes (true);                      // swap the byte order for pushImage() - corrects endianness
  this->oldText="";
  
  /*
  TextSize(1) The space occupied by a character using the standard font is 6 pixels wide by 8 pixels high.
  A two characters string sent with this command occup a space that is 12 pixels wide by 8 pixels high.
  Other sizes are multiples of this size.
  Thus, the space occupied by a character whith TextSize(3) is 18 pixels wide by 24 pixels high. The command only excepts integers are arguments
  See more http://henrysbench.capnfatz.com/henrys-bench/arduino-adafruit-gfx-library-user-guide/arduino-adafruit-gfx-printing-to-the-tft-screen/
  */

  TFT_setScreenRotation(LCD_TFT_LANDSCAPE_FLIPED);
  this->tft.fillScreen(TFT_BLACK);

  if(this->TEST_LCD){
    this->interface->mserial->printStrln("LCD optimized lines red blue...");
    // optimized lines
    digitalWrite(LCD_BACKLIT_LED, HIGH); // Enable LCD backlight 
    this->testfastlines(TFT_RED, TFT_BLUE);
    delay(500);

    this->interface->mserial->printStr("Hello! ST77xx TFT Test...");
    this->interface->mserial->printStr("Black screen fill: ");
    
    uint16_t time = millis();
    this->tft.fillScreen(TFT_BLACK);
    time = millis() - time;
    
    this->interface->mserial->printStrln(String(time/1000)+" sec. \n");
    delay(5000);
  }
  
  this->tft.pushImage (10,65,250,87,AEONLABS_16BIT_BITMAP_LOGO);

  // PosX, PoxY, text, text size, text align, text color, delete previous text true/false
  this->tftPrintText(0,160,"Loading...",2,"center", TFT_WHITE, false);

  delay(5000);

  this->interface->mserial->printStrln("done.");
}

// *********************************************************

// **************** LCD ****************************

void DISPLAY_LCD_CLASS::testfastlines(uint16_t color1, uint16_t color2) {
  if (this->LCD_DISABLED)
    return;
  this->tft.fillScreen(TFT_BLACK);
  for (int16_t y=0; y < this->tft.height(); y+=5) {
    this->tft.drawFastHLine(0, y, this->tft.width(), color1);
  }
  for (int16_t x=0; x < this->tft.width(); x+=5) {
    this->tft.drawFastVLine(x, 0, this->tft.height(), color2);
  }
}

// ***************************************************************
void DISPLAY_LCD_CLASS::loadStatus(String text){
  if (this->LCD_DISABLED)
    return;
  this->tft.setTextColor(TFT_BLACK);
  this->tft.setTextSize(2);
  this->tft.setCursor(80, 120);
  this->tft.println(oldText);
  this->tft.setTextColor(TFT_BLACK);
  this->tft.println(text); 
  this->oldText=text;
}

//************************************************************
void DISPLAY_LCD_CLASS::tftPrintText(int x, int y, char text[], uint8_t textSize , char align[], uint16_t color, bool deletePrevText){
    if (this->LCD_DISABLED)
      return;
    /*
    align: left, center, right
    */
    int posX=x;
    int posY=y;
    uint8_t pad=3; // pixel padding from the border
    uint8_t TEXT_CHAR_WIDTH=6;

    if (align=="center"){
      posX= (this->TFT_CURRENT_X_RES/2) - strlen(text)*TEXT_CHAR_WIDTH*textSize/2;
    }else if (align=="left"){
      posX=pad;
    } else if (align=="right"){
      posX=this->TFT_CURRENT_X_RES - strlen(text)*6*textSize-pad;
    }

    if (strlen(text)*6*textSize>this->TFT_CURRENT_X_RES)
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
      this->tft.setTextColor(TFT_BLACK);
      this->tft.setTextSize(oldTextsize);
      this->tft.drawString(this->oldTFTtext, oldPosX, oldPosY);
    }

    this->tft.setTextSize(textSize);
    this->tft.setTextColor(color);
          this->interface->mserial->printStrln("here 6.3");
    this->tft.drawString(text, posX, posY);

    this->oldTFTtext=text;
    this->oldPosX=posX;
    this->oldPosY=posY;
    this->oldTextsize=textSize; 
  
        this->interface->mserial->printStrln("here 11");
  }


//**********************************************
void DISPLAY_LCD_CLASS::TFT_setScreenRotation(uint8_t rotation){
  if (this->LCD_DISABLED)
    return;

  this->tft.setRotation(rotation);
  switch(rotation){
      case this->LCD_TFT_LANDSCAPE:
        this->TFT_CURRENT_X_RES=this->LCD_TFT_HEIGHT;
        this->TFT_CURRENT_X_RES= this->LCD_TFT_WIDTH;
        break;

      case this->LCD_TFT_LANDSCAPE_FLIPED:
        this->TFT_CURRENT_X_RES=this->LCD_TFT_HEIGHT;
        this->TFT_CURRENT_X_RES= this->LCD_TFT_WIDTH;
        break;

      case this->LCD_TFT_PORTRAIT:
        this->TFT_CURRENT_X_RES=this->LCD_TFT_HEIGHT;
        this->TFT_CURRENT_X_RES= this->LCD_TFT_HEIGHT;
        break;
        
      case this->LCD_TFT_PORTRAIT_FLIPED:
        this->TFT_CURRENT_X_RES=this->LCD_TFT_HEIGHT;
        this->TFT_CURRENT_X_RES= this->LCD_TFT_HEIGHT;
        break;
  }
}

// ***************************************************