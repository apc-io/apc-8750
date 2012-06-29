/*++
	drivers/spi/wmt-spiio.h

	Some descriptions of such software. Copyright (c) 2008  WonderMedia Technologies, Inc.

	This program is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software Foundation,
	either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with
	this program.  If not, see <http://www.gnu.org/licenses/>.

	WonderMedia Technologies, Inc.
	10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.

	History:
		The code was inherit from vt8430
		2009/03/04 First Version
--*/
#include <linux/config.h>
#ifndef WMT_SPIIO_H
/* To assert that only one occurrence is included */
#define WMT_SPIIO_H


/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WMT_SPI_C /* allocate memory for variables only in wmt_spi.c */
#       define EXTERN
#else
#       define EXTERN   extern
#endif /* ifdef WMT_SPI_C */

/* EXTERN int      spi_xxxx; *//*Example*/
#undef EXTERN

/*---------------------------------------------------------------------------------------*/
/* GPIO Public functions declaration*/
/*---------------------------------------------------------------------------------------*/

#define SPI_IOC_MAGIC	'k'

/* SPI IO Control*/
/*-----------------------------*/
/* parameter:1 bus frequence */
/* Get current working frequency of this user*/
#define SPIIOGET_FREQ		_IOR(SPI_IOC_MAGIC,  0, unsigned int)
/* Set working frequency for the user*/
#define SPIIOSET_FREQ		_IOWR(SPI_IOC_MAGIC, 1, unsigned int)

/* parameter:2 clock mode */
/* Get current bus clock mode of this user*/
#define SPIIOGET_CLKMODE	_IOR(SPI_IOC_MAGIC,  2, unsigned int)
/* Set bus clock mode for the user*/
#define SPIIOSET_CLKMODE	_IOWR(SPI_IOC_MAGIC, 3, unsigned int)

/* parameter:4 bus arbiter */
/* Get current bus arbiter of this user*/
#define SPIIOGET_ARBITER	_IOR(SPI_IOC_MAGIC,  4, unsigned int)
/* Set bus arbiter for the user*/
#define SPIIOSET_ARBITER	_IOWR(SPI_IOC_MAGIC, 5, unsigned int)

/* parameter:5 operation mode */
/* Get current operation mode of this user*/
#define SPIIOGET_OPMODE		_IOR(SPI_IOC_MAGIC,  6, unsigned int)
/* Set operation mode for the user*/
#define SPIIOSET_OPMODE		_IOWR(SPI_IOC_MAGIC, 7, unsigned int)

/* parameter:5 port mode */
/* Get current SSn port mode of this user*/
#define SPIIOGET_PORTMODE	_IOR(SPI_IOC_MAGIC,  8, unsigned int)
/* Set SSn port mode for the user*/
#define SPIIOSET_PORTMODE	_IOWR(SPI_IOC_MAGIC, 9, unsigned int)

/* parameter:6 read/write flow control */
/* Read in data from SPI port*/
#define SPIIO_READ			_IOWR(SPI_IOC_MAGIC, 10, struct spi_xfer_req_s)
/* Write out data to SPI port*/
#define SPIIO_WRITE			_IOWR(SPI_IOC_MAGIC, 11, struct spi_xfer_req_s)
/* Write out data from spi port then Read in data to SPI port*/
#define SPIIO_READANDWRITE	_IOWR(SPI_IOC_MAGIC, 12, struct spi_xfer_req_s)

#define SPI_IOC_MAXNR	12

struct spi_xfer_req_s{
	unsigned char *rbuf;	/* read in buffer address*/
	unsigned int rsize;		/* read in data size*/
	unsigned char *wbuf;	/* write out data address*/
	unsigned int wsize;		/* write out data size*/
	unsigned int size;		/* Total size which use when read and write control by upper AP*/
};

#endif
