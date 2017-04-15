#ifndef _ADS7846_X11_daemon_H_
#define _ADS7846_X11_daemon_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <spidev_lib++.h>
#include <wiringPi.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include "calibration.h"

calibMatrix matrix;

#endif