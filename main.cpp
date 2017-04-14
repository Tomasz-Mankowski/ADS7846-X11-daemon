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
					"\t--cal     - calibration mode"
					"\t--spi     - set spi device path (default: '/dev/spidev0.0')\n"
					"\t--pin     - set interrupt pin in wiringPi gpio space (default: 6)"
					"\t--disp    - specify display in X11 (default ':0.0'), use system '$ w' command to specify "
					
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

int getXYdata(int &Xres, int &Yres)
{
	memset(spiRxBuffer,0,5);
	
	spiHandler->xfer(spiTxBuffer,5,spiRxBuffer,5);
		
	Xres = ((spiRxBuffer[1] << 8) | spiRxBuffer[2] ) >> 3;
	Yres = ((spiRxBuffer[3] << 8) | spiRxBuffer[4] ) >> 3;
	
	return 1;
}

int Xres, Yres;

void penInterrupt(void) 
{
	usleep(1000);
	
	getXYdata(Xres, Yres);
	
	if(digitalRead(irqPin) && (Xres == 0x000) && (Yres == 0xFFF))
	{
		printf("Pen up\n");
		XTestFakeButtonEvent(display, 1, False, CurrentTime); 
		XFlush(display);
	}else if((Xres > 0x000) && (Yres < 0xFFF))
	{
		printf("Pen down\n"); 
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
			
			if(strcmp("--spi", argv[i]) == 0) //spi path
			{
				if(argc > i)
				{
					spiPath = argv[i+1];
					i++;
				}
			}
			
			if(strcmp("--pin", argv[i]) == 0) //irq pin in wiringPi gpio space
			{
				if(argc > i)
				{
					irqPin = atoi(argv[i+1]);
					i++;
				}
			}
			
			if(strcmp("--disp", argv[i]) == 0) //X11 display address
			{
				if(argc > i)
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
			fflush (stdout) ;
			/*int Xts, Yts;
			
			getXYdata(Xts, Yts);
			
			int Xscr = -0.0095 * (float)Xts + 0.5346 * (float)Yts - 550.0317;
			int Yscr = 0.3056 * (float)Xts + 0.0047 * (float)Yts - 322.8494;		
			
			printf("X: %d ; Y: %d -> Xscr: %d ; Yscr: %d \n", Xts, Yts, Xscr, Yscr);
			*/
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