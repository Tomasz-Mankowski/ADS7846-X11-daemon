DEBUG = -O3
CC = g++
CFLAGS = $(DEBUG) -Wall

LDLIBS = -lspidev-lib++ -lwiringPi -lpthread -lX11 -lXtst

ADS7846-X11: main.cpp calibration.cpp calibration.h point.cpp point.h ADS7846.cpp ADS7846.h
	$(CC) -o ./build/ADS7846-X11 main.cpp calibration.cpp calibration.h point.cpp point.h ADS7846.cpp ADS7846.h $(LDLIBS) $(CFLAGS)