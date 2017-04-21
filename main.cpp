#include <stdio.h>
#include <string.h>

#include <iostream>
#include <fstream>

#include <wiringPi.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include <signal.h>

#include "ADS7846.h"
#include "calibration.h"
#include "point.h"

Display *display;
Window win;
GC gc;
unsigned int disp_w;
unsigned int disp_h;
int screen_num;
calibration touchCalib;
ADS7846 *ads = NULL;
std::fstream *calibFile = NULL;
point screenCalibPoints[3];
point displayCalibPoints[3];

const char *usage = "ADS7846_X11_daemon usage:\n\n"
					"ADS7846_X11_daemon [<property> <argument>]\n\n"
					"\t-h --help - this help\n"
					"\t--spi     - set spi device path (default: '/dev/spidev0.0')\n"
					"\t--pin     - set interrupt pin in wiringPi gpio space (default: 6)\n"
					"\t--disp    - specify display in X11 (default ':0.0'), use system '$ w' command to specify\n"
					"\t--cal     - enter calibration mode\n"
					"\t--cfile   - change calibration source file (default: 'calibpoints.cal')\n"				
					"\t--ctime   - waiting time for next calibration point in seconds (default: '5')\n"
					;

char* spiPath = (char*)"/dev/spidev0.0";
char* screenName = (char*)":0.0";
int irqPin = 6;
char* calibFileName = (char*)"calibpoints.cal";
int waitTime = 5;

bool calibrationMode = false;

void penInterrupt(void) 
{
	usleep(10000); //bruteforce debounce
	
	point resistance = ads->getXYdata();
	
	if(!digitalRead(irqPin) && resistance != point(0x000, 0xFFF))
	{
		//Pen down		
		point position = touchCalib.getDisplayPoint(resistance);
		
		XTestFakeMotionEvent (display, 0, position.x(), position.y(), CurrentTime );
		XFlush(display);
		
		XTestFakeButtonEvent(display, 1, True, CurrentTime);
		XFlush(display);
		
		while(!digitalRead(irqPin) && resistance != point(0x000, 0xFFF))
		{
			usleep(100000);
			
			if(!digitalRead(irqPin))
			{
				//Pen holding			
				resistance = ads->getXYdata();
				position = touchCalib.getDisplayPoint(resistance);
				XTestFakeMotionEvent (display, 0, position.x(), position.y(), CurrentTime);
				XFlush(display);
			}			
		}
		
		//Pen up
		XTestFakeButtonEvent(display, 1, False, CurrentTime); 
		XFlush(display);
	}
}

void closeApp(int s=0)
{	
	if(display != NULL)
	{
		XCloseDisplay(display); 
	}
	
	if(calibFile != NULL)
	{
		calibFile->close();		
	}
		
	if(ads != NULL)
	{
		delete ads;
	}
	
	printf("All closed...\n");
	
	exit(0);
}

int currentWaitTime;

void handleCalibrationWait()
{
	usleep(100000);
	currentWaitTime += 100000;
	
	XSetForeground(display, gc, WhitePixel(display, screen_num));
	XFillRectangle(display, win, gc, disp_w/2+2*12, disp_h/2+2-24, 6*12, 24);
	XFlush(display);
	
	char* str=NULL;
	int len = asprintf(&str, "%.1f s", (float)waitTime - (float)currentWaitTime / 1000000);
	if(len != -1)
	{
		XSetForeground(display, gc, BlackPixel(display, screen_num));
		XDrawString(display, win, gc, disp_w/2+2*12, disp_h/2+2, str, len);
		XFlush(display);
	}
	
	if(waitTime*1000000 <= currentWaitTime)
	{
		printf("\nCalibration time exceeded\n");
		closeApp();
	}
}

int main(int argc, char *argv[])
{
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = closeApp;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	
	if(argc > 1)
	{
		for(int i = 1; i < argc; i++)
		{
			if((strcmp("--help", argv[i]) == 0) || (strcmp("-h", argv[i]) == 0)) //help
			{
				printf("%s", usage);
				return 0;
			}
			
			if(strcmp("--cal", argv[i]) == 0) //spi path
			{
				calibrationMode = true;
			}
			
			if (argc > 1)
			{
				if(strcmp("--spi", argv[i]) == 0) //spi path
				{
						spiPath = argv[i+1];
						i++;
				}
				
				if(strcmp("--pin", argv[i]) == 0) //irq pin in wiringPi gpio space
				{
						irqPin = atoi(argv[i+1]);
						i++;
				}
				
				if(strcmp("--disp", argv[i]) == 0) //X11 display address
				{
						screenName = argv[i+1];
						i++;
				}
				
				if(strcmp("--cfile", argv[i]) == 0) //calibration file
				{
						calibFileName = argv[i+1];
						i++;
				}
				
				if(strcmp("--ctime", argv[i]) == 0) //calibration time
				{
						waitTime = atoi(argv[i+1]);
						i++;
				}
			}
		}
	}	
	
	display = XOpenDisplay(screenName);	
	if(display == NULL)
	{
		fprintf(stderr, "%s: Error connecting to X11 server: %s\n", argv[0], screenName);
		closeApp();
	}
	
	screen_num = DefaultScreen(display);
	disp_w = DisplayWidth(display, screen_num);
	disp_h = DisplayHeight(display, screen_num);
	
	displayCalibPoints[0] = point(0.15*disp_w,  0.15*disp_h);
	displayCalibPoints[1] = point(0.50*disp_w,  0.85*disp_h);
	displayCalibPoints[2] = point(0.85*disp_w,  0.50*disp_h);
	
	if(!calibrationMode)
	{
		calibFile = new std::fstream(calibFileName);
		if(!calibFile->is_open())
		{
			fprintf(stderr, "%s: Error opening calibration file: %s\n", argv[0], calibFileName);
			closeApp();
		}
		
		std::string calibLine;
		int lineCount = 0;
		while(std::getline(*calibFile, calibLine) && lineCount < 3)
		{
			std::size_t posFound = calibLine.find_first_of(";");
			if(posFound != std::string::npos)
			{
				std::string xRes = calibLine.substr(0, posFound);
				std::string yRes = calibLine.substr(posFound+1, calibLine.length()-1);
					
				screenCalibPoints[lineCount] = point(std::stoi(xRes), std::stoi(yRes));
				
				lineCount ++;
			}
			
			if(lineCount >= 3)
			{
				break;
			}		
		}
		
		if(lineCount <3)
		{
			fprintf(stderr, "%s: Error not enough calib points in fie %s, run: %s --cal\n", argv[0], calibFileName, argv[0]);
			closeApp();
		}
		
		touchCalib.setCalibrationMatrix(displayCalibPoints, screenCalibPoints);
	}
	ads = new ADS7846();
	
	if(ads->openCon(spiPath))
	{
		wiringPiSetup();
		
		if(!calibrationMode) //operation mode
		{
			wiringPiISR(irqPin, INT_EDGE_BOTH, &penInterrupt);		
			
			printf("%s: Running...\n", argv[0]);
			
			while(1)
			{
				sleep(1);
			}
		}else //calibration mode
		{
			printf("%s: Calibrating...\n", argv[0]);
		
			pinMode(irqPin, INPUT);
			
			win = XCreateSimpleWindow(display, RootWindow(display, screen_num), 0, 0, disp_w, disp_h, 0, BlackPixel(display, screen_num), WhitePixel(display, screen_num));
			Atom wm_state   = XInternAtom (display, "_NET_WM_STATE", 1);
			Atom wm_fullscreen = XInternAtom (display, "_NET_WM_STATE_FULLSCREEN", 1);
			XChangeProperty(display, win, wm_state, 4, 32, PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
			XMapWindow(display, win);
			XFlush(display);
			
			XGCValues values;
			gc = XCreateGC(display, win, 0, &values);
			if (gc < 0) 
			{
				fprintf(stderr, "%s: Error creating GC for X11\n", argv[0]);
				closeApp();
			}

			XSetForeground(display, gc, BlackPixel(display, screen_num));
			XSetBackground(display, gc, WhitePixel(display, screen_num));
 
			XSetLineAttributes(display, gc, 2, LineSolid, CapButt, JoinBevel);
			XSetFillStyle(display, gc, FillSolid);
			
			Font font;
			font = XLoadFont(display, "12x24");
			if(font == BadAlloc || font == BadName)
			{
				fprintf(stderr, "%s: Could not load font 12x24\n", argv[0]);
				closeApp();
			}
			XSetFont(display, gc, font);

			XSync(display, False);

			Pixmap bitmap;
			unsigned int bitmap_w, bitmap_h;
			int hotspot_x, hotspot_y;

			int rc = XReadBitmapFile(display, win, "cross.xbm", &bitmap_w, &bitmap_h, &bitmap, &hotspot_x, &hotspot_y);
			if(rc != BitmapSuccess)
			{
				fprintf(stderr, "%s: Error reading cross.xbm bitmap\n", argv[0]);
				closeApp();
			}
			
			XDrawString(display, win, gc, disp_w/2-28*6, disp_h/2-24, "Touch points to calibrate...", 28);
			XDrawString(display, win, gc, disp_w/2-28*6, disp_h/2+2, "Remaining time:", 15);
			XFlush(display);
			
			for(int i=0; i<3; i++)
			{
				XSetForeground(display, gc, BlackPixel(display, screen_num));
				XCopyPlane(display, bitmap, win, gc, 0, 0, bitmap_w, bitmap_h, displayCalibPoints[i].x()-bitmap_w/2, displayCalibPoints[i].y()-bitmap_h/2, 1);
				XFlush(display);
				
				if(i > 0)
				{
					XSetForeground(display, gc, WhitePixel(display, screen_num));
					XFillRectangle(display, win, gc, displayCalibPoints[i-1].x()-bitmap_w/2, displayCalibPoints[i-1].y()-bitmap_h/2, bitmap_w, bitmap_h);
					XFlush(display);
				}
				
				currentWaitTime = 0;
				
				while(digitalRead(irqPin))
				{
					handleCalibrationWait();
				}				
				usleep(10000);
				
				screenCalibPoints[i] = ads->getXYdata();				
				if(screenCalibPoints[i] == point(0x000, 0xFFF))
				{
					i--;
					continue;
				}
				
				while(!digitalRead(irqPin))
				{
					handleCalibrationWait();
				}				
				usleep(10000);	
			}
			
			calibFile = new std::fstream(calibFileName, std::fstream::out);
			if(!calibFile->is_open())
			{
				fprintf(stderr, "%s: Error opening calibration file: %s for write\n", argv[0], calibFileName);
				closeApp();
			}
			
			for(int i=0; i<3; i++)
			{
				*calibFile << screenCalibPoints[i].x() << ";" << screenCalibPoints[i].y() << std::endl;
			}		
		}	
		
	}else
	{
		fprintf(stderr, "%s: Error connecting to SPI dev: %s\n", argv[0], spiPath);
		closeApp();
	}
	
	closeApp();	
		
	return 1;
}