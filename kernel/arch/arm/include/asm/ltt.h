/*
 * linux/include/asm-arm/ltt.h
 *
 * Copyright (C) 2005, Mathieu Desnoyers
 *
 * ARM definitions for tracing system
 */

#ifndef _ASM_ARM_LTT_H
#define _ASM_ARM_LTT_H

#include <linux/jiffies.h>
#include <linux/seqlock.h>

#define LTT_ARCH_TYPE LTT_ARCH_TYPE_ARM
#define LTT_ARCH_VARIANT LTT_ARCH_VARIANT_NONE

#undef LTT_HAS_TSC

#define LTTNG_LOGICAL_SHIFT 13

extern atomic_t lttng_logical_clock;

static inline u32 ltt_get_timestamp32(void)
{
	unsigned long seq;
	unsigned long try = 5;
	u32 ret;

	do {
		seq = read_seqbegin(&xtime_lock);
		ret = (jiffies << LTTNG_LOGICAL_SHIFT) 
			| (atomic_add_return(1, &lttng_logical_clock));
	} while (read_seqretry(&xtime_lock, seq) && (--try) > 0);

	if (try == 0)
		return 0;
	else
		return ret;
}


/* The shift overflow doesn't matter */
static inline u64 ltt_get_timestamp64(void)
{
	unsigned long seq;
	unsigned long try = 5;
	u64 ret;

	do {
		seq = read_seqbegin(&xtime_lock);
		ret = (jiffies_64 << LTTNG_LOGICAL_SHIFT) 
			| (atomic_add_return(1, &lttng_logical_clock));
	} while (read_seqretry(&xtime_lock, seq) && (--try) > 0);

	if (try == 0)
		return 0;
	else
		return ret;
}

/* this has to be called with the write seqlock held */
static inline void ltt_reset_timestamp(void)
{
	atomic_set(&lttng_logical_clock, 0);
}


static inline unsigned int ltt_frequency(void)
{
  return HZ << LTTNG_LOGICAL_SHIFT;
}


static inline u32 ltt_freq_scale(void)
{
  return 1;
}



#endif
