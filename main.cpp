#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <spidev_lib++.h>
#include <wiringPi.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

const char *usage = "ADS7846_X11_daemon usage:\n\n"
					"ADS7846_X11_daemon [<property> <argument>]\n\n"
					"\t-h --help - this help\n"
					"\t--cal     - calibration mode\n"
					"\t--spi     - set spi device path (default: '/dev/spidev0.0')\n"
					"\t--pin     - set interrupt pin in wiringPi gpio space (default: 6)\n"
					"\t--disp    - specify display in X11 (default ':0.0'), use system '$ w' command to specify\n"
					
					;

SPI* spiHandler = NULL;
char* spiPath = "/dev/spidev0.0";
char* screenName = ":0.0";
int irqPin = 6;

Display *display;

int openSPIconnection(char* spiName)
{
	spi_config_t spiConfig;
	
	spiConfig.mode=0;
	spiConfig.speed=1000000;
	spiConfig.delay=0;
	spiConfig.bits_per_word=8;
	
	spiHandler = new SPI(spiName, &spiConfig);
	
	return spiHandler->begin();
}

uint8_t spiTxBuffer[5] = {0xD0, 0x00, 0x90, 0x00, 0x00};
uint8_t spiRxBuffer[5];

void getXYdata(int &Xres, int &Yres)
{
	memset(spiRxBuffer,0,5);
	
	spiHandler->xfer(spiTxBuffer,5,spiRxBuffer,5);
		
	Xres = ((spiRxBuffer[1] << 8) | spiRxBuffer[2] ) >> 3;
	Yres = ((spiRxBuffer[3] << 8) | spiRxBuffer[4] ) >> 3;
}


void calculateScreenPosition(int &Xres, int &Yres, int &Xpos, int &Ypos)
{
	Xpos = 0.4423 * (float)Xres + 0.5838 * (float)Yres - 1515.9;
	Ypos = -0.3199 * (float)Xres + 0.0477 * (float)Yres + 773.6623;	
}

void getAvaregeResistance(int &XavRes, int &YavRes)
{
	#define READS_NUM 10
	
	int Xres, Yres;
	
	XavRes = 0;
	YavRes = 0;
	
	for(int i=0; i<READS_NUM; i++)
	{
		getXYdata(Xres, Yres);
		XavRes += Xres;
		YavRes += Yres;
		usleep(1000);
	}
	
	XavRes = XavRes / READS_NUM;
	YavRes = YavRes / READS_NUM;
}

void penInterrupt(void) 
{
	usleep(10000);
	
	int Xres, Yres;
	int Xpos, Ypos;
	
	getXYdata(Xres, Yres);
	
	if(digitalRead(irqPin) && (Xres == 0x000) && (Yres == 0xFFF))
	{
		printf("Pen up\n");
		XTestFakeButtonEvent(display, 1, False, CurrentTime); 
		XFlush(display);
	}else if((Xres > 0x000) && (Yres < 0xFFF))
	{
		printf("Pen down\n"); 
		
		getAvaregeResistance(Xres, Yres);
		
		calculateScreenPosition(Xres, Yres, Xpos, Ypos);
		
		printf("X: %d ; Y: %d\n", Xres, Yres);
		XTestFakeMotionEvent (display, 0, Xpos, Ypos, CurrentTime );
		XFlush(display);
		
		XTestFakeButtonEvent(display, 1, True, CurrentTime);
		XFlush(display);
	}
	
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
			}
		}
	}
	
	if (openSPIconnection(spiPath))
	{
		display = XOpenDisplay(screenName);		
		
		if(!display)
		{
			printf("Error connecting to X11 server");
			return 0;
		}	
		
		wiringPiSetup();
		wiringPiISR(irqPin, INT_EDGE_BOTH, &penInterrupt);		
		
		printf("Running...\n");
		
		while(1)
		{
			fflush(stdout);
			//int Xres, Yres;
			//getAvaregeResistance(Xres, Yres);
			//printf("X: %d ; Y: %d\n", Xres, Yres);
			//sleep(1);
		}
	
		delete spiHandler;
		XCloseDisplay(display); 
	
	}else
	{
		printf("SPI not connected \n");
		return 0;
	}
	
	return 1;
}