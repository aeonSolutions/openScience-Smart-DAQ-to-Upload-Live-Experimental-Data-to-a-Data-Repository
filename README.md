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

This paper discusses an innovative proof-of-concept method for management of collected experimental data in real-time and compatible with any open environment. The proposed proof-of-concept smart DAQ device has the minimum hardware caracteristics to handle data measurements collection from sensors locally connected to it, store it on a local CSV or SQLite database file and finally connect and synchronize data measurements collected with a data repository hosted remotely on a Dataverse. 

This paper is currently being written, in an open enviroment format. Available as a prePrint draft document at Elsevier's SSRN platform.
https://ssrn.com/abstract=4210504 

<br>
<br>

**version and revision history**
- ToDo:
  - Calc MD5 hash of the dataset file yo compare with json result on upload 
  - OTA firmware update
  - Remove lock on a dataset (by an admin)
  - Validate Json received on a new dataset upload
  - Load dataset repository metadata
  - Output board startup diagnostics serial stream to a Bluetooth or WIFI data strean
  - accecpt GBRL like setup and config $ commands.

<br>
<br>

## Smart DAQ for LIVE Experimental Data
This repository holds the firmware C code compatible with Tensilica's Xtensa LX6/7 microprocessors. 
<br>
<br>

### Smart PCB desgin and proof of Concept 
The hardware specifications for the 12bit pcb with dimensions of 23.5x43.5mm are the following:
<br>
<br>
-	QFN-56 Dual Xtensa LX7 Core Processors running up to 240MHz
  -	RISC V ultra-low power co-processor
  - 512Kb RAM (PSRAM max 1 Gb);
  - 16Mb SOIC 8 NOR SPI Flash Memory (max 1Gb);
  - 2.4GHz ISM wireless connectivity;
  - Up to 118 12bit ADC Multiplexed DAQ channels;
-	Authentication & Security:
  - SOIC-8 ATSHA204A SHA-256 high-security hardware authentication IC for secure and unique communication between devices;
-	Power management:
  - DFN-6 AUR9718 high efficiency step-down 3.3V 1.5A DC converter;
-	Onboard sensors:
  - DFN-8 SHT3.x; temperature sensor with a precision of 1.5C;
  - DFN-8 SHT3.x humidity sensor;
  - LGA-14 LSM6DS3 a 6-axis accelerometer and gyroscope;
-	External connectivity for up to 118 sensors:
  - 1x I2C 2 pin terminal connector (shared)
  - 1x 12bit digital terminal connector (shared)
  - 1x 12bit analog terminal connector (shared)


The PCB design files are underway....
<br>
<br>
in the meantime the reader can download the KiCad project files of similar smart DAQ compatible with the firmware code available on this repository:

https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/blob/main/Smart%20DAQ/README.md

![](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/LDAD_ATOM.jfif)

<br>
<br>

## Proof of Concept

To test and validate proposed smart DAQ PCB electronics and its firmware as a solution for LIVE experimental data measurements on any test specimen part of a experimental campaign, This PCB electronics is being used to measure a predefined set of variables/parameters to further study several asphalt mixtures with known carbon fiber weight content in the asphalt matrix. Below is a YouTube link to an unedited short video showing one of the experimental setups.

[![](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/smart_asphalt.png)](https://youtu.be/6td_RrH29jA)
