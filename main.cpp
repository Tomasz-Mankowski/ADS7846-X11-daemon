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
	printf("\r%.1f s    ", (float)waitTime - (float)currentWaitTime / 1000000);	
	fflush(stdout);
	
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
	
	int screen_num = DefaultScreen(display);
	unsigned int disp_w = DisplayWidth(display, screen_num);
	unsigned int disp_h = DisplayHeight(display, screen_num);
	
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
		
		if(!calibrationMode)
		{
			wiringPiISR(irqPin, INT_EDGE_BOTH, &penInterrupt);		
			
			printf("%s: Running...\n", argv[0]);
			
			while(1)
			{
				sleep(1);
			}
		}else //calibration mode
		{
			pinMode(irqPin, INPUT);
			
			for(int i=0; i<3; i++)
			{
				currentWaitTime = 0;
				
				printf("Remaining time: \n");
				
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
				
				printf("\nPoint %d caught\n", i);
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