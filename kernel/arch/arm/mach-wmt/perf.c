/*++
	linux/arch/arm/mach-wmt/perf.c

	wmt generic architecture level codes
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

/*
  * Set and Get performance monitor register of ARM11
  *
  * Ref: ARM1176JZ-S Technical Reference Manual
  * 1) 3.2.51--c15, Performance Monitor Control Register
  * 2) 3.2.52--c15, Cycle Counter Register
  * 3) 3.2.53--c15, Count Register 0
  * 4) 3.2.54--c15, Count Register 1
  *
  * Note: Please pay attention to the overflow of the count register
  */

/*
  * Usage:
  * 0, #include <asm/perf.h>
  *
  * 1, define and initialize the event:
		unsigned char evt0, evt1;
		uint32_t cycle_count, count0, count1;
  *
  * 2, Setup and start count:
  		wmt_arm11_setup_pmu(evt0, evt1);
  		wmt_arm11_start_pmu();
  *
  * 3, Stop and read count:
  		wmt_arm11_stop_pmu();
 		wmt_arm11_read_counter(&cycle, &count0, &count1);
  *
  */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/timex.h>
#include <linux/errno.h>
#include <linux/profile.h>
#include <linux/sysdev.h>
#include <linux/timer.h>
#include <linux/irq.h>

#include <linux/spinlock.h>


//#include <mach/perf.h>
//#include <asm/perf.h>


#if 0
#define BIT8                    0x00000100
#define BIT9                    0x00000200
#define BIT10                   0x00000400
#endif

/*
 * ARM11 PMU support
 */

#define PMCR_E		(1 << 0)	/* Enable */
#define PMCR_P		(1 << 1)	/* Count reset */
#define PMCR_C		(1 << 2)	/* Cycle counter reset */
#define PMCR_D		(1 << 3)	/* Cycle counter counts every 64th cpu cycle */
#define PMCR_IEN_PMN0	(1 << 4)	/* Interrupt enable count reg 0 */
#define PMCR_IEN_PMN1	(1 << 5)	/* Interrupt enable count reg 1 */
#define PMCR_IEN_CCNT	(1 << 6)	/* Interrupt enable cycle counter */
#define PMCR_OFL_PMN0	(1 << 8)	/* Count reg 0 overflow */
#define PMCR_OFL_PMN1	(1 << 9)	/* Count reg 1 overflow */
#define PMCR_OFL_CCNT	(1 << 10)	/* Cycle counter overflow */
#define PMCR_EVENTBUS_E	(1 << 11)	/* Enable Export of the events to the event bus */

#define PMN0 0
#define PMN1 1
#define CCNT 2

static DEFINE_SPINLOCK(perf_lock);


static inline void wmt_arm11_write_pmnc(uint32_t val)
{
	/* upper 4bits and 7, 11 are write-as-0 */
	val &= 0x0ffff77f;
	asm volatile("mcr p15, 0, %0, c15, c12, 0" : : "r" (val));
}

static inline uint32_t wmt_arm11_read_pmnc(void)
{
	uint32_t val;
	asm volatile("mrc p15, 0, %0, c15, c12, 0" : "=r" (val));
	return val;
}
/*
  * disable and get performance monitor registers
  *
  * @param[out] cycle_count: The value of cycle count register
  * @param[out] count0:  The value of count register 0
  * @param[out] count1:  The value of count register 1
  *
  */

void wmt_arm11_read_counter(uint32_t *cycle_count, uint32_t *count0, uint32_t *count1)
{
	unsigned long flags;

	spin_lock_irqsave(&perf_lock, flags);

	asm volatile (
		"mrc 	p15, 0, r3, c15, c12, 1	\n\t"
		"str 	r3, [%0] 				\n\t"
		"mrc 	p15, 0, r3, c15, c12, 2 \n\t"
		"str 	r3, [%1] 				\n\t" 
		"mrc 	p15, 0, r3, c15, c12, 3 \n\t"
		"str 	r3, [%2] 				\n\t" 
		: 
		: "r" (cycle_count), "r" (count0), "r" (count1)
		: "r3"
	);
	spin_unlock_irqrestore(&perf_lock, flags);

}
EXPORT_SYMBOL(wmt_arm11_read_counter);

static void wmt_arm11_reset_counter(uint32_t cnt, uint32_t val)
{
	unsigned long flags;

	spin_lock_irqsave(&perf_lock, flags);

	switch (cnt) {
	case CCNT:
		asm volatile("mcr p15, 0, %0, c15, c12, 1" : : "r" (val));
		break;

	case PMN0:
		asm volatile("mcr p15, 0, %0, c15, c12, 2" : : "r" (val));
		break;

	case PMN1:
		asm volatile("mcr p15, 0, %0, c15, c12, 3" : : "r" (val));
		break;
	}

	spin_unlock_irqrestore(&perf_lock, flags);

}


int wmt_arm11_setup_pmu(uint8_t event0, uint8_t event1)
{
	uint32_t pmnc = 0;

	unsigned long flags;

	spin_lock_irqsave(&perf_lock, flags);

	if (wmt_arm11_read_pmnc() & PMCR_E) {
		printk(KERN_ERR "perf: CPU%u PMU still enabled when setup new event counter.\n", smp_processor_id());
		spin_unlock_irqrestore(&perf_lock, flags);
		return -1;
	}

	/* initialize PMNC, reset overflow, D bit, C bit and P bit. */
	wmt_arm11_write_pmnc(PMCR_OFL_PMN0 | PMCR_OFL_PMN1 | PMCR_OFL_CCNT |
			 PMCR_C | PMCR_P);

	/*
	 * Set event (if destined for PMNx counters)
	 */
	pmnc |= event0 << 20;
	pmnc |= event1 << 12;

	/*
	 * We don't need to set the event if it's a cycle count
	 * Enable interrupt for this counter
	 */
	//pmnc |= PMCR_IEN_PMN0 << cnt;
	//arm11_reset_counter(cnt);
	
	wmt_arm11_write_pmnc(pmnc);
	spin_unlock_irqrestore(&perf_lock, flags);

	return 0;
}
EXPORT_SYMBOL(wmt_arm11_setup_pmu);

inline int wmt_arm11_start_pmu(void)
{
	unsigned long flags;

	spin_lock_irqsave(&perf_lock, flags);

	wmt_arm11_write_pmnc(wmt_arm11_read_pmnc() | PMCR_E);

	spin_unlock_irqrestore(&perf_lock, flags);

	return 0;
}
EXPORT_SYMBOL(wmt_arm11_start_pmu);

inline int wmt_arm11_stop_pmu(void)
{
	uint32_t cnt;
	unsigned long flags;

	spin_lock_irqsave(&perf_lock, flags);

	cnt = wmt_arm11_read_pmnc();
	wmt_arm11_write_pmnc(cnt & ~PMCR_E); // disable counters

	spin_unlock_irqrestore(&perf_lock, flags);

	//printk("stop: pmnc = 0x%08X\n", cnt);
	if(cnt & PMCR_OFL_PMN0) {
		printk(KERN_INFO "perf: CPU%u Count reg 0 overflow.\n", smp_processor_id());
	}

	if(cnt & PMCR_OFL_PMN1) {
		printk(KERN_INFO "perf: CPU%u Count reg 1 overflow.\n", smp_processor_id());
	}
	if(cnt & PMCR_OFL_CCNT) {
		printk(KERN_INFO "perf: CPU%u Cycle counter overflow.\n", smp_processor_id());
	}

	return 0;
}
EXPORT_SYMBOL(wmt_arm11_stop_pmu);
	
void enable_user_access(void)
{
	uint32_t c15_val;
	uint32_t c15_enable_val;
	unsigned long flags;

	spin_lock_irqsave(&perf_lock, flags);
	
	/*Before Enable: check Access validation register V bit */
	asm volatile("mrc p15, 0, %0, c15, c9, 0" : "=r" (c15_val)); 
	printk("init access control register=0x%8.8x\n", c15_val);

	/* enable V bit */
	c15_enable_val = c15_val | 0x1;
	asm volatile("mcr p15, 0, %0, c15, c9, 0" : : "r" (c15_enable_val));

	/* After Enable: check Access validation register V bit */
	asm volatile("mrc p15, 0, %0, c15, c9, 0" : "=r" (c15_val)); 
	printk("now  access control register=0x%8.8x\n", c15_val);

	spin_unlock_irqrestore(&perf_lock, flags);


}
EXPORT_SYMBOL(enable_user_access);
#if 0
/*
  * Set and enable performance monitor registers
  *
  * @param[in] evt0: The event for count register 0 (should <=0xff)
  * @param[in] evt1: The event for count register 1 (should <=0xff)
  */
inline void arm11_profs(unsigned char evt0, unsigned char evt1)
{
	asm volatile (
		"mov r0, #0 \n\t"
		"mcr p15, 0, r0, c15, c12, 0 \n\t"	/* Disable performance counter */
		"mov r0, %0 \n\t"
		"mov r1, %1 \n\t"
		"mov r0, r0, lsl #20 \n\t"
		"mov r1, r1, lsl #12 \n\t"
		"orr r0, r0, r1 \n\t"
		"mcr p15, 0, r0, c15, c12, 0 \n\t"	/* set evt0, evt1 */
		"mov r2, #6 \n\t"
		"orr r0, r0, r2 \n\t"
		"mcr p15, 0, r0, c15, c12, 0 \n\t"	/* reset counters */
		"mov r2, #1 \n\t"
		"orr r0, r0, r2 \n\t"
		"mcr p15, 0, r0, c15, c12, 0 \n\t"	/* Enable */
		:
		: "r" (evt0), "r" (evt1)
		: "r0", "r1", "r2"
	);
}

/*
  * disable and get performance monitor registers
  *
  * @param[out] cycle_count: The value of cycle count register
  * @param[out] count0:  The value of count register 0
  * @param[out] count1:  The value of count register 1
  *
  * @return: 0: no overflow; 1: have overflow
  */

inline int arm11_profe(uint32_t *cycle_count, uint32_t *count0, uint32_t *count1)
{
	uint32_t ret;
	asm volatile (
		"mov r3, #0 \n\t"
		"mcr p15, 0, r3, c15, c12, 0 \n\t"	/* Disable performance counter */
		"mrc p15, 0, r3, c15, c12, 1 \n\t"
		"str r3, [%0] \n\t"
		"mrc p15, 0, r3, c15, c12, 2 \n\t"
		"str r3, [%1] \n\t"	
		"mrc p15, 0, r3, c15, c12, 3 \n\t"
		"str r3, [%2] \n\t"	
		: 
		: "r" (cycle_count), "r" (count0), "r" (count1)
		: "r3"
	);
	asm volatile (
		"mrc p15, 0, r3, c15, c12, 0 \n\t"
		"mov %0, r3 \n\t"
		: "=r" (ret)
		: 
		: "r3"
	);
	/* overflow check */
	if((ret & PMCR_OFL_PMN0) || (ret & PMCR_OFL_PMN1) || (ret & PMCR_OFL_CCNT)){
		return 1;
	}

	return 0;

}
#endif
