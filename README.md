Information:
------------

This is a linux user space daemon-driver for ADS7846 based touch screen. Daemon is intended for Allwiner based devices like Orange Pi. 
Program includes calibration mode ( --cal ) with visual guidence based on X11. Tested for Orange Pi PC Plus running Armbian.

SPI communication is based on c++ wrapper created by Philippe Van Hecke - https://github.com/lephiloux/spidev-lib . 
Touch screen calibration procedure was designed and coded by Carlos E. Vidales - http://www.embedded.com/design/system-integration/4023968/How-To-Calibrate-Touch-Screens .

Tips:
-----
When using Waveshare LCDs like http://www.waveshare.com/wiki/10.1inch_HDMI_LCD bear in mind RPi pinout incompatibility of OrangePi. Reroute CS pin to PC3 (SPI0_CS) of your OPi. 

If you wan to use nonroot user add /etc/udev.d rule for spi and gpio.

How to:
-------

Install WereCatf version of WiringOP for handling pin/pen interrupt:
```
https://github.com/WereCatf/WiringOP
```

Compile using:
```
$ make
```

Calibrate touch screen:
```
$ ./build/ADS7846-X11 --cal
```

Run:
```
$ ./build/ADS7846-X11
```

Or run it as service/daemon using:
```
$ nohup ./build/ADS7846-X11 > /dev/null 2>&1 &
```

For help use:
```
$ ./build/ADS7846-X11 --help
```	
