
#ifndef __WMT_PERF_H__
#define __WMT_PERF_H__


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
		uint8_t evt0, evt1;
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

extern int wmt_arm11_setup_pmu(uint8_t event0, uint8_t event1);
extern inline int wmt_arm11_start_pmu(void);
extern inline int wmt_arm11_stop_pmu(void);
extern void wmt_arm11_read_counter(uint32_t *cycle_count, uint32_t *count0, uint32_t *count1);

#ifdef CONFIG_CACHE_L2X0
/*
  * Set and Get event counter of L2C-310
  *
  * Ref: AMBA Level 2 Cache Controller (L2C-310) Technical Reference Manual
  * 1) 3.3.6  Event Counter Control Register
  * 2) 3.3.7  Event Counter Configuration Registers
  * 3) 3.3.8  Event counter value registers
  *
  * Note: Please pay attention to the overflow of the count register
  */

/*
  * Usage:
  * 0, #include <asm/perf.h>
  *
  * 1, define and initialize the event:
		uint8_t l2c_evt0, l2c_evt1;
		uint32_t l2c_counter0, l2c_counter1;
  *
  * 2, Set and enable count:
  		wmt_l2c_setup_counter(l2c_evt0, l2c_evt1);
  		wmt_l2c_stop_counter();
  *
  * 3, Disable and get count:
  		wmt_l2c_stop_counter();
 		wmt_l2c_read_counter(&l2c_counter0, &l2c_counter1);
  *
  */

extern int wmt_l2c_setup_counter(uint8_t evt0,uint8_t evt1);
extern inline void wmt_l2c_start_counter(void);
extern inline void wmt_l2c_stop_counter(void);
extern void wmt_l2c_read_counter(uint32_t *count0, uint32_t *count1);
#endif

#endif
