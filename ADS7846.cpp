#include "ADS7846.h"

int ADS7846::openCon(char* spiName)
{
	if (spiOpened_)
	{
       return true;
	}
	
	if (spiName == NULL)
	{
       return false;
    }
	
	uint8_t mode = 0;
    uint8_t bits_per_word = 8;
    uint32_t speed = 50000;	
	
	spiHandler_ = open(spiName, O_RDWR);	
	
	if (spiHandler_ < 0) 
	{
        return false;
    }
	
    /* Set SPI_POL and SPI_PHA */
    if (ioctl(spiHandler_, SPI_IOC_WR_MODE, &mode) < 0) 
	{
        close(spiHandler_);
        return false;
    }	
    if (ioctl(spiHandler_, SPI_IOC_RD_MODE, &mode) < 0) 
	{
		close(spiHandler_);
        return false;
    }

    /* Set bits per word*/
    if (ioctl(spiHandler_, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) 
	{
        close(spiHandler_);
        return false;
    }
    if (ioctl(spiHandler_, SPI_IOC_RD_BITS_PER_WORD, &bits_per_word) < 0) 
	{
        close(spiHandler_);
        return false;
    }

    /* Set SPI speed*/
    if (ioctl(spiHandler_, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) 
	{
        close(spiHandler_);
        return false;
    }
    if (ioctl(spiHandler_, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) 
	{
        close(spiHandler_);
        return false;
    }

	spiOpened_ = true;
    return true;
}

uint8_t spiTxBuffer[5] = {0xD0, 0x00, 0x90, 0x00, 0x00};
uint8_t spiRxBuffer[5];

point ADS7846::getXYdata()
{	
	struct spi_ioc_transfer spi_message[1];
    memset(spi_message, 0, sizeof(spi_message));
    
    spi_message[0].rx_buf = (unsigned long)spiRxBuffer;
    spi_message[0].tx_buf = (unsigned long)spiTxBuffer;
    spi_message[0].len = 5;
    ioctl(spiHandler_, SPI_IOC_MESSAGE(1), spi_message);
	
	return point(((spiRxBuffer[1] << 8) | spiRxBuffer[2] ) >> 3, ((spiRxBuffer[3] << 8) | spiRxBuffer[4] ) >> 3);
}

ADS7846::~ADS7846()
{
	if(spiOpened_)
	{
		close(spiHandler_);
	}
}