#include "ADS7846.h"

int ADS7846::open(char* spiName)
{
	spi_config_t spiConfig;
	
	spiConfig.mode=0;
	spiConfig.speed=50000;
	spiConfig.delay=0;
	spiConfig.bits_per_word=8;
	
	spiHandler = new SPI(spiName, &spiConfig);
	
	return spiHandler->begin();
}

uint8_t spiTxBuffer[5] = {0xD0, 0x00, 0x90, 0x00, 0x00};
uint8_t spiRxBuffer[5];

point ADS7846::getXYdata()
{	
	spiHandler->xfer(spiTxBuffer,5,spiRxBuffer,5);
		
	return point(((spiRxBuffer[1] << 8) | spiRxBuffer[2] ) >> 3, ((spiRxBuffer[3] << 8) | spiRxBuffer[4] ) >> 3);
}


/**
 *void getAvaregeResistance(point &avResistance)
 *{
 *	#define READS_NUM 10
 *	
 *	point resistance;
 *	
 *	avResistance.x() = 0;
 *	avResistance.y() = 0;
 *	
 *	for(int i=0; i<READS_NUM; i++)
 *	{
 *		getXYdata(resistance);
 *		avResistance.x() += resistance.x();
 *		avResistance.y() += resistance.y();
 *		usleep(1000);
 *	}
 *	
 *	avResistance.x() = avResistance.x() / READS_NUM;
 *	avResistance.y() = avResistance.y() / READS_NUM;
 *}
 */