#ifndef _LINUX_SPI_BU21020_H
#define _LINUX_SPI_BU21020_H

#include <linux/types.h>

struct bu21020_platform_data {
	s16	reset_gpio;
	s16	dav_gpio;
	s16	pen_int_gpio;

	unsigned ts_ignore_last : 1;
};

struct calibration_parameter {
	int   a1;
	int   b1;
	int   c1;
	int   a2;
	int   b2;
	int   c2;
	int   delta;
};


#endif
