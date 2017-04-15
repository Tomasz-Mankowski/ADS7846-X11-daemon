DEBUG = -O3
CC = g++
CFLAGS = $(DEBUG) -Wall

LDLIBS = -lspidev-lib++ -lwiringPi -lpthread -lX11 -lXtst

ADS7846_X11_daemon: ADS7846_X11_daemon.cpp ADS7846_X11_daemon.h calibration.cpp calibration.h
	$(CC) -o ./build/ADS7846_X11_daemon ADS7846_X11_daemon.cpp ADS7846_X11_daemon.h calibration.cpp calibration.h $(LDLIBS) $(CFLAGS)