#ifndef _ADS7846_X11_daemon_H_
#define _ADS7846_X11_daemon_H_

#include <stdio.h>
#include <string.h>

#include <wiringPi.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include <signal.h>
#include <sys/time.h>

#include "ADS7846.h"
#include "calibration.h"
#include "point.h"

Display *display;
calibration touchCalib;
ADS7846 ads;

#endif