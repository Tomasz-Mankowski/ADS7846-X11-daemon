#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spidev_lib++.h>
#include <unistd.h>

spi_config_t ADS_spi_config;
uint8_t tx_buffer[3];
uint8_t rx_buffer[3];

int  main( void)
{
  SPI *ADS_SPI = NULL;

  ADS_spi_config.mode=0;
  ADS_spi_config.speed=1000000;
  ADS_spi_config.delay=0;
  ADS_spi_config.bits_per_word=8;

  ADS_SPI=new SPI("/dev/spidev0.0",&ADS_spi_config);

  if (ADS_SPI->begin())
  {
	printf("Beginned\n");
    memset(tx_buffer,0,3);
    memset(rx_buffer,0,3);
    
	const uint8_t poll_commands[4] = {0xD0, 0x90};
	uint16_t values[2];	

	while(1)
	{
		for(int i=0; i<2; i++)
		{
			tx_buffer[0] = poll_commands[i];
			ADS_SPI->xfer(tx_buffer,3,rx_buffer,3);
			
			values[i] = ((rx_buffer[1] << 8) | rx_buffer[2] ) >> 3;
		}
		
		//printf("Data send: 0x%02X 0x%02X 0x%02X\n", tx_buffer[0], tx_buffer[1], tx_buffer[2]);
		//printf("Data received: 0x%02X 0x%02X 0x%02X\n", rx_buffer[0], rx_buffer[1], rx_buffer[2]);
		
		float Xts = (float)values[0];
		float Yts = (float)values[1];
		
		float Xscr = -0.0095 * Xts + 0.5346 * Yts - 550.0317;
		float Yscr = 0.3056 * Xts + 0.0047 * Yts - 322.8494;		
		
		printf("X: %f ; Y: %f -> Xscr: %f ; Yscr: %f \n", Xts, Yts, Xscr, Yscr);
		
	
		
		
		sleep(1);
		//usleep(10000);
	}
	
    delete ADS_SPI; 
  }else
  {
  printf("SPI not connected");
  }
 return 1;
}