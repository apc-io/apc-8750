/*++
linux/arch/arm/mach-wmt/cpu-freq.c

CPU frequency scaling
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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>

#include <mach/hardware.h>

#include "generic.h"

typedef struct wmt_scale_s {
    unsigned int khz;       /* cpu_clk in khz */
    unsigned char pll;      /* pll mul */
    unsigned char cpu;      /* cpu div */
    unsigned char ahb;      /* ahb div */

} wmt_scale_t;

wmt_scale_t wm8425_freqs[] = {
    /* khz, pll, cpu, ahb */
    { 133001, 0x8, 2, 1 },			/* 250/25/25*/
    { 133002, 0x8, 2, 2 },			/* 250/25/25*/
    { 166001, 0xa, 2, 1 },			/* 250/125/125*/
    { 166002, 0xa, 2, 2 },			/* 250/125/125*/
    { 200002, 0xc, 2, 2 },			/* 450/225/112*/
    { 200003, 0xc, 2, 3 },			/* 450/225/112		198000*/
    { 233002, 0xe, 2, 2 },			/* 500/250/125*/
    { 233003, 0xe, 2, 3 },			/* 500/250/125		231000*/
    { 250002, 0xf, 2, 2 },			/* 250/25/25*/
    { 250003, 0xf, 2, 3 },			/* 250/25/25			247500*/
    { 266002, 0x10, 2, 2 },			/* 250/125/125*/
    { 266003, 0x10, 2, 3 },			/* 250/125/125		264000*/
    { 300002, 0x12, 2, 2 },			/* 450/225/112*/
    { 300003, 0x12, 2, 3 },			/* 450/225/112		297000*/
    { 333002, 0x14, 2, 2 },			/*333 111	 330000*/
    { 333003, 0x14, 2, 3 },			/*333 166*/
};

#define NR_FREQS        ARRAY_SIZE(wm8425_freqs)

unsigned int wm8425_arm_khz(void)
{
	unsigned int value;
	value = 33*1000*(*(unsigned int *)0xd8130200 & 0x1f)/(*(unsigned int *)0xd8130300 & 0x1f);

    return value;
}

unsigned int wm8425_ahb_khz(void)
{
	unsigned int value, cpu_freq;

	cpu_freq = wm8425_arm_khz();
	value = cpu_freq/(*(unsigned int *)0xd8130304);

	return value;
}

unsigned int wmt_freq_to_idx(unsigned int khz)
{
    int i;

    for (i = 0; i < NR_FREQS; i++)
			if (wm8425_freqs[i].khz >= khz)
				break;

    return (i < NR_FREQS) ? (i) : (NR_FREQS-1);
}

wmt_scale_t *wmt_idx_to_parms(unsigned int idx)
{
    wmt_scale_t *ret = NULL;

    if (idx < NR_FREQS)
			ret = &wm8425_freqs[idx];
    else
			ret = &wm8425_freqs[NR_FREQS];

    return ret;
}

unsigned int wmt_idx_to_freq(unsigned int idx)
{
    int freq = 0;

    if (idx < NR_FREQS)
			freq = wm8425_freqs[idx].khz;
    else
			freq = wm8425_freqs[NR_FREQS].khz;

    return freq;
}

/*
 * make sure that only the "userspace" governor is run -- anything else wouldn't make sense on
 * this platform, anyway.
 */
int wmt_verify_speed(struct cpufreq_policy *policy)
{
    unsigned int tmp;

    if (policy->cpu)
			return -EINVAL;

    cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq, policy->cpuinfo.max_freq);

    /*
     * Make sure that at least one frequency is within the policy
     */
    tmp = wm8425_freqs[wmt_freq_to_idx(policy->min)].khz;

    if (tmp > policy->max)
			policy->max = tmp;

    cpufreq_verify_within_limits(policy,
				policy->cpuinfo.min_freq,
				policy->cpuinfo.max_freq);
    return 0;

}

/*
 * Generic macro : restart CPU for reload PLL.
 * PLL-A ,ZAC Clock Divisor,AHB Clock Divisor,PLL-B ,PLL-C
 * need waitting about 5ms for PLL to stable
 */
static void wmt_restart_cpu(void)
{
	unsigned int reg_11c, reg_120;
	const int zero = 0;
	unsigned int reg_time;

	REG8_VAL(0xd8140000 + 0x65) = 0x8;			/* route irq to irq*/
	REG32_VAL(0xd8130000 + 0x120) = 0x03;
	while (REG32_VAL(0xd8130000 + 0x124) != 0)
		;
	reg_time = REG32_VAL(0xd8130000 + 0x110);
	reg_time += 0x0000EA60;
	REG32_VAL(0xd8130000 + 0x104) = reg_time;

	reg_11c = REG32_VAL(0xd8130000 + 0x11C);
	REG32_VAL(0xd8130000 + 0x11C) = 0x02;	/* OSTimer Int Enable*/
	reg_120 = REG32_VAL(0xd8130000 + 0x120);
	REG32_VAL(0xd8130000 + 0x120) = 0x00000001;	/* OSTimer Ctrl*/

    asm("mcr%? p15, 0, %0, c7, c0, 4" : : "r" (zero));		/* Force ARM to idle mode*/
    asm("nop");
    asm("nop");
    asm("nop");

	REG32_VAL(0xd8130000+0x11C) = reg_11c;	/* OSTimer Int Enable*/
	REG32_VAL(0xd8130000+0x120) = reg_120;	/* OSTimer Ctrl*/
	return;
}

static void wm8425_speedstep(unsigned int idx)
{
	wmt_scale_t *np;

	np = wmt_idx_to_parms(idx);

	if (np->ahb > 1) {
		if (np->ahb != *(volatile unsigned int *)0xd8130304) {
			while (PMCS_VAL & PMCS_BUSY)
				;
			while ((*(volatile unsigned int *)0xd8130000) & 0x1fff9b37)
				;
			*(volatile unsigned int *)0xd8130304 = np->ahb;
		}

		if (np->pll != *(volatile unsigned int *)0xd8130200) {
			while ((*(volatile unsigned int *)0xd8130000) & 0x1fff9b37)
				;
			*(volatile unsigned int *)0xd8130200 = np->pll;
		}

		if (np->cpu != *(volatile unsigned int *)0xd8130300) {
			while ((*(volatile unsigned int *)0xd8130000) & 0x1fff9b37)
				;
			*(volatile unsigned int *)0xd8130300 = np->cpu;
		}
	}	else {
		if (np->pll != *(volatile unsigned int *)0xd8130200) {
			while ((*(volatile unsigned int *)0xd8130000) & 0x1fff9b37)
				;
			*(volatile unsigned int *)0xd8130200 = np->pll;
		}
		if (np->cpu != *(volatile unsigned int *)0xd8130300) {
			while ((*(volatile unsigned int *)0xd8130000) & 0x1fff9b37)
				;
			*(volatile unsigned int *)0xd8130300 = np->cpu;
		}
		if (np->ahb != *(volatile unsigned int *)0xd8130304) {
			while (PMCS_VAL & PMCS_BUSY)
				;
			while ((*(volatile unsigned int *)0xd8130000) & 0x1fff9b37)
				;
			*(volatile unsigned int *)0xd8130304 = np->ahb;
		}
	}
	while ((*(volatile unsigned int *)0xd8130000)&0x1fff9b37)
		;
	wmt_restart_cpu();
}

static int wmt_target(struct cpufreq_policy *policy,
							unsigned int target_freq,
							unsigned int relation)
{
    unsigned int idx;
    unsigned long flags;
    struct cpufreq_freqs freqs;

    idx = wmt_freq_to_idx(target_freq);

		switch (relation) {
		case CPUFREQ_RELATION_L:
			/*
			* Try to select a new_freq higher than or equal target_freq.
			*/
			if (wmt_idx_to_freq(idx) > policy->max)
				idx--;
			break;
		case CPUFREQ_RELATION_H:
			/*
			* Try to select a new_freq lower than or equal target_freq.
			*/
			if ((wmt_idx_to_freq(idx) > target_freq) &&
			  (wmt_idx_to_freq(idx-1) >= policy->min))
				idx--;
			break;
		}

    freqs.old = wm8425_arm_khz();
    freqs.new = wmt_idx_to_freq(idx);
    freqs.cpu = 0;

    if (freqs.new != freqs.old) {
			/*
			* Because current design we have all differnet ZAC2
			* clock scheme on each entry, so we can do this check
			* to make sure only apply change on differnet clock.
			* The benefit is skip redundent I/O stop and start.
			*
			* But notice that if we have same ZAC2 clock scheme, we
			* must move CPUFREQ_PRECHANGE and CPUFREQ_POSTCHANGE
			* notify call out side this block.
			*/
			cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
			local_irq_save(flags);
			wm8425_speedstep(idx);
			local_irq_restore(flags);
			cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
    }

    return 0;
}

static int __init wmt_cpu_init(struct cpufreq_policy *policy)
{
    if (policy->cpu != 0)
			return -EINVAL;

    policy->governor = CPUFREQ_DEFAULT_GOVERNOR;
    policy->cur = wm8425_arm_khz();
    policy->min = 133001;
    policy->max = 333003;
    policy->cpuinfo.min_freq = 133001;
    policy->cpuinfo.max_freq = 333003;
    policy->cpuinfo.transition_latency = CPUFREQ_ETERNAL;
    return 0;
}

static struct cpufreq_driver wmt_cpufreq_driver = {
    .flags			= CPUFREQ_STICKY,
    .verify         = wmt_verify_speed,
    .target         = wmt_target,
    .init           = wmt_cpu_init,
    .name           = "wmt",
};

static int __init wmt_cpufreq_init(void)
{
    return cpufreq_register_driver(&wmt_cpufreq_driver);
}

arch_initcall(wmt_cpufreq_init)

