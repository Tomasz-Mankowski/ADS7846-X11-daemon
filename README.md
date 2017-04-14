Information:
------------
This is a linux user space daemon-driver for ADS7846 based touch screen. Daemon is intendet for Allwiner based devices like Orange Pi.
Tested for Orange Pi PC Plus running Armbian.

WORK IN PROGRESS!

Warning:
--------
When using Waveshare LDCs like http://www.waveshare.com/wiki/10.1inch_HDMI_LCD bare in mind rPi pinout incompatibility of OrangePi. Reroute CS pin to PC3 (SPI0_CS) of your OPI. 

How to:
-------
Install spidev-lib wrapper by lephiloux from:
    https://github.com/lephiloux/spidev-lib

Install WereCatf version of WiringOP:
    https://github.com/WereCatf/WiringOP
	
Run:
```
make
./build/ADS7846_X11_daemon
```

For help use:
```
./build/ADS7846_X11_daemon --help
```	