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
calibration touchCalib;
ADS7846 *ads;
std::fstream *calibFile;

const char *usage = "ADS7846_X11_daemon usage:\n\n"
					"ADS7846_X11_daemon [<property> <argument>]\n\n"
					"\t-h --help - this help\n"
					"\t--cal     - calibration mode\n"
					"\t--cfile   - change calibration source file (default: 'calibpoints.cal')\n"
					"\t--spi     - set spi device path (default: '/dev/spidev0.0')\n"
					"\t--pin     - set interrupt pin in wiringPi gpio space (default: 6)\n"
					"\t--disp    - specify display in X11 (default ':0.0'), use system '$ w' command to specify\n"
					
					;

char* spiPath = (char*)"/dev/spidev0.0";
char* screenName = (char*)":0.0";
int irqPin = 6;
char* calibFileName = (char*)"calibpoints.cal";

bool running = true;
bool calibrationMode = false;

void penInterrupt(void) 
{
	usleep(10000); //bruteforce debounce
	
	point resistance = ads->getXYdata();
	
	if( !digitalRead(irqPin) && resistance != point(0x000, 0xFFF) )
	{
		//printf("%s: Pen down\n", argv[0]);
		
		point position = touchCalib.getDisplayPoint(resistance);
		
		//printf("%s: Resistance: X: %d ; Y: %d\n", argv[0], resistance.x(), resistance.y());
		//printf("%s: Position: X: %d ; Y: %d\n", argv[0], position.x(), position.y());
		XTestFakeMotionEvent (display, 0, position.x(), position.y(), CurrentTime );
		XFlush(display);
		
		XTestFakeButtonEvent(display, 1, True, CurrentTime);
		XFlush(display);
		
		while( !digitalRead(irqPin) && resistance != point(0x000, 0xFFF) )
		{
			usleep(100000);
			
			if(!digitalRead(irqPin))
			{
				//printf("%s: Pen holding\n", argv[0]);				
				resistance = ads->getXYdata();
				position = touchCalib.getDisplayPoint(resistance);
				XTestFakeMotionEvent (display, 0, position.x(), position.y(), CurrentTime);
				XFlush(display);
			}			
		}
		
		//printf("%s: Pen up\n", argv[0]);
		XTestFakeButtonEvent(display, 1, False, CurrentTime); 
		XFlush(display);
	}
}

void intHandler(int s)
{
	running = false;
}

int main(int argc, char *argv[])
{
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
			}
		}
	}
	
	ads = new ADS7846();
	
	display = XOpenDisplay(screenName);	
	if(display == NULL)
	{
		fprintf(stderr, "%s: Error connecting to X11 server: %s\n", argv[0], screenName);
		return 0;
	}
	
	calibFile = new std::fstream(calibFileName);
	if(!calibFile->is_open())
	{
		fprintf(stderr, "%s: Error opening calibration file: %s\n", argv[0], calibFileName);
		return 0;
	}	
	
	int screen_num = DefaultScreen(display);
	unsigned int disp_w = DisplayWidth(display, screen_num);
	unsigned int disp_h = DisplayHeight(display, screen_num);
	
	point displayCalibPoints[3] = { point(0.15*disp_w,  0.15*disp_h), point(0.50*disp_w,  0.85*disp_h), point(0.85*disp_w,  0.50*disp_h) } ;
	
	point screenCalibPoints[3];
	
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
		return 0;
	}
	
	touchCalib.setCalibrationMatrix(displayCalibPoints, screenCalibPoints);
	
	if (ads->openCon(spiPath))
	{
		wiringPiSetup();
		wiringPiISR(irqPin, INT_EDGE_BOTH, &penInterrupt);		
		
		printf("%s: Running...\n", argv[0]);
		
		struct sigaction sigIntHandler;
		sigIntHandler.sa_handler = intHandler;
		sigemptyset(&sigIntHandler.sa_mask);
		sigIntHandler.sa_flags = 0;
		sigaction(SIGINT, &sigIntHandler, NULL);
		
		while(running)
		{
			sleep(1);
		}	
		
	}else
	{
		fprintf(stderr, "%s: Error connecting to SPI dev: %s\n", argv[0], spiPath);
		return 0;
	}
	
	calibFile->close();
	
	XCloseDisplay(display); 
	
	delete ads;
	
	printf("%s: All closed...\n", argv[0]);
	
	return 1;
}