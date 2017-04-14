#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spidev_lib++.h>
#include <unistd.h>
#include <wiringPi.h>

const char *usage = "ADS7846_X11_daemon usage:\n"
					"\t-h - this help\n"
					"\t-s - set spi device path (default: '/dev/spidev0.0')\n"
					"\t-c - calibration mode"
					;

SPI* spiHandler = NULL;
char* spiPath = "/dev/spidev0.0";

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

uint8_t spiTxBuffer[3];
uint8_t spiRxBuffer[3];

int getXYdata(int &Xts, int &Yts)
{
	memset(spiTxBuffer,0,3);
	memset(spiRxBuffer,0,3);
	
	const uint8_t poll_commands[4] = {0xD0, 0x90};
	uint16_t values[2];	
	
	for(int i=0; i<2; i++)
	{
		spiTxBuffer[0] = poll_commands[i];
		spiHandler->xfer(spiTxBuffer,3,spiRxBuffer,3);
		
		values[i] = ((spiRxBuffer[1] << 8) | spiRxBuffer[2] ) >> 3;
	}
	
	Xts = values[0];
	Yts = values[1];
	
	return 1;
}


int main(int argc, char *argv[])
{
	if(argc > 1)
	{
		for(int i = 1; i < argc; i++)
		{
			if(strcmp("-h", argv[i]) == 0)
			{
				printf("%d: %s", argc, usage);
				return 0;
			}
			
			if(strcmp("-s", argv[i]) == 0)
			{
				if(argc > i)
				{
					spiPath = argv[i+1];
					i++;
				}
			}
		}
	}

	if (openSPIconnection(spiPath))
	{
		printf("Beginned\n");

		while(1)
		{
			int Xts, Yts;
			
			getXYdata(Xts, Yts);
			
			int Xscr = -0.0095 * (float)Xts + 0.5346 * (float)Yts - 550.0317;
			int Yscr = 0.3056 * (float)Xts + 0.0047 * (float)Yts - 322.8494;		
			
			printf("X: %d ; Y: %d -> Xscr: %d ; Yscr: %d \n", Xts, Yts, Xscr, Yscr);
			
		
			
			
			sleep(1);
			//usleep(10000);
		}
	
	delete spiHandler; 
	
	}else
	{
		printf("SPI not connected \n");
		return 0;
	}
	
	return 1;
}