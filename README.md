[![Telegram](https://img.shields.io/badge/join-telegram-blue.svg?style=for-the-badge)](https://t.me/+W4rVVa0_VLEzYmI0)
 [![WhatsApp](https://img.shields.io/badge/join-whatsapp-green.svg?style=for-the-badge)](https://chat.whatsapp.com/FkNC7u83kuy2QRA5sqjBVg) 
 [![Donate](https://img.shields.io/badge/donate-$-brown.svg?style=for-the-badge)](http://paypal.me/mtpsilva)
 [![Say Thanks](https://img.shields.io/badge/Say%20Thanks-!-yellow.svg?style=for-the-badge)](https://saythanks.io/to/mtpsilva)
![](https://img.shields.io/github/last-commit/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo?style=for-the-badge)
<a href="https://trackgit.com">
<img src="https://us-central1-trackgit-analytics.cloudfunctions.net/token/ping/l95wbzce6vrs7sr48tbl" alt="trackgit-views" />
</a>
![](https://views.whatilearened.today/views/github/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo.svg)



## Proof of Concept of a Smart DAQ Device able to Upload Live Experimental Sensor Data to a Data Repository with a Unique Data Fingerprint

This paper discusses an innovative proof-of-concept method for management of collected experimental data in real-time and compatible with any open environment. The proposed proof-of-concept smart DAQ device has the minimum hardware characteristics to handle data measurements collection from sensors locally connected to it, store it on a local CSV or SQLite database file and finally connect and synchronize data measurements collected with a data repository hosted remotely on a Dataverse. 

This paper is currently being written, in an open enviroment format. Available as a prePrint draft document at Elsevier's SSRN platform.
https://ssrn.com/abstract=4210504 

<br>
<br>

**Status**

This project is waiting for parts to arrive from aliExpress. Will resume at the end of November 2022. In 4 weeks time.

<br>

**version and revision history**

- ToDo (firmware):
  - Calc MD5 hash of the dataset file yo compare with json result on upload 
  - OTA firmware update
  - Remove lock on a dataset (by an admin)
  - Validate Json received on a new dataset upload
  - Load dataset repository metadata
  - Output board startup diagnostics serial stream to a Bluetooth or WIFI data strean
  - accecpt GBRL like setup and config $ commands.
  - SQLite dataset 

<br>
<br>

## Dataverse API C library

In paralell is being written a C library to expedite API integration on smart DAQ devices or elsewhere. Follow the link to its repository:

https://github.com/aeonSolutions/OpenScience-Dataverse-API-C-library


[![](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/dataverse_r_project.png)](https://github.com/aeonSolutions/OpenScience-Dataverse-API-C-library)

<br>

**Dataverse API in another coding language**

Goto dataverse.org for another coding language that best suits your coding style and needs. Currently there are client libraries for Python, Javascript, R, Java, and Julia that can be used to develop against Dataverse Software APIs

https://guides.dataverse.org/en/5.12/api/client-libraries.html

<br>
<br>

![](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/12bitSmartDAQoctober.jpeg)

On the photo above the smart DAQ is installed on an acrylic case and screwed with plastic screws to an acrylic base with the same cross section area as the specimen to be tested. 

The acrylic base can be bought [here](https://s.click.aliexpress.com/e/_DEGsZaL). And the acrylic case [here](https://s.click.aliexpress.com/e/_Dmudkjt). 

<br>

![](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/smartAsphaltSample.jpeg)

On the photo above is one of my many specimens I purposely fabricated to research self-sensing properties of a asphalt mixed with a known content of carbon fibers. This is a 10cm cylinder specimen and on the top is already setup my own design smart #DAQ (get it here on my GitHub ) with ability do upload LIVE experimental data to a #dataverse.
<br>


## Smart DAQ for LIVE Experimental Data
This repository holds the firmware C code compatible with Tensilica's Xtensa LX6/7 microprocessors. Files are located in the folder "Firmware Code".

<br>
<br>

### Smart PCB Hardware Specifications
The hardware specifications for the 12bit pcb with dimensions of 23.5x43.5mm are the following:


- QFN 56 Dual Xtensa LX7 Core Processors running up to 240MHz
  -	RISC V ultra-low power co-processor
  - 512Kb RAM (PSRAM max 1 Gb);
  - 16Mb SOIC 8 NOR SPI Flash Memory (max 1Gb);
  - 2.4GHz ISM wireless connectivity;
  - Up to 118 12bit ADC Multiplexed DAQ channels;
- Authentication & Security:
  - SOIC-8 ATSHA204A SHA-256 high-security hardware authentication IC for secure and unique communication between devices;
- Power management
  - DFN-6 AUR9718 high efficiency step-down 3.3V 1.5A DC converter;
- Onboard sensors:
  - DFN-8 SHT3.x; temperature sensor with a precision of 1.5C;
  - DFN-8 SHT3.x humidity sensor;
  - LGA-14 LSM6DS3 a 6-axis accelerometer and gyroscope;
  - reference voltage sessor calibration wiht temperature and humidity
- 	External connectivity for up to 118 sensors:
  -  1x I2C pin terminal connector (shared)
  -  1x 12 to 16bit (oversampling & digitization) digital terminal connector (shared)
  -  1x 12 to 16bit (oversampling & digitization) analog terminal connector (shared) with manual scale selection ohmeter via jumper pin
  -  12 to 16bit (oversampling & digitization) voltage reference analyser with temperature compensation 

<br>

**Smart PCB design wiht a built in 1.69" LCD TFT IPS display**

Another version of this smart DAQ will be available next December that includes built in a 1.69" LCD TFT IPS display with ability to display LIVE experimental data being collected.. Stay tuned for updates. 

<br>
<br>

 **In real life this smart DAQ is able to:**
- connect to all kinds of 3V to 5V Digital sensors;
- connect to all kinds of sensors compatible with the I2C protocol. (max 118 sensors simultaneously)
- measure voltage in the range of  [0;3.3V]
- measure electrical resistance [0; 10^6] Ohm 
- do temperature and humidity compensation on all measurements 
- has a voltage reference sensor for improved accuracy on ADC measurements  
- has a motion sensor to know if anyone moved a specimen during an experiment

<br>
<br>

## PCB circuit Schematic 
The PCB circuit schematic is available in PDF located in the folder "PCB Schematic"

https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/tree/main/PCB%20Schematic

## PCB design files
The PCB gerber files are located in the folder "PCB Gerber Files" and are ready to order online on any pcb fabrication store. (PCBWay, AllPCB, Eurocircuits, etc)
<br>
<br>
other open hardware PCB electronics are also available on my PCB catalogue that maybe worth have a look.  The catalogue includes the KiCad project files of similar smart DAQ compatible with the firmware code available on this repository:

https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/blob/main/Smart%20DAQ/README.md

![](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/LDAD%20fron.png)
![](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/LDAD%20back.png)

(November pcb revision)
<br>
<br>

## Proof of Concept

To test and validate proposed smart DAQ PCB electronics and its firmware as a solution for LIVE experimental data measurements on any test specimen part of a experimental campaign, This PCB electronics is being used to measure a predefined set of variables/parameters to further study several asphalt mixtures with known carbon fiber weight content in the asphalt matrix. Below is a YouTube link to an unedited short video showing one of the experimental setups.

[![](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/youtube.png)](https://youtu.be/ycCJRiwse2M)


<br />
<br />
<br />

______________________________________________________________________________________________________________________________
### License
2022 Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International

https://creativecommons.org/licenses/by-nc-sa/4.0/

______________________________________________________________________________________________________________________________

<br />
<br />

### Be supportive of my dedication and work towards technology education and buy me a cup of coffee
Buy me a cup of coffee, a slice of pizza or a book to help me study, eat and think new PCB design files.

[<img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" data-canonical-src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" height="70" />](https://www.buymeacoffee.com/migueltomas)

<br />
<br />

### Make a donation on Paypal
Make a donation on paypal and get a TAX refund*.

[![](https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/blob/main/paypal_small.png)](http://paypal.me/mtpsilva)


### Support all these open hardware projects and become a patreon  
Liked any of my PCB KiCad Designs? Help and Support my open work to all by becomming a LDAD Patreon.
In return I will give a free PCB design in KiCad to all patreon supporters. To learn more go to patreon.com. Link below.

[![](https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/blob/main/patreon_small.png)](https://www.patreon.com/ldad)
