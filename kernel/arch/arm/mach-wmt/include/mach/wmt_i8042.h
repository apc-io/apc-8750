/*++
linux/include/asm-arm/arch-wmt/wmt_i8042.h

Copyright (c) 2008  WonderMedia Technologies, Inc.

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
--*/

/* Be sure that virtual mapping is defined right */
#ifndef __ASM_ARCH_HARDWARE_H
#error "You must include hardware.h, not wmt_i8042.h"
#endif

#ifndef __VT8500_I8042_H
#define __VT8500_I8042_H

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by 
 * the Free Software Foundation.
 */


/*
 * This is in 50us units, the time we wait for the i8042 to react. This
 * has to be long enough for the i8042 itself to timeout on sending a byte
 * to a non-existent mouse.
 */

#define I8042_CTL_TIMEOUT	10000

/*
 * When the device isn't opened and it's interrupts aren't used, we poll it at
 * regular intervals to see if any characters arrived. If yes, we can start
 * probing for any mouse / keyboard connected. This is the period of the
 * polling.
 */

#define I8042_POLL_PERIOD	HZ/20

/*
 * Status register bits.
 */

#define I8042_STR_PARITY	0x80
#define I8042_STR_TIMEOUT	0x40
#define I8042_STR_AUXDATA	0x20
#define I8042_STR_KEYLOCK	0x10
#define I8042_STR_CMDDAT	0x08
#define I8042_STR_MUXERR	0x04
#define I8042_STR_IBF		0x02
#define	I8042_STR_OBF		0x01

/*
 * Control register bits.
 */

#define I8042_CTR_KBDINT	0x01
#define I8042_CTR_AUXINT	0x02
#define I8042_CTR_IGNKEYLOCK	0x08
#define I8042_CTR_KBDDIS	0x10
#define I8042_CTR_AUXDIS	0x20
#define I8042_CTR_XLATE		0x40

/*
 * Commands.
 */

#define I8042_CMD_CTL_RCTR	0x0120
#define I8042_CMD_CTL_WCTR	0x1060
#define I8042_CMD_CTL_TEST	0x01aa

#define I8042_CMD_KBD_DISABLE	0x00ad
#define I8042_CMD_KBD_ENABLE	0x00ae
#define I8042_CMD_KBD_TEST	0x01ab
#define I8042_CMD_KBD_LOOP	0x11d2

#define I8042_CMD_AUX_DISABLE	0x00a7
#define I8042_CMD_AUX_ENABLE	0x00a8
#define I8042_CMD_AUX_TEST	0x01a9
#define I8042_CMD_AUX_SEND	0x10d4
#define I8042_CMD_AUX_LOOP	0x11d3

#define I8042_CMD_MUX_PFX	0x0090
#define I8042_CMD_MUX_SEND	0x1090

/*
 * Return codes.
 */

#define I8042_RET_CTL_TEST	0x55

/*
 * Expected maximum internal i8042 buffer size. This is used for flushing
 * the i8042 buffers.
 */

#define I8042_BUFFER_SIZE	16

/*
 * Number of AUX ports on controllers supporting active multiplexing
 * specification
 */

#define I8042_NUM_MUX_PORTS	4

/*
 * Debug.
 */

#ifdef DEBUG
static unsigned long i8042_start_time;
#define dbg_init() do { i8042_start_time = jiffies; } while (0)
#define dbg(format, arg...) 							\
	do { 									\
		if (i8042_debug)						\
			printk(KERN_DEBUG __FILE__ ": " format " [%d]\n" ,	\
	 			## arg, (int) (jiffies - i8042_start_time));	\
	} while (0)
#else
#define dbg_init() do { } while (0)
#define dbg(format, arg...) do {} while (0)
#endif


/*
 * Names.
 */

#define I8042_KBD_PHYS_DESC "isa0060/serio0"
#define I8042_AUX_PHYS_DESC "isa0060/serio1"
#define I8042_MUX_PHYS_DESC "isa0060/serio%d"

/*
 * Register numbers.
 */

#define __KBDC_BASE 0xD8008800

#define I8042_COMMAND_REG	(__KBDC_BASE+0x04)	
#define I8042_STATUS_REG	(__KBDC_BASE+0x04)	
#define I8042_DATA_REG		(__KBDC_BASE)

static inline int i8042_read_data(void)
{	
	//printk("i8042_read_data\n");
	return (*(volatile int *)(__KBDC_BASE));
}

static inline int i8042_read_status(void)
{
	//printk("i8042_read_status\n");
	return (*(volatile int *)(__KBDC_BASE+0x04));
}

static inline void i8042_write_data(int val)
{
	//printk("i8042_write_data 0x%x\n",val);
	(*(volatile unsigned char *)(__KBDC_BASE)=(unsigned char)val);
	return;
}

static inline void i8042_write_command(int val)
{
	//printk("i8042_write_command 0x%x\n",val);
	(*(volatile unsigned char *)(__KBDC_BASE+0x04)=(unsigned char)val);
	return;
}

/*Eric added to debug kbdc wakeup*/
/*
static inline int i8042_read_data(void)
{
        //printk("i8042_read_data\n");
        return (*(volatile int *)(__KBDC_BASE));
}
*/
static inline void i8042_write_2e(int val)
{
        //printk("i8042_read_status\n");
        (*(volatile int *)(__KBDC_BASE+0x08)) = (unsigned char)val;
	return;
}

static inline int i8042_read_2f(void)
{
        //printk("i8042_write_data 0x%x\n",val);
        return (*(volatile unsigned char *)(__KBDC_BASE + 0x0C));
}

static inline void i8042_write_2f(int val)
{
        //printk("i8042_write_command 0x%x\n",val);
        (*(volatile unsigned char *)(__KBDC_BASE+0x0C)=(unsigned char)val);
        return;
}


static inline int i8042_platform_init(void)
{
	return 0;
}

static inline void i8042_platform_exit(void)
{

}

#endif /* __WMT_I8042_H */
