#ifndef _ADS7846_H_
#define _ADS7846_H_

#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "point.h"

class ADS7846
{
public:
	ADS7846() { spiOpened_ = false; }
	~ADS7846();
	
	int openCon(char* spiName);
	point getXYdata();
	
private:
	int spiHandler_;
	bool spiOpened_;
};

#endif  /* _ADS7846_H_ */
