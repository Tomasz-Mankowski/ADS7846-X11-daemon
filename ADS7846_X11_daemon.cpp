#include "ADS7846_X11_daemon.h"

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

void getXYdata(point &resistance)
{
	memset(spiRxBuffer,0,5);
	
	spiHandler->xfer(spiTxBuffer,5,spiRxBuffer,5);
		
	resistance.x = ((spiRxBuffer[1] << 8) | spiRxBuffer[2] ) >> 3;
	resistance.y = ((spiRxBuffer[3] << 8) | spiRxBuffer[4] ) >> 3;
}


void calculateScreenPosition(point &resistance, point &position)
{
	position.x = 0.4423 * (float)resistance.x + 0.5838 * (float)resistance.y - 1515.9;
	position.y = -0.3199 * (float)resistance.x + 0.0477 * (float)resistance.y + 773.6623;	
}

void getAvaregeResistance(point &avResistance)
{
	#define READS_NUM 10
	
	point resistance;
	
	avResistance.x = 0;
	avResistance.y = 0;
	
	for(int i=0; i<READS_NUM; i++)
	{
		getXYdata(resistance);
		avResistance.x += resistance.x;
		avResistance.y += resistance.y;
		usleep(1000);
	}
	
	avResistance.x = avResistance.x / READS_NUM;
	avResistance.y = avResistance.y / READS_NUM;
}

void penInterrupt(void) 
{
	usleep(10000);
	
	point resistance;
	point position;
	
	getXYdata(resistance);
	
	if(digitalRead(irqPin) && (resistance.x == 0x000) && (resistance.y == 0xFFF))
	{
		printf("Pen up\n");
		XTestFakeButtonEvent(display, 1, False, CurrentTime); 
		XFlush(display);
	}else if((resistance.x > 0x000) && (resistance.y < 0xFFF))
	{
		printf("Pen down\n"); 
		
		getAvaregeResistance(resistance);
		
		getDisplayPoint( &position, &resistance, &matrix ) ;
		
		printf("X: %d ; Y: %d\n", resistance.x, resistance.y);
		XTestFakeMotionEvent (display, 0, position.x, position.y, CurrentTime );
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
	
	point screenSample[6] =	{
                                            { 2369, 856 },
                                            { 1213, 2559 },
                                            { 1872, 2628 },
                                            { 2157, 1349 },
                                            { 1851, 1998 },
                                            { 1499, 2613 }
                                    } ;
									
	point displaySample[6] =	{
                                            { 154,  90 },
                                            { 512, 510 },
                                            { 870, 300 },
                                            { 307, 180 },
                                            { 512, 300 },
                                            { 717, 420 }
                                    } ;
									
	setCalibrationMatrix( &displaySample[0], &screenSample[0], &matrix ) ;
	
	point position;
	
	for(int n = 0 ; n < 6 ; ++n )
    {
        getDisplayPoint( &position, &screenSample[n], &matrix ) ;
        printf("  % 6d,%-6d      % 6d,%-6d       % 6d,%-6d\n",
                screenSample[n].x,  screenSample[n].y,
                position.x,          position.y,
                displaySample[n].x, displaySample[n].y ) ;
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