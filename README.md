[![Telegram](https://img.shields.io/badge/join-telegram-blue.svg?style=for-the-badge)](https://t.me/+W4rVVa0_VLEzYmI0)
 [![WhatsApp](https://img.shields.io/badge/join-whatsapp-green.svg?style=for-the-badge)](https://chat.whatsapp.com/FkNC7u83kuy2QRA5sqjBVg) 
 [![Donate](https://img.shields.io/badge/donate-$-brown.svg?style=for-the-badge)](http://paypal.me/mtpsilva)
 [![Say Thanks](https://img.shields.io/badge/Say%20Thanks-!-yellow.svg?style=for-the-badge)](https://saythanks.io/to/mtpsilva)
![](https://img.shields.io/github/last-commit/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo?style=for-the-badge)
<a href="https://trackgit.com">
<img src="https://us-central1-trackgit-analytics.cloudfunctions.net/token/ping/l95wbzce6vrs7sr48tbl" alt="trackgit-views" />
</a>
![](https://views.whatilearened.today/views/github/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo.svg)
[![Open Source Love svg1](https://badges.frapsoft.com/os/v1/open-source.svg?v=103)](#)
[![GitHub Forks](https://img.shields.io/github/forks/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo.svg?style=social&label=Fork&maxAge=2592000)](https://www.github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/fork)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat&label=Contributions&colorA=red&colorB=black	)](#)
[<img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" data-canonical-src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" height="30" />](https://www.buymeacoffee.com/migueltomas)


# Validation of Experimental Data Origins: A Swarm of DAQ devices able to Deliver Unique Experimental Data using Blockchain-like Fingerprint ID to a Data Repository

<br>

**The problem**

Current scientific methods use traditional dataloggers (DAQ) for doing experimental data measurements and collection. This means collected data many times is stored in a paper format, and most of the time in a conventional CSV excel data file. This is prone to errors and even worst, forgery of experimental data. To this date, there are no dataloggers able to automate an experimental data acquisition in a scientific experiment, making it transparent and trustworthy beyond common critique found nowadays. 

<br>

**The idea**

This Sci. research presents an innovative method for experimental data acquisition and management of collected data in real-time and is compatible with any open environment. The proposed smart DAQ device prototype has the minimum hardware characteristics to handle data measurements collected from sensors locally connected to it, store it on a local CSV or SQLite database file, and finally connect and synchronize data measurements collected with a data repository hosted remotely on a Dataverse.

These Smart DAQ devices are of type "Internet of Everything" (IoE) Smart Devices and are able to connect with each other using swarm intelligence. The main purpose is to increase data integrity and trustworthiness among DAQ devices connected and on all experimental data collected during an experiment or research project.

<p align="center">
  <img width="600" src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/swarm.png">
</p>

Experimental data collected is stored in a block format, meaning, a single block stores an individual piece of experimental data written to it, the hash of the previous block, and its own hash. 

<p align="center">
  <img width="600" src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/block.png">
</p>

This is the main principle of operation behind blockchain technologies, to make it really difficult to modify experimental data once it’s written to a block since hashes are interconnected among each other since the beginning of an experiment, experimental campaign, and even since the beginning of a research project. Every block written references the hash of its previous block. This way, for any modification to the data stored in a block, the hash it stores changes forcing the following blocks to also indicate a change (since they must have the hash of the previous block). To modify a block is needed a rewrite on all blocks.

<p align="center">
  <img width="600" src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/blockchain.png">
</p>

**In everyday science at a laboratory these Smart DAQ devices are able to connect among each other, in a swarm-like manner, and when doing so, increase experimental data trustworthiness and authenticity in an experiment part of a research project or experimental campaign.** Setting up a Swarm network of smart DAQ devices not only increases the quality of research results, by tagging each individual piece of experimental data collected from each individual sensor, with a unique data fingerprint ID (hash) at the exact same moment of data collection, broadcast it to other nearby smart DAQ devices and finally do data upload to a repository where a new, additional data fingerprint is added to existing ones (generated locally). This way is maintained and guaranteed data collection integrity locally, from the laboratory, until the moment is received and stored in a data repository in a cloud server.     

A paper is currently being written, in an open-environment format. Available as a preprint draft document at Elsevier's SSRN platform. https://ssrn.com/abstract=4210504 . See the [WIKI](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/wiki) for how this kind of smart electronics can be connected for fail-safe data and redundancy (and as an IoE DAQ device) on any experimental setup.
I also have available [here](https://github.com/aeonSolutions/AeonLabs--Writing-Styles-Classification-Using-Stylometric-Analysis), an intelligent system that takes a single document and classifies different writing styles within the document using stylometric analysis, that is worth have a run in python. 
<br>
<br>


**Get a Notification on every PCB update**

| [<img src="https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/blob/main/mailing-list_orig.png" alt="" width="80">](https://www.tindie.com/stores/aeonlabs/) | You can fill in your [email here (Google form)](https://docs.google.com/forms/d/e/1FAIpQLScErMgQYRdA-umvCjvTPPrCO7Lg1QYowTxb7vfa8cTfrcPEAA/viewform?usp=pp_url) and I'll send a reminder when a new PCB prototype is made available here or a new revision for an existing PCB. Stay tuned! |
|-------------|------|

<br>

**Most recent update**

The LDAD is officially up and running! Well, the very first release candidate of the firmware. Now, on this code revision, electrical resistance and measurement voltage are correctly measured by the built-in multimeter. And this means the smart DAQ is now ready to be installed on all specimens in an experimental campaign after configuration with the correct calibration curves (temperature compensated). From now on all changes will be towards improving usability and user experience in a laboratory.

The C firmware code now includes SRAM support for temporary storage of measured data in RAM up to 9175 samplings (for the 64Mbit IC) before flushing it into the onboard Flash storage. See [this](https://stackoverflow.com/questions/75004548/multidimensional-char-string-array-initialization-and-usage-for-esp32-mcus-with/75004549#75004549) stack overflow question for more info. The [1.69" Display](https://s.click.aliexpress.com/e/_DklsWrB) is now also available and a standard layout design and configuration are now shown when the smart DAQ is powered.

A new PCB design layout made to fit a well-known [waterproof acrylic enclosure](https://s.click.aliexpress.com/e/_Dmudkjt), is available with new DAQ capabilities. This new PCB design dimension is the one selected to move forward on this sci. research project. See the photos below.

<br>

**Status**

The pcb hardware has now a status of fully tested and fully working. The next milestone is now the firmware code being programmed. More updates in comming weeks. 

<br>

## ToDo List

**Smart DAQ Firmware:**
  - Calc MD5 hash of the dataset file yo compare with json result on upload 
  - OTA firmware update
  - Remove lock on a dataset (by an admin)
  - ~~Validate Json received on a new dataset upload~~
  - ~~Load dataset repository metadata~~
  - Output board startup diagnostics serial stream to a Bluetooth or WIFI data stream
  - accept GBRL like setup and config $ commands.
  - SQLite database to store measured experumental data and upload to a dataverse repository
  - Swarm connectivity to other nearby Smart DAQ devices for experimental data redundancy and sharing (IoE - Internet of Everything). 
  - Blockchain like data storage and exchange of experimental data collected
  - Experimental Data multiple Signing with multiple unique Fingerprint ID requests to other Smart DAQ devices:
    - from nearby devices on the same research project
    - from nearby devices on the same Laboratory
    - from nearby devices on the same campus
    - from devices located elsewhere (on the internet) and available to do experimental data signing (obtained from a list of devices registered in a public server) 
  - Automatic geolocation of experimental data origins using the wifi network it connects
  - Allow remote query of a device's setup, for instance sensor calibration,  at any time of the day (and night) from the data repository server
<br>

**Smart DAQ setup and configuration**
- Multi environment smart DAQ manager coded in QT6 (Android, Windows, Lunux,...)  for setup and configuration of individual smart DAQ devices to each specimen or sample (using RFID NFC technolgies).
- Ability to define a policy of experimental expected/ possible warnings on measured results (when in autonomous mode). 

<br>

**Experimental Data Media Manager**
- Multi environment experimental files media manager coded in QT6 (Android, Windows, Linux,...) able to upload an Edit metadata of photos and video files to a dataverse repository.

<br>
<br>

## Dataverse API C library

In paralell is being written a C library to expedite API integration on smart DAQ devices or elsewhere. Follow the link to its repository:

https://github.com/aeonSolutions/OpenScience-Dataverse-API-C-library

<p align="center">
  <a href="https://github.com/aeonSolutions/OpenScience-Dataverse-API-C-library">
   <img width="300" src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/dataverse_r_project.png">
 </a>
</p>

<br>

**Dataverse API in another coding language**

Goto dataverse.org for another coding language that best suits your coding style and needs. Currently there are client libraries for Python, Javascript, R, Java, and Julia that can be used to develop against Dataverse Software APIs

https://guides.dataverse.org/en/5.12/api/client-libraries.html

<br>
<br>

-----------------------------------------------------------------------------------
<br>

### Open Data Respsitories 

| [<img src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/dataverse_org.png" alt="" width="150">](https://dataverse.org) | The Dataverse Project is an open source web application to share, preserve, cite, explore, and analyze research data. It facilitates making data available to others, and allows you to replicate others' work more easily. Researchers, journals, data authors, publishers, data distributors, and affiliated institutions all receive academic credit and web visibility. |
|-------------|------|

<br>

| [<img src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/figshare_com.png" alt="" width="150">](https://figshare.com) | figshare is a repository where users can make all of their research outputs available in a citable, shareable and discoverable manner |
|-------------|------|

<br>

### OPEN Science Networks

| [<img src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/RDA_org.png" alt="" width="150">](https://rd-alliance.org) | With over 10000 members from 145 countries, Research Data Alliance provides a neutral space where its members can come together to develop and adopt infrastructure that promotes data-sharing and data-driven research |
|-------------|------|

<br>

| [<img src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/openAIRE_eu.png" alt="" width="150">](https://www.openaire.eu) | OpenAIRE's vision is to transform society through validated scientific knowledge and allow citizens, educators, funders, civil servants and industry to find ways to make science useful for themselves, their working environments and society. |
|-------------|------|

<br>

In the Wiki  the reader can find a [list of Open Science projects](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/wiki) worth mention. And if you know one of interest, share it here too, for everyone to see and use.   

<br>
<br>

-----------------------------------------------------------------------------------
<br>

### Smart DAQ on a Laboratory Specimen

<p align="center">
  <img width="600" src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/12bitSmartDAQ2023january_2.jpeg">
</p>

On the photo above the smart DAQ is installed on an acrylic case and screwed with plastic screws to an acrylic base with the same cross section area as the specimen to be tested. The acrylic base can be bought [here](https://s.click.aliexpress.com/e/_DEGsZaL). And the acrylic case [here](https://s.click.aliexpress.com/e/_Dmudkjt). 

<br>

<p align="center">
  <img width="600" src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/12bitSmartDAQ2023january_4.jpeg">
</p>

On the photo above is one of my many specimens I purposely fabricated to research self-sensing properties of a asphalt mixed with a known content of carbon fibers. This is a 10cm cylinder specimen and on the top is already setup my own design smart #DAQ (get it here on my GitHub ) with ability do upload LIVE experimental data to a #dataverse.

<br>

**Proof of Concept**

To test and validate proposed smart DAQ PCB electronics and its firmware as a solution for LIVE experimental data measurements on any test specimen part of a experimental campaign, This PCB electronics is being used to measure a predefined set of variables/parameters to further study several asphalt mixtures with known carbon fiber weight content in the asphalt matrix. Below is a YouTube link to an unedited short video showing one of the experimental setups.

<p align="center">
  <a href="https://youtu.be/ycCJRiwse2M">
   <img width="600" src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/Design/youtube.png">
 </a>
</p>

<br>
<br>

### Smart PCB Hardware Specifications

See a list with the hardware specifications for the 12bit pcb on the [WiKi](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/wiki).

<br>

 **In everyday science at a laboratory this smart DAQ is able to:**
- [connect to all kinds of 3.3V digital sensors](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/wiki/Types-of-external-sensors-this-devices-is-able-to-connect)
- [connect to all kinds of sensors compatible with the I2C protocol](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/wiki/Types-of-external-sensors-this-devices-is-able-to-connect) (max 118 sensors simultaneously)
- measure voltage in the range of  [0;3.3V]
- measure electrical resistance [0; 10^6] Ohm 
- do temperature and humidity compensation on all measurements 
- [has a voltage reference sensor for improved accuracy on ADC measurements](https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/wiki/Voltage-Reference-Sensor-Calibration)  
- has a motion sensor to know if anyone moved a specimen during an experiment
- can be powered using 4.2V LiPo batteries
<br>
<br>

## PCB Design Files and Circuit Schematic

<br>

### [12bit Smart DAQ Device with unique data fingerprint and a 1.69" TFT LCD made for an Acrylic 52x48mm Enclosure](https://github.com/aeonSolutions/Open-Science-12bit-Smart-DAQ-unique-data-fingerprint-made-for-an-Acrylic-52x48mm-)

This is the repository for the 12bit Smart DAQ Device made to fit an acrylic enclosure as seen on the photos above. 
 
<p align="center">
  <img width="350" src="https://github.com/aeonSolutions/Open-Science-12bit-Smart-DAQ-unique-data-fingerprint-made-for-an-Acrylic-52x48mm-/blob/main/designs/pcb_front.png">
</p>

<br>

<br>

### [Open Science: 12bit Smart DAQ Device with unique data fingerprint and a 1.69" TFT LCD](https://github.com/aeonSolutions/-openScienceResearch-12bit-LCD-1.69-TFT-Smart-DAQ-Device-with-unique-data-fingerprint)

This is the repository for the 12bit Smart DAQ Device with unique data fingerprint able to do experimental data upload to any data repository
 
<p align="center">
  <img width="350" src="https://github.com/aeonSolutions/-openScienceResearch-12bit-LCD-1.69-TFT-Smart-DAQ-Device-with-unique-data-fingerprint/blob/main/Designs/smart_daq_front.png">
</p>

<br>

### [Open Science: 12bit Smart DAQ Device with unique data fingerprint](https://github.com/aeonSolutions/openScienceResearch-12bit-Smart-DAQ-Device-with-unique-data-fingerprint-)

This is the repository for the 12bit Smart DAQ Device with unique data fingerprint able to do experimental data upload to any data repository
 
<p align="center">
  <img width="350" src="https://github.com/aeonSolutions/openScienceResearch-Smart-DAQ-Device-able-to-Upload-Live-Experimental-Sensor-Data-to-a-Data-Repo/blob/main/LDAD%20fron.png">
</p>

<br>
<br>

## Author

You can get in touch with me on my LinkedIn Profile:

#### Miguel Tomas

[![LinkedIn Link](https://img.shields.io/badge/Connect-Miguel--Tomas-blue.svg?logo=linkedin&longCache=true&style=social&label=Connect)](https://www.linkedin.com/in/migueltomas/)

You can also follow my GitHub Profile to stay updated about my latest projects: [![GitHub Follow](https://img.shields.io/badge/Connect-Miguel--Tomas-blue.svg?logo=Github&longCache=true&style=social&label=Follow)](https://github.com/aeonSolutions)

______________________________________________________________________________________________________________________________
### License

Before proceeding to download any of AeonLabs software solutions for open-source development and/or PCB hardware electronics development make sure you are choosing the right license for your project. See [AeonLabs Solutions for Open Hardware & Source Development](https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/wiki/AeonLabs-Solutions-for-Open-Hardware-&-Source-Development) for more information. 

### Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/aeonSolutions/OpenScience-Dataverse-API-C-library. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [code of conduct](https://github.com/aeonSolutions/OpenScience-Dataverse-API-C-library/blob/main/CODE_OF_CONDUCT.md). Don't forget to read [AeonLabs's Wiki](https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/wiki) before using any code or electronics available here on Github. Thank you.

### Coding standards

Please make sure tests pass before committing, and to add new tests for new additions.

______________________________________________________________________________________________________________________________

<br />

### Be supportive of my dedication and work towards technology education and buy me a cup of coffee
Buy me a cup of coffee, a slice of pizza or a book to help me study, eat and think new PCB design files.

<p align+"left">
<a href="https://www.buymeacoffee.com/migueltomas">
   <img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" data-canonical-src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" height="35" />
 </a>
<br>
<img src="https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/blob/main/Designs/bmc_qr.png" height="130">
</p>

<br />

### Make a donation on Paypal
Make a donation on paypal and get a TAX refund*.

[![](https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/blob/main/paypal_small.png)](http://paypal.me/mtpsilva)


### Support all these open hardware projects and become a patreon  
Liked any of my PCB KiCad Designs? Help and Support my open work to all by becomming a LDAD Patreon.
In return I will give a free PCB design in KiCad to all patreon supporters. To learn more go to patreon.com. Link below.

[![](https://github.com/aeonSolutions/PCB-Prototyping-Catalogue/blob/main/patreon_small.png)](https://www.patreon.com/ldad)


