#include "ADS7846_X11_daemon.h"

const char *usage = "ADS7846_X11_daemon usage:\n\n"
					"ADS7846_X11_daemon [<property> <argument>]\n\n"
					"\t-h --help - this help\n"
					"\t--cal     - calibration mode\n"
					"\t--spi     - set spi device path (default: '/dev/spidev0.0')\n"
					"\t--pin     - set interrupt pin in wiringPi gpio space (default: 6)\n"
					"\t--disp    - specify display in X11 (default ':0.0'), use system '$ w' command to specify\n"
					
					;

char* spiPath = (char*)"/dev/spidev0.0";
char* screenName = (char*)":0.0";
int irqPin = 6;

void penInterrupt(void) 
{
	usleep(10000); //bruteforce debounce
	
	point resistance = ads.getXYdata();
	
	if( !digitalRead(irqPin) && resistance != point(0x000, 0xFFF) )
	{
		//printf("Pen down\n");
		
		point position = touchCalib.getDisplayPoint(resistance);
		
		//printf("Resistance: X: %d ; Y: %d\n", resistance.x(), resistance.y());
		//printf("Position: X: %d ; Y: %d\n", position.x(), position.y());
		XTestFakeMotionEvent (display, 0, position.x(), position.y(), CurrentTime );
		XFlush(display);
		
		XTestFakeButtonEvent(display, 1, True, CurrentTime);
		XFlush(display);
		
		while( !digitalRead(irqPin) && resistance != point(0x000, 0xFFF) )
		{
			usleep(100000);
			
			if(!digitalRead(irqPin))
			{
				//printf("Pen holding\n");				
				resistance = ads.getXYdata();
				position = touchCalib.getDisplayPoint(resistance);
				XTestFakeMotionEvent (display, 0, position.x(), position.y(), CurrentTime);
				XFlush(display);
			}			
		}
		
		//printf("Pen up\n");
		XTestFakeButtonEvent(display, 1, False, CurrentTime); 
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
	
	point screenSample[3] =	{
                                            point(3300, 620),
                                            point(570, 2014),
                                            point(1948, 3429)
                                    } ;
									
	point displaySample[3] =	{
                                            point(154,  90),
                                            point(512, 510),
                                            point(870, 300)
                                    } ;
									
	touchCalib.setCalibrationMatrix(displaySample, screenSample) ;
	
	
	if (ads.open(spiPath))
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
			//fflush(stdout);
		}
	
		XCloseDisplay(display); 
	
	}else
	{
		printf("SPI not connected \n");
		return 0;
	}
	
	return 1;
}