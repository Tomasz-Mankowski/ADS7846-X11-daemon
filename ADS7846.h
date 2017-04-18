#ifndef _ADS7846_H_
#define _ADS7846_H_

#include <spidev_lib++.h>

#include <stdlib.h>
#include <unistd.h>

#include "point.h"

class ADS7846
{
public:
	ADS7846() {};
	~ADS7846() {delete spiHandler;}
	
	int open(char* spiName);
	point getXYdata();
	
private:
	SPI* spiHandler = NULL;
};

#endif  /* _ADS7846_H_ */