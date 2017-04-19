Information:
------------
This is a linux user space daemon-driver for ADS7846 based touch screen. Daemon is intendet for Allwiner based devices like Orange Pi.
Tested for Orange Pi PC Plus running Armbian.

SPI communication is based on c++ wrapper created by Philippe Van Hecke - https://github.com/lephiloux/spidev-lib . 
Touch screen calibration procedure was designed and coded by Carlos E. Vidales - http://www.embedded.com/design/system-integration/4023968/How-To-Calibrate-Touch-Screens .

WORK IN PROGRESS!

Warning:
--------
When using Waveshare LDCs like http://www.waveshare.com/wiki/10.1inch_HDMI_LCD bare in mind rPi pinout incompatibility of OrangePi. Reroute CS pin to PC3 (SPI0_CS) of your OPI. 

How to:
-------

Install WereCatf version of WiringOP for handling pin/pen interrupt:
```
https://github.com/WereCatf/WiringOP
```

Run:
```
make
./build/ADS7846-X11
```

For help use:
```
./build/ADS7846-X11 --help
```	
